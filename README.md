# UyaGUI

`UyaGUI` 是一个基于 Uya 语言实现的嵌入式 GUI 运行时与 Linux 模拟器工程，仓库内包含 GUI 核心、渲染管线、布局系统、组件库、资源管理、平台抽象、SDL2/Framebuffer 模拟后端、示例、测试与性能基准。

项目目标是为资源受限场景提供零 GC、可移植、可测试、便于在 PC 上先行验证的 GUI 开发基线。

## 当前状态

- 截至 `2026-04-26`，Phase 0 到 Phase 5 的仓库内基线已具备，Phase 6 的文档与示例基线已补齐。
- 当前默认 smoke 入口为 `gui/phase6_smoke.uya`，`make build` 会生成 `build/phase6_smoke`。
- Linux 模拟器首版 SDL2 MVP 已落地，入口为 `gui/sim_main.uya`，可通过 `make sim-build`、`make sim-run`、`make sim-headless` 使用。
- 默认文本渲染已从“占位方框”切换为内置 `5x7` 位图字形，英文、数字和常见标点可正常显示。
- 目标板实机验证、正式发布打包、真实字体/图片解码链路仍在后续范围内。

### 本地验证快照

以下结果基于 `2026-04-26` 在当前仓库工作区的实际执行：

- `./uya/bin/uya --version` 输出 `v0.9.4`
- `make build` 通过
- `make sim-build` 通过
- `make sim-run SIM_ARGS="--max-frames 2 --screenshot build/sim/readme_check.bmp"` 通过，并生成截图
- `make test` 通过
  - `gui/test_suite.uya` 的 `92` 个测试通过
  - `gui/render_test_suite.uya` 的 `12` 个测试通过

如果你当前只是想验证 GUI 核心与模拟器相关逻辑，可先单独执行：

```bash
./uya/bin/uya test gui/test_suite.uya -O0
```

## 快速开始

### 1. 检查工具链

仓库已内置 Uya 编译器：

```bash
./uya/bin/uya --version
```

当前 `uya.toml` 要求的最小版本为 `0.9.4`，仓库内置版本与之匹配。

### 2. 常用命令

```bash
make build
make test
make bench
make bench-report
make docs-api
```

说明：

- `make build`：构建默认 smoke 应用 `gui/phase6_smoke.uya`
- `make test`：运行 GUI 主测试集和 render 测试集
- `make bench`：执行 benchmark
- `make bench-report`：生成 `build/phase5_bench.txt`
- `make docs-api`：生成 API 索引文档 `docs/gui_uya_api_reference.md`

### 3. 启用仓库 hooks

```bash
make hooks
```

仓库的 pre-commit hook 会在可用时格式化已暂存的 `*.uya` 文件，并执行 `make test`。

## Linux 模拟器

### 依赖

如果需要 SDL2 窗口模拟器，请先安装 SDL2 开发包。

Debian / Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev
```

### 构建与运行

```bash
make sim-build
make sim-run
make sim-debug
make sim-headless
make sim-fb-run
```

常见用法：

```bash
make sim-run SIM_ARGS="--demo phase6 --max-frames 120"
make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/manual.bmp"
make sim-headless SIM_HEADLESS_ARGS="--max-frames 5 --screenshot build/sim/headless.bmp"
```

可用运行参数：

- `--demo phase4|phase6`
- `--width N --height N --scale N`
- `--fullscreen | --windowed`
- `--root PATH`
- `--title TEXT`
- `--screenshot PATH`
- `--record PATH`
- `--playback PATH`
- `--max-frames N`
- `--profile-every N`
- `--hud | --no-hud`

当前 SDL2 模拟器常用快捷键：

- `Esc`：退出
- `P`：抓取截图（`.bmp` 或 `.uyafb`）
- `R`：开始或结束输入录制
- `L`：回放录制
- `F11`：切换全屏
- `1` / `6`：切换 `phase4` / `phase6` 场景

当前内置字体说明：

- 默认字体为内置 `5x7` 位图字形
- 英文、数字和常见标点可正常显示
- 已支持 `U+4E00..U+9FFF` 常用 CJK 统一表意文字的内置 `8x8` 点阵显示
- 彩色与灰度 framebuffer 上会对字形斜边做轻量 alpha 抗锯齿软化
- 常见全角标点会归一化到 ASCII 标点显示
- 单色 `I1` framebuffer 仍保持硬边渲染
- 其余未覆盖的 Unicode 字符仍会回退为 `?`

## 交叉编译入口

仓库已提供几个便捷目标：

```bash
make build-arm
make build-riscv
make build-esp32
```

说明：

- `build-arm`：导出 C99，便于 Cortex-M 侧工具链继续处理
- `build-riscv`：导出 `rv32_baremetal_softvm` microapp
- `build-esp32`：导出 `xtensa_baremetal_softvm` microapp

## 目录结构

```text
.
├── docs/           # 架构、快速开始、模拟器、移植、性能、主题等文档
├── gui/            # GUI 主工程
│   ├── core/       # 对象树、事件、脏区、几何与基础类型
│   ├── render/     # 渲染上下文、批处理、GPU、零拷贝、字体、图像
│   ├── widget/     # Button、Label、Page、Slider、Chart 等组件
│   ├── layout/     # abs / auto / flex / grid
│   ├── anim/       # tween / timeline / easing
│   ├── style/      # 主题、样式、属性系统
│   ├── res/        # pool、buffer、cache、fs
│   ├── platform/   # 显示、输入、时钟与 SDL2 / FB 后端
│   ├── sim/        # 模拟器 runner、配置、录制、截图、profiler
│   ├── examples/   # smoke 与 demo 示例
│   └── tests/      # 单元测试与回归测试
├── tools/          # 构建与文档脚本
├── uya/            # 内置 Uya 编译器与标准库
├── Makefile
└── uya.toml
```

## 示例入口

- `gui/examples/phase0_smoke.uya`
- `gui/examples/phase1_smoke.uya`
- `gui/examples/phase2_smoke.uya`
- `gui/examples/phase3_smoke.uya`
- `gui/examples/phase4_smoke.uya`
- `gui/examples/phase6_smoke.uya`
- `gui/examples/demo_clock.uya`
- `gui/examples/demo_music.uya`
- `gui/examples/demo_settings.uya`
- `gui/examples/demo_dashboard.uya`
- `gui/examples/demo_game.uya`
- `gui/examples/demo_perf.uya`

## 文档导航

建议阅读顺序：

1. [快速开始](docs/gui_uya_quickstart.md)
2. [架构说明](docs/gui_uya_architecture.md)
3. [Linux 模拟器方案](docs/gui_uya_linux_sim.md)
4. [自定义组件指南](docs/gui_uya_custom_widget_guide.md)
5. [主题指南](docs/gui_uya_theme_guide.md)
6. [性能指南](docs/gui_uya_performance_guide.md)
7. [移植指南](docs/gui_uya_porting_guide.md)
8. [API 参考索引](docs/gui_uya_api_reference.md)

其他参考：

- [开发环境说明](docs/development.md)
- [设计文档](docs/gui_uya_design.md)
- [Phase 6 报告](docs/gui_uya_phase6_report.md)
- [项目 TODO](docs/gui_uya_todo.md)

## 已知问题

- `make sim-build` 能完成链接，但由 Uya 生成的 C 文件仍会产生较多编译 warning；目前不影响 `make sim-run` 正常使用。
- 当前截图已支持 `BMP` 导出；如需原始 framebuffer dump，可显式使用 `.uyafb`；`PNG` 仍未接入。
- Framebuffer 后端已具备首版显示链路，但输入侧 `indev_fb` 仍未补齐。
- 默认内置字体已支持 ASCII 与 `U+4E00..U+9FFF` 中文常用字，但仍未覆盖完整 Unicode 字体排版链路。

## 相关文件

- [Makefile](Makefile)
- [项目配置](uya.toml)
- [模拟器构建脚本](tools/build_gui_sim.sh)
- [测试总入口](gui/test_suite.uya)
- [Render 测试总入口](gui/render_test_suite.uya)
- [模拟器入口](gui/sim_main.uya)
