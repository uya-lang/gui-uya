# UyaGUI 快速入门

## 目标

这份文档面向第一次接触仓库的开发者，帮助你在本地完成以下事情：

1. 验证工具链
2. 构建最新 smoke 示例
3. 运行测试与 benchmark
4. 理解一个最小页面如何由组件拼装

## 环境准备

仓库已经内置编译器：

```bash
./uya/bin/uya --version
```

常用命令：

```bash
make build
make test
make bench
make bench-report
make bench-verify
make dashboard-compare-report
make docs-api
```

默认 `make build` 当前会编译 `gui/phase6_smoke.uya`，它会串联 Phase 6 的 demo 应用。

## 目录速览

- `gui/core/`: 基础类型、对象树、事件、脏区
- `gui/render/`: 渲染上下文、图片、批处理、GPU、零拷贝
- `gui/widget/`: 组件库
- `gui/layout/`: Flex/Grid/自动布局
- `gui/anim/`: tween、timeline、easing
- `gui/res/`: 池、缓存、文件系统
- `gui/platform/`: 显示、输入、时钟
- `gui/examples/`: smoke 和 demo 应用
- `gui/tests/`: 单元测试与运行时回归

## 最小页面

下面这个思路与 [phase6_smoke.uya](/home/winger/gui-uya/gui/examples/phase6_smoke.uya:1) 中的 demo 一致：

```uya
var page: Page = Page.new("Demo");
page.widget.size(96, 64);

var panel: Panel = Panel.new();
panel.widget.at(4, 4);
panel.widget.size(88, 56);

var label: Label = Label.new("Hello UyaGUI");
label.widget.at(8, 8);
label.widget.size(72, 12);

page.add(panel.obj());
panel.add(label.obj());
page.render(&ctx);
```

它的关键点只有三件事：

1. 创建 `Page` 作为页面根节点
2. 把组件通过 `obj()` 暴露为 `GuiObj` 再挂进树里
3. 通过 `RenderCtx` 渲染到 framebuffer

## 推荐阅读顺序

1. [gui_uya_architecture.md](./gui_uya_architecture.md)
2. [gui_uya_theme_guide.md](./gui_uya_theme_guide.md)
3. [gui_uya_custom_widget_guide.md](./gui_uya_custom_widget_guide.md)
4. [gui_uya_performance_guide.md](./gui_uya_performance_guide.md)
5. [gui_uya_porting_guide.md](./gui_uya_porting_guide.md)

## 示例入口

- [demo_clock.uya](/home/winger/uya/gui-uya/gui/examples/demo_clock.uya:1)
- [demo_music.uya](/home/winger/uya/gui-uya/gui/examples/demo_music.uya:1)
- [demo_settings.uya](/home/winger/uya/gui-uya/gui/examples/demo_settings.uya:1)
- [demo_dashboard.uya](/home/winger/uya/gui-uya/gui/examples/demo_dashboard.uya:1)
- [demo_game.uya](/home/winger/uya/gui-uya/gui/examples/demo_game.uya:1)
- [demo_perf.uya](/home/winger/uya/gui-uya/gui/examples/demo_perf.uya:1)
- [demo_novel.uya](/home/winger/gui-uya/gui/examples/demo_novel.uya:1)
