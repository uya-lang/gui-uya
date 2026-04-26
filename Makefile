UYA ?= ./uya/bin/uya
BUILD_DIR ?= build
TEST_DIR ?= gui/tests
BENCH_DIR ?= gui/benchmarks
EXAMPLE_DIR ?= gui/examples
SMOKE_APP ?= gui/phase4_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= gui/test_suite.uya
RENDER_TEST_ENTRY ?= gui/render_test_suite.uya
MODE ?= debug
UYA_OPT := $(if $(filter release,$(MODE)),-O3,-O0)

.PHONY: build test bench clean hooks build-arm build-riscv build-esp32

BENCH_REPORT ?= $(BUILD_DIR)/phase5_bench.txt

build:
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/phase4_smoke

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

ci:
	$(MAKE) build
	$(MAKE) test
	$(MAKE) bench

clean:
	rm -rf $(BUILD_DIR) .uyacache

hooks:
	git config core.hooksPath .githooks

# ARM Cortex-M 当前以 C99 代码生成为交叉编译交接点。
build-arm:
	@mkdir -p $(BUILD_DIR)/c99
	TARGET_ARCH=arm TARGET_OS=none $(UYA) build --c99 $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/c99/phase4_smoke_arm.c

build-riscv:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile rv32_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase4_smoke_rv32.pobj

build-esp32:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile xtensa_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase4_smoke_xtensa.pobj
