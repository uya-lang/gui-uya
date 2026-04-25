UYA ?= ./uya/bin/uya
BUILD_DIR ?= build
TEST_DIR ?= gui/tests
BENCH_DIR ?= gui/benchmarks
EXAMPLE_DIR ?= gui/examples
SMOKE_APP ?= gui/phase0_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= gui/test_suite.uya
ABS_BUILD_DIR := $(abspath $(BUILD_DIR))
MODE ?= debug
UYA_OPT := $(if $(filter release,$(MODE)),-O3,-O0)

.PHONY: build test bench clean hooks build-arm build-riscv build-esp32

build:
	@mkdir -p $(ABS_BUILD_DIR)
	$(UYA) build $(SMOKE_APP) $(UYA_OPT) -o $(ABS_BUILD_DIR)/phase0_smoke

test:
	$(UYA) test $(TEST_ENTRY) $(UYA_OPT)

bench:
	@mkdir -p $(ABS_BUILD_DIR)
	$(UYA) run $(BENCH_APP) $(UYA_OPT)

clean:
	rm -rf $(BUILD_DIR) .uyacache

hooks:
	git config core.hooksPath .githooks

# ARM Cortex-M 当前以 C99 代码生成为交叉编译交接点。
build-arm:
	@mkdir -p $(ABS_BUILD_DIR)/c99
	TARGET_ARCH=arm TARGET_OS=none $(UYA) build --c99 $(SMOKE_APP) $(UYA_OPT) -o $(ABS_BUILD_DIR)/c99/phase0_smoke_arm.c

build-riscv:
	@mkdir -p $(ABS_BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile rv32_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(ABS_BUILD_DIR)/microapp/phase0_smoke_rv32.pobj

build-esp32:
	@mkdir -p $(ABS_BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile xtensa_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(ABS_BUILD_DIR)/microapp/phase0_smoke_xtensa.pobj
