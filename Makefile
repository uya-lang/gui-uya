UYA ?= ./uya/bin/uya
BUILD_DIR ?= build
TEST_DIR ?= gui/tests
BENCH_DIR ?= gui/benchmarks
EXAMPLE_DIR ?= gui/examples
SMOKE_APP ?= gui/phase6_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= gui/test_suite.uya
RENDER_TEST_ENTRY ?= gui/render_test_suite.uya
TEXT_COMPARE_APP ?= gui/text_render_compare.uya
LVGL_COMPARE_DIR ?= tools/lvgl_compare
LVGL_COMPARE_BUILD_DIR ?= $(BUILD_DIR)/lvgl_compare
MODE ?= debug
UYA_OPT := $(if $(filter release,$(MODE)),-O3,-O0)
DASHBOARD_COMPARE_FRAMES ?= 120
DASHBOARD_COMPARE_MODE ?= release
LVGL_DASHBOARD_FRAMES ?= $(DASHBOARD_COMPARE_FRAMES)
LVGL_DASHBOARD_REBUILD ?= 0
FONT_BACKEND_COMPARE_FRAMES ?= 120

.PHONY: build test bench bench-report docs-api ci clean hooks build-arm build-riscv build-esp32 sim-build sim-run sim-debug sim-headless text-compare lvgl-text-compare lvgl-dashboard-compare dashboard-compare font-backend-compare

SIM_BUILD_DIR ?= $(BUILD_DIR)/sim
SIM_BIN ?= $(SIM_BUILD_DIR)/gui_uya_sim
SIM_ARGS ?=
SIM_HEADLESS_ARGS ?= --max-frames 3 --screenshot $(SIM_BUILD_DIR)/headless.bmp
SIM_FB_ARGS ?= --backend fb --max-frames 60

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
	$(UYA) run $(BENCH_APP) -O3 2>&1 | sed -n '/^Phase5 benchmark\/report/,$$p' > $(BENCH_REPORT)
	@sed -n '1,240p' $(BENCH_REPORT)

text-compare:
	@mkdir -p $(BUILD_DIR)/text_compare
	$(UYA) run $(TEXT_COMPARE_APP) $(UYA_OPT)

lvgl-text-compare:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR)
	cmake -S $(LVGL_COMPARE_DIR) -B $(LVGL_COMPARE_BUILD_DIR)
	cmake --build $(LVGL_COMPARE_BUILD_DIR) -j
	SDL_VIDEODRIVER=dummy $(LVGL_COMPARE_BUILD_DIR)/lvgl_text_compare

lvgl-dashboard-compare:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR) $(BUILD_DIR)/dashboard_compare
	cmake -S $(LVGL_COMPARE_DIR) -B $(LVGL_COMPARE_BUILD_DIR)
	cmake --build $(LVGL_COMPARE_BUILD_DIR) -j --target lvgl_dashboard_compare
	SDL_VIDEODRIVER=dummy LVGL_DASHBOARD_FRAMES=$(LVGL_DASHBOARD_FRAMES) LVGL_DASHBOARD_REBUILD=$(LVGL_DASHBOARD_REBUILD) $(LVGL_COMPARE_BUILD_DIR)/lvgl_dashboard_compare

dashboard-compare:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) sim-build MODE=$(DASHBOARD_COMPARE_MODE)
	SDL_VIDEODRIVER=dummy $(SIM_BIN) --demo dashboard --max-frames $(DASHBOARD_COMPARE_FRAMES) --screenshot $(BUILD_DIR)/dashboard_compare/uya_dashboard.bmp
	$(MAKE) lvgl-dashboard-compare LVGL_DASHBOARD_FRAMES=$(DASHBOARD_COMPARE_FRAMES) LVGL_DASHBOARD_REBUILD=$(LVGL_DASHBOARD_REBUILD)

font-backend-compare:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) sim-build MODE=release
	SDL_VIDEODRIVER=dummy $(SIM_BIN) --demo dashboard --max-frames $(FONT_BACKEND_COMPARE_FRAMES) --screenshot $(BUILD_DIR)/dashboard_compare/uya_font_backend_uya.bmp
	SDL_VIDEODRIVER=dummy UYA_FONT_BACKEND=c $(SIM_BIN) --demo dashboard --max-frames $(FONT_BACKEND_COMPARE_FRAMES) --screenshot $(BUILD_DIR)/dashboard_compare/uya_font_backend_c.bmp

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

sim-headless: sim-build
	SDL_VIDEODRIVER=dummy $(SIM_BIN) $(SIM_HEADLESS_ARGS)

sim-fb-run: sim-build
	$(SIM_BIN) $(SIM_FB_ARGS)
