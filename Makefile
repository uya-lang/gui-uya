UYA ?= ./uya/bin/uya
BUILD_DIR ?= build
TEST_DIR ?= gui/tests
BENCH_DIR ?= gui/benchmarks
EXAMPLE_DIR ?= gui/examples
SMOKE_APP ?= gui/phase6_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= gui/test_suite.uya
RENDER_TEST_ENTRY ?= gui/render_test_suite.uya
MODE ?= debug
UYA_OPT := $(if $(filter release,$(MODE)),-O3,-O0)

.PHONY: build test bench bench-report docs-api ci clean hooks build-arm build-riscv build-esp32

SIM_BUILD_DIR ?= $(BUILD_DIR)/sim
SIM_BIN ?= $(SIM_BUILD_DIR)/gui_uya_sim
SIM_ARGS ?=

BENCH_REPORT ?= $(BUILD_DIR)/phase5_bench.txt

build:
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/phase6_smoke

test:
	$(UYA) test $(TEST_ENTRY) $(UYA_OPT)
	$(UYA) test $(RENDER_TEST_ENTRY) $(UYA_OPT)

bench:
	@mkdir -p $(BUILD_DIR)
	$(UYA) run $(BENCH_APP) $(UYA_OPT)

bench-report:
	@mkdir -p $(BUILD_DIR)
	$(UYA) run $(BENCH_APP) -O3 | tail -n 18 > $(BENCH_REPORT)
	@sed -n '1,160p' $(BENCH_REPORT)

docs-api:
	bash tools/gen_gui_api_docs.sh

ci:
	$(MAKE) build
	$(MAKE) test
	$(MAKE) bench
	$(MAKE) docs-api

clean:
	rm -rf $(BUILD_DIR) .uyacache

hooks:
	git config core.hooksPath .githooks

# ARM Cortex-M 当前以 C99 代码生成为交叉编译交接点。
build-arm:
	@mkdir -p $(BUILD_DIR)/c99
	TARGET_ARCH=arm TARGET_OS=none $(UYA) build --c99 $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/c99/phase6_smoke_arm.c

build-riscv:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile rv32_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_rv32.pobj

build-esp32:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile xtensa_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_xtensa.pobj

sim-build:
	@mkdir -p $(SIM_BUILD_DIR)
	@MODE=$(MODE) BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh

sim-run: sim-build
	$(SIM_BIN) $(SIM_ARGS)

sim-debug:
	@mkdir -p $(SIM_BUILD_DIR)
	@MODE=debug BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh >/dev/null
	$(SIM_BIN) --hud --profile-every 60 $(SIM_ARGS)
