# UyaGUI - 详细开发 TODO 文档

> 版本: v0.1.0  
> 日期: 2026-04-24  
> 状态: 详细设计阶段

> 说明: 本文中的类型与函数签名默认以当前 Uya 语法为准；涉及未来 API 形态时，会显式写成“提案”而不是直接当成现行语法。

---

## 项目总览

| 阶段 | 状态 | 预计工期 | 实际工期 |
|------|------|---------|---------|
| Phase 0: 基础设施 | 已实现 | 3 周 | - |
| Phase 1: 核心系统 | 未开始 | 4 周 | - |
| Phase 2: 渲染引擎 | 未开始 | 3 周 | - |
| Phase 3: 组件库 | 未开始 | 3 周 | - |
| Phase 4: 高级特性 | 未开始 | 2 周 | - |
| Phase 5: 优化与测试 | 未开始 | 3 周 | - |
| Phase 6: 文档与示例 | 未开始 | 2 周 | - |
| **总计** | | **20 周** | |

---

## Phase 0: 基础设施 (Week 1-3)

> 注: 当前仓库已完成 Phase 0 的可编译可测试版本；少数条目因现阶段 Uya/C99 后端限制采用了等价实现，例如 `Rect.union_rect`、`EventQueue::pop(out_evt)`、`GuiObj` 专用对象池以及扁平化事件载荷字段。

### Week 1: 项目搭建与环境配置

#### Day 1-2: 项目初始化
- [x] 创建 git 仓库和目录结构
  - [x] `mkdir -p gui/{core,render,widget,layout,anim,style,res,platform,tests,benchmarks,examples}`
  - [x] 初始化 `uya.toml` 项目配置文件
  - [x] 设置 CI/CD 工作流 (GitHub Actions)
- [ ] 配置开发环境
  - [x] 安装 Uya 编译器 v0.1.0+
  - [x] 配置 VS Code / Cursor 编辑环境
  - [x] 安装 uyaFmt 格式化工具（当前阶段不需要，保持可选）
  - [x] 配置 git hooks (pre-commit 格式化)
- [x] 编写构建脚本
  - [x] `Makefile` - 定义 build/test/bench/clean 目标
  - [x] 支持目标平台交叉编译 (ARM Cortex-M, ESP32, RISC-V)
  - [x] 调试/发布模式配置

#### Day 3-4: 基础类型与工具函数
- [x] `core/point.uya` - 2D点/向量基础
  - [x] `struct Point { x: i16, y: i16 }`
  - [x] 结构体方法: `add`, `sub`, `scale`, `distance`, `lerp`
  - [x] 常量: `POINT_ZERO`, `POINT_ONE`
- [ ] `core/rect.uya` - 矩形区域运算
  - [x] `struct Rect { x: i16, y: i16, w: u16, h: u16 }`
  - [x] 结构体方法: `is_empty`, `contains`, `intersect`, `union`, `inflate`, `offset`
  - [ ] 实现 `Copy` trait (Uya 中通过 `Copy` 标记)
  - [ ] 说明: 当前实现使用 `union_rect` / `rect_union` 规避关键字冲突
- [x] `core/color.uya` - 颜色空间
  - [x] `struct Color { r: u8, g: u8, b: u8, a: u8 }`
  - [x] 宏 `COLOR(hex)` 编译期颜色常量
  - [x] 结构体方法: `blend`, `to_rgb565`, `to_argb8888`, `lerp`
  - [x] 预定义颜色常量: `BLACK`, `WHITE`, `RED`, `GREEN`, `BLUE`, `TRANSPARENT`
- [x] `core/size.uya` - 尺寸类型
  - [x] `struct Size { w: u16, h: u16 }`
  - [x] 结构体方法: `is_empty`, `area`, `aspect_ratio`

#### Day 5: 单元测试框架
- [x] 编写 `tests/test_utils.uya`
  - [x] `assert_eq` 泛型断言宏
  - [x] `assert_near` 浮点比较
  - [x] `test_suite` 测试套件宏
- [x] 为核心类型编写单元测试
  - [x] `test_rect.uya` - 矩形运算测试 (20+ 用例)
  - [x] `test_color.uya` - 颜色混合测试 (10+ 用例)

### Week 2: 内存管理与对象池

#### Day 1-2: 位图分配器
- [ ] `core/bitmap.uya` - 位图分配器
  - [x] `struct BitmapAllocator`
  - [x] 结构体方法: `alloc`, `free`, `is_used`, `find_first_zero`
  - [ ] 使用 `ctz` 指令优化查找 (O(1))
  - [x] 线程安全: `atomic` 操作
- [ ] 单元测试
  - [x] 分配/释放循环测试
  - [ ] 并发分配测试

#### Day 3-4: 内存池
- [x] `res/pool.uya` - 内存池
  - [x] `struct MemPool` - 固定大小块分配
  - [x] `struct PoolManager` - 多级内存池管理
  - [x] 泛型接口: `TypedPool<T: Sized>`
  - [x] 结构体方法: `alloc`, `free`, `used_count`, `free_count`
- [ ] `res/buf.uya` - 缓冲区管理
  - [x] `struct Buffer` - 动态缓冲区
  - [x] 结构体方法: `resize`, `clear`, `append`
  - [ ] `Slice<T>` 类型安全切片
- [ ] 单元测试
  - [x] 内存池压力测试 (10000+ 次分配/释放)
  - [ ] 内存碎片测试
  - [x] 边界测试

#### Day 5: 对象池
- [ ] `core/obj_pool.uya` - GUI 对象池
  - [ ] `struct ObjPool<T: IGuiObj>` - 泛型对象池
  - [x] 结构体方法: `alloc_obj`, `free_obj`, `get_obj`, `foreach`
  - [x] 对象槽位状态管理 (Free/Used/Recycling)
- [x] 性能基准测试
  - [x] 对象创建延迟测试
  - [x] 与裸 malloc/free 对比

### Week 3: 对象系统核心

#### Day 1-2: 基础对象
- [ ] `core/obj.uya` - GuiObj 核心
  - [x] 定义 `IGuiObj` 接口
    - [x] `fn type_id(self: &Self) u32`
    - [x] `fn parent(self: &Self) &GuiObj`
    - [x] `fn area(self: &Self) Rect`
    - [x] `fn set_area(self: &Self, r: Rect) void`
    - [x] `fn screen_area(self: &Self) Rect`
    - [x] `fn invalidate(self: &Self) void`
    - [x] `fn visible(self: &Self) bool`
    - [x] `fn layout(self: &Self, available: Rect) void`
    - [x] `fn render(self: &Self, ctx: &RenderCtx) void`
    - [x] `fn handle_event(self: &Self, evt: &Event) bool`
  - [ ] 定义 `struct GuiObj: IGuiObj`
    - [x] 树形结构字段 (parent, children 链表)
    - [x] 几何字段 (x, y, w, h)
    - [x] 标志字段 (ObjFlags 位域)
    - [x] 样式引用
  - [ ] 结构体方法实现
    - [x] `fn default() GuiObj` - 默认构造
    - [ ] `fn drop(self: &GuiObj) void` - RAII 析构
    - [x] `fn screen_area(self: &GuiObj) Rect` - 绝对坐标计算
    - [x] `fn invalidate(self: &GuiObj) void` - 脏区域传播
    - [x] `fn move_to(self: &GuiObj, x: i16, y: i16) void`
    - [x] `fn set_size(self: &GuiObj, w: u16, h: u16) void`
- [x] `IContainer` 接口定义
  - [x] `fn add_child(self: &Self, child: &GuiObj) void`
  - [x] `fn remove_child(self: &Self, child: &GuiObj) void`
  - [x] `fn child_count(self: &Self) i32`
  - [x] `fn child_at(self: &Self, index: i32) &GuiObj`

#### Day 3: 对象树管理
- [ ] `core/obj_tree.uya` - 对象树操作
  - [x] `struct ObjTree`
  - [ ] 结构体方法:
    - [x] `attach(parent_idx, child_idx)` - 附加子对象
    - [x] `detach(child_idx)` - 分离子对象
    - [x] `foreach_child(parent_idx, callback)` - 遍历子对象
    - [x] `post_order(root_idx, callback)` - 后序遍历
    - [x] `pre_order(root_idx, callback)` - 前序遍历
    - [x] `find_at_point(root_idx, point)` - 命中测试
    - [x] `bubble_path(target_idx)` - 冒泡路径构建
- [x] 单元测试
  - [x] 树的构建/销毁
  - [x] 遍历顺序验证
  - [x] 深度/广度测试

#### Day 4-5: 事件系统基础
- [ ] `core/event.uya` - 事件定义
  - [x] `enum EventType: u8` - 事件类型枚举
  - [x] `enum InputDev: u8` - 输入设备类型
  - [x] `struct Point` - 坐标点
  - [ ] `struct Event` - 事件结构体
    - [x] 基础字段 (type, target, timestamp)
    - [ ] Union 数据 (touch/key/encoder)
    - [x] 传播控制 (stop_bubble, handled)
  - [ ] `struct EventQueue` - 环形缓冲区队列
    - [x] `fn push(self, evt) bool` - 入队
    - [x] `fn pop(self: &Self) Option<Event>` - 出队
    - [x] `fn is_empty(self) bool`
    - [x] `fn is_full(self) bool`
- [x] 手势识别器基础
  - [x] `struct GestureDetector`
  - [x] 结构体方法: `process_touch`

---

## Phase 1: 核心系统 (Week 4-7)

### Week 4: 样式系统

#### Day 1-2: 样式定义
- [ ] `style/prop.uya` - 样式属性枚举
  - [ ] `enum StyleProp: u16` - 所有样式属性
    - [ ] 颜色组 (BgColor, FgColor, BorderColor, ...)
    - [ ] 尺寸组 (Width, Height, Padding, Margin, ...)
    - [ ] 字体组 (Font, FontSize, ...)
    - [ ] 效果组 (Radius, Shadow, Opacity, ...)
- [ ] `style/style.uya` - 样式实现
  - [ ] `union StyleValue` - 样式值
  - [ ] `struct StyleEntry` - 属性-值对
  - [ ] `struct Style` - 样式表
    - [ ] 内联常用属性 (O(1)访问)
    - [ ] 扩展属性数组
  - [ ] 结构体方法:
    - [ ] `fn get(self: &Self, prop: StyleProp) Option<StyleValue>`
    - [ ] `fn set(self, prop, value) void`
    - [ ] `fn merge(self, other) void` - 样式合并
    - [ ] `fn clone(self) Style` - 样式复制
- [ ] 单元测试 (属性读写 50+ 用例)

#### Day 3-4: 主题系统
- [ ] `style/theme.uya` - 主题管理
  - [ ] `enum ThemeVariant: u8` (Light/Dark/System)
  - [ ] `struct Theme`
    - [ ] 配色方案 (primary, secondary, accent, ...)
    - [ ] 组件样式引用
  - [ ] `struct ThemeManager`
    - [ ] `fn register_theme(self, theme) void`
    - [ ] `fn switch_theme(self, variant) void`
    - [ ] `fn current(self) &Theme`
    - [ ] `fn apply_to_tree(self, root) void`
- [ ] 预定义主题
  - [ ] Material Light 主题
  - [ ] Material Dark 主题
  - [ ] 紧凑主题 (资源受限设备)

#### Day 5: 样式构建器
- [ ] 实现 Fluent API 构建器
  - [ ] `StyleBuilder::new()`
  - [ ] `.bg_color(c)`, `.fg_color(c)`, `.border(w, c)`
  - [ ] `.radius(r)`, `.padding(v)`, `.margin(v)`
  - [ ] `.font(f)`, `.font_size(s)`
  - [ ] `.opacity(o)`, `.shadow(x, y, blur, color)`
  - [ ] `.build() -> Style`
- [ ] 宏 `style!` 简化定义
- [ ] 单元测试

### Week 5: 事件系统完整实现

#### Day 1-2: 手势识别
- [ ] `core/gesture.uya` - 完整手势识别
  - [ ] `GestureDetector` 增强
    - [ ] 点击识别 (Press + Release, 时间/距离阈值)
    - [ ] 长按识别 (时间阈值)
    - [ ] 滑动识别 (方向 + 速度)
    - [ ] 拖拽识别
    - [ ] 双击识别
    - [ ] 捏合识别 (多点触摸)
  - [ ] 配置参数结构体
    - [ ] `click_max_duration: u16` (默认 500ms)
    - [ ] `long_press_duration: u16` (默认 800ms)
    - [ ] `swipe_threshold: u16` (默认 30px)
    - [ ] `double_click_interval: u16` (默认 300ms)
- [ ] 单元测试 (模拟输入序列)

#### Day 3-4: 事件分发器
- [ ] `core/event_dispatch.uya` - 事件分发
  - [ ] `struct EventDispatcher`
    - [ ] 事件队列 (`EventQueue`)
    - [ ] 手势识别器 (`GestureDetector`)
    - [ ] 焦点管理 (`focus_obj: atomic i32`)
    - [ ] 捕获管理 (`capture_obj: atomic i32`)
  - [ ] 结构体方法:
    - [ ] `fn dispatch(self, evt, tree) void` - 事件分发
    - [ ] `fn find_target(self, tree, point) i32` - 命中测试
    - [ ] `fn bubble_event(self, target, evt) bool` - 冒泡处理
    - [ ] `fn set_focus(self, obj_idx) void`
    - [ ] `fn get_focus(self) i32`
  - [ ] 事件三阶段: Capture -> Target -> Bubble
- [ ] 输入设备抽象
  - [ ] `interface IInputDev`
  - [ ] `struct TouchDriver: IInputDev`
  - [ ] `struct MouseDriver: IInputDev`
  - [ ] `struct KeyDriver: IInputDev`
  - [ ] `struct EncoderDriver: IInputDev`

#### Day 5: 事件回调系统
- [ ] 实现回调机制
  - [ ] `fn on_click(self, callback) void`
  - [ ] `fn on_press(self, callback) void`
  - [ ] `fn on_release(self, callback) void`
  - [ ] `fn on_long_press(self, callback) void`
  - [ ] `fn on_value_change(self, callback) void`
  - [ ] `fn on_focus(self, callback) void`
- [ ] 回调上下文传递 (user_data)
- [ ] 单元测试 (完整事件流)

### Week 6: 布局系统

#### Day 1-2: Flex 布局
- [ ] `layout/flex.uya` - Flex 布局引擎
  - [ ] `enum FlexDir: u8` (Row/Column/Reverse)
  - [ ] `enum Justify: u8` (Start/Center/End/SpaceBetween/SpaceAround/SpaceEvenly)
  - [ ] `enum Align: u8` (Start/Center/End/Stretch/Baseline)
  - [ ] `struct FlexConfig`
    - [ ] direction, justify, align_items, align_content
    - [ ] wrap, gap, row_gap, col_gap
    - [ ] padding
  - [ ] `struct FlexLayout`
    - [ ] `fn apply(self, container) void` - 执行布局
  - [ ] Flex 属性
    - [ ] flex_grow, flex_shrink, flex_basis
    - [ ] align_self (覆盖容器 align_items)
- [ ] 单元测试 (各种组合 30+ 用例)

#### Day 3: Grid 布局
- [ ] `layout/grid.uya` - Grid 布局
  - [ ] `struct GridConfig`
    - [ ] columns: `[u8: MAX_GRID_COLS]` (列宽模板)
    - [ ] rows: `[u8: MAX_GRID_ROWS]` (行高模板)
    - [ ] gap: `(u8, u8)` (行列间距)
    - [ ] auto_flow: `GridAutoFlow`
  - [ ] `struct GridLayout`
    - [ ] `fn apply(self, container) void`
  - [ ] Grid 属性
    - [ ] grid_column, grid_row
    - [ ] grid_area
- [ ] 单元测试

#### Day 4: 绝对定位与混合布局
- [ ] `layout/abs.uya` - 绝对定位
- [ ] `layout/auto.uya` - 自动布局选择
- [ ] 布局缓存系统
  - [ ] 脏标记 (`layout_dirty`)
  - [ ] 增量布局 (只重算变化的子树)
  - [ ] 布局约束传播
- [ ] 性能测试

#### Day 5: 布局集成
- [ ] 布局系统与对象树集成
- [ ] `ObjTree::perform_layout()` 实现
- [ ] 最小/最大尺寸约束
- [ ] 溢出处理 (clip/scroll/visible)
- [ ] 综合测试

### Week 7: 脏区域管理

#### Day 1-2: 脏区域系统
- [ ] `core/dirty_region.uya` - 脏区域管理
  - [ ] `struct DirtyRegion`
    - [ ] 精确区域数组
    - [ ] 合并区域数组
    - [ ] 全屏刷新阈值
  - [ ] 结构体方法:
    - [ ] `fn add(self, rect) void` - 添加脏区域
    - [ ] `fn merge(self) void` - 合并优化
    - [ ] `fn should_full_refresh(self) bool`
    - [ ] `fn get_regions(self) &[Rect]`
    - [ ] `fn clear(self) void`
- [ ] 合并算法
  - [ ] 交集合并策略
  - [ ] 面积启发式
  - [ ] 数量限制 (超出则全屏刷新)
- [ ] 单元测试

#### Day 3-4: 渲染调度
- [ ] 渲染流水线
  - [ ] 收集脏区域
  - [ ] 查找受影响对象
  - [ ] 按 Z-order 排序
  - [ ] 执行绘制
  - [ ] 交换缓冲区
- [ ] VSync 支持
- [ ] 帧率控制 (自适应)

#### Day 5: 性能基准
- [ ] 建立性能测试基线
  - [ ] 空渲染帧时间
  - [ ] 全屏刷新时间
  - [ ] 脏区域处理时间
  - [ ] 内存分配/释放时间
- [ ] 编写 `benchmarks/core_bench.uya`

---

## Phase 2: 渲染引擎 (Week 8-10)

### Week 8: 基础渲染

#### Day 1-2: 帧缓冲抽象
- [ ] `platform/disp.uya` - 显示接口
  - [ ] `enum PixelFormat: u8`
    - [ ] RGB565, RGB888, ARGB8888, ARGB4444
    - [ ] L8, A8, I1, I4
  - [ ] `struct FrameBuffer`
    - [ ] buf: `&void` (像素数据)
    - [ ] w, h, stride: `u16`
    - [ ] format: `PixelFormat`
  - [ ] `struct DisplayCtx`
    - [ ] 前后缓冲 (双缓冲)
    - [ ] 显示驱动接口
  - [ ] 结构体方法:
    - [ ] `fn swap_buffers(self) void`
    - [ ] `fn get_back_buffer(self) &FrameBuffer`
    - [ ] `fn clear(self, color) void`

#### Day 3-4: 渲染上下文
- [ ] `render/ctx.uya` - 渲染上下文
  - [ ] `struct RenderCtx`
    - [ ] 目标帧缓冲 (`FrameBuffer`)
    - [ ] 裁剪区域栈 (`clip_stack`)
    - [ ] 当前脏区域 (`dirty`)
    - [ ] 渲染统计 (`RenderStats`)
    - [ ] 渲染模式 (`RenderMode`)
  - [ ] 裁剪管理
    - [ ] `fn push_clip(self, rect) void`
    - [ ] `fn pop_clip(self) void`
    - [ ] `fn current_clip(self) Rect`
  - [ ] 基础绘制 API
    - [ ] `fn draw_pixel(self, x, y, color) void`
    - [ ] `fn fill_rect(self, rect, color) void`
    - [ ] `fn draw_rect(self, rect, color, width) void`
    - [ ] `fn draw_line(self, x1, y1, x2, y2, color, width) void`
    - [ ] `fn draw_circle(self, cx, cy, r, color) void`
    - [ ] `fn fill_circle(self, cx, cy, r, color) void`
    - [ ] `fn fill_round_rect(self, rect, radius, color) void`
    - [ ] `fn draw_round_rect(self, rect, radius, color, width) void`
    - [ ] `fn draw_arc(self, cx, cy, r, start, end, width, color) void`
    - [ ] `fn fill_arc(self, cx, cy, r, start, end, color) void`

#### Day 5: 像素格式优化
- [ ] 各像素格式特化实现
  - [ ] RGB565 快速填充 (32-bit 批量写入)
  - [ ] ARGB8888 Alpha 混合
  - [ ] I1 单色位图绘制
  - [ ] I4 4bpp 索引色
- [ ] 内联汇编优化 (关键路径)
- [ ] 单元测试

### Week 9: 高级绘制

#### Day 1-2: 图像绘制
- [ ] `render/img.uya` - 图像处理
  - [ ] `struct ImageData`
    - [ ] 像素数据引用
    - [ ] 宽度、高度、格式
    - [ ] 引用计数
  - [ ] 图像绘制方法
    - [ ] `fn draw_image(self, x, y, img) void`
    - [ ] `fn draw_image_scaled(self, rect, img) void`
    - [ ] `fn draw_image_clipped(self, dst_rect, img, src_rect) void`
    - [ ] `fn draw_image_rotated(self, x, y, img, angle) void`
  - [ ] 图像格式支持
    - [ ] 原始像素 (RAW)
    - [ ] RLE 编码
    - [ ] BMP 解码
    - [ ] PNG 解码 (简化)
    - [ ] 自定义格式
- [ ] 图像缓存 (`res/cache.uya`)
  - [ ] LRU 缓存策略
  - [ ] 引用计数管理
  - [ ] 内存预算控制

#### Day 3: 文本渲染
- [ ] `render/font.uya` - 字体引擎
  - [ ] `struct Glyph`
    - [ ] 字符编码
    - [ ] 位图数据 (Alpha/单色)
    - [ ] 偏移、步进、尺寸
  - [ ] `struct Font`
    - [ ] 字体元数据 (高度、基线、行高)
    - [ ] 字形查找表
    - [ ] 字距调整表
  - [ ] 字体绘制
    - [ ] `fn draw_text(self, x, y, text, len, font, color) void`
    - [ ] `fn draw_text_aligned(self, rect, text, font, color, align) void`
    - [ ] `fn text_width(self, text, len, font) i16`
  - [ ] 字体格式
    - [ ] 位图字体 (BMFont 兼容)
    - [ ] 矢量字体 (TTF 简化光栅化)
    - [ ] 内置字体 (系统字体)

#### Day 4: 抗锯齿与效果
- [ ] Alpha 混合优化
  - [ ] 快速 Alpha 混合 (查表法)
  - [ ] 预乘 Alpha
- [ ] 圆角抗锯齿
- [ ] 阴影效果
  - [ ]  box-shadow 模拟
  - [ ] 模糊阴影 (简化版)
- [ ] 渐变填充
  - [ ] 线性渐变
  - [ ] 径向渐变

#### Day 5: 批处理渲染
- [ ] `render/batch.uya` - 绘制批处理
  - [ ] `struct DrawCmd` - 绘制命令
  - [ ] `struct DrawBatch`
    - [ ] 命令缓冲区
    - [ ] 合并统计
  - [ ] 优化策略
    - [ ] 同色矩形合并
    - [ ] 相邻水平线合并
    - [ ] 按颜色排序减少状态切换
  - [ ] `fn execute(self, ctx) void`
- [ ] 性能对比测试

### Week 10: GPU 支持与优化

#### Day 1-2: GPU 抽象层
- [ ] `render/gpu.uya` - GPU 抽象
  - [ ] `interface IGpuCtx`
    - [ ] `fn draw_rects(self, rects, colors, count) void`
    - [ ] `fn draw_images(self, cmds, count) void`
    - [ ] `fn fill_rects(self, rects, colors, count) void`
    - [ ] `fn sync(self) void`
  - [ ] GPU 后端
    - [ ] STM32 DMA2D
    - [ ] ESP32-S3 LCD_CAM
    - [ ] NXP PXP
    - [ ] 通用 OpenGL ES 2.0
  - [ ] 自动检测与回退

#### Day 3-4: 零拷贝与 DMA
- [ ] `render/zerocopy.uya` - 零拷贝优化
  - [ ] 双缓冲管理
  - [ ] DMA 异步传输
  - [ ] 脏区域 DMA 传输
  - [ ] 传输完成中断
- [ ] 性能测试
  - [ ] DMA vs CPU memcpy
  - [ ] 帧完成延迟

#### Day 5: 渲染性能调优
- [ ] 内联热点函数
- [ ] SIMD 优化 (如果目标支持)
- [ ] 查找表优化 (Alpha 混合、Sin/Cos)
- [ ] 缓存友好数据布局
- [ ] 完整渲染基准测试

---

## Phase 3: 组件库 (Week 11-13)

### Week 11: 基础组件

#### Day 1: 组件基类
- [ ] `widget/base.uya` - Widget 基类
  - [ ] `enum WidgetState: u8`
    - [ ] Normal, Hovered, Pressed, Disabled, Focused, Checked
  - [ ] `struct Widget: IStyled`
    - [ ] `base: GuiObj`
    - [ ] state: `WidgetState`
    - [ ] anim_state: `&AnimState`
    - [ ] 事件回调函数指针
  - [ ] 结构体方法:
    - [ ] `fn handle_input(self, evt) bool` - 状态机转换
    - [ ] `fn set_state(self, state) void`
    - [ ] `fn is_enabled(self) bool`
    - [ ] `fn set_enabled(self, enabled) void`
    - [ ] Fluent API: `.at()`, `.size()`, `.with_style()`
  - [ ] `interface IStyled` 实现

#### Day 2: 按钮
- [ ] `widget/btn.uya` - 按钮组件
  - [ ] `enum BtnVariant: u8` (Filled/Outlined/Text/Icon/IconText)
  - [ ] `struct Button`
    - [ ] `widget: Widget`
    - [ ] variant, label, icon
    - [ ] click_ctx (闭包环境)
  - [ ] 结构体方法:
    - [ ] `fn filled(text) Button` - 构造宏
    - [ ] `fn outlined(text) Button`
    - [ ] `fn text_only(text) Button`
    - [ ] `fn with_icon(icon, text) Button`
    - [ ] `fn render(self, ctx) void`
    - [ ] `fn on_click(self, callback) &Button`

#### Day 3: 标签
- [ ] `widget/lbl.uya` - 标签组件
  - [ ] `enum TextAlign: u8` (9种对齐方式)
  - [ ] `enum TextOverflow: u8` (Clip/Ellipsis/Wrap/Scroll)
  - [ ] `struct Label`
    - [ ] `widget: Widget`
    - [ ] text, text_len, align, overflow
    - [ ] 行缓存 (layout_dirty 标记)
  - [ ] 结构体方法:
    - [ ] `fn new(text) Label`
    - [ ] `fn set_text(self, text) &Label`
    - [ ] `fn set_align(self, align) &Label`
    - [ ] `fn render(self, ctx) void`
    - [ ] `fn recalc_layout(self, ctx) void` (缓存策略)

#### Day 4: 图像组件
- [ ] `widget/img.uya` - 图像组件
  - [ ] `enum ImageScale: u8` (Fit/Fill/Stretch/Center/Tile)
  - [ ] `struct Image`
    - [ ] `widget: Widget`
    - [ ] src: `&ImageData`
    - [ ] scale_mode
    - [ ] corner_radius
  - [ ] 结构体方法:
    - [ ] `fn from_data(data) Image`
    - [ ] `fn from_file(path) Image` (异步加载)
    - [ ] `fn set_source(self, src) &Image`
    - [ ] `fn render(self, ctx) void`

#### Day 5: 滑块与开关
- [ ] `widget/slider.uya` - 滑块
  - [ ] `struct Slider`
    - [ ] `widget: Widget`
    - [ ] min, max, value, step
    - [ ] orientation (Horizontal/Vertical)
    - [ ] track_color, progress_color, thumb_color
  - [ ] 结构体方法:
    - [ ] `fn new(min, max, value) Slider`
    - [ ] `fn set_value(self, v) void`
    - [ ] `fn get_value(self) i32`
    - [ ] `fn on_change(self, cb) &Slider`
    - [ ] `fn render(self, ctx) void`
    - [ ] `fn handle_drag(self, evt) void`
- [ ] `widget/switch.uya` - 开关
  - [ ] `struct Switch`
    - [ ] `widget: Widget`
    - [ ] checked: bool
    - [ ] anim_progress: f32 (切换动画)
  - [ ] 结构体方法:
    - [ ] `fn new(checked) Switch`
    - [ ] `fn toggle(self) void`
    - [ ] `fn render(self, ctx) void`

### Week 12: 容器与列表

#### Day 1-2: 页面与面板
- [ ] `widget/page.uya` - 页面容器
  - [ ] `struct Page: IContainer`
    - [ ] `widget: Widget`
    - [ ] scrollable: bool
    - [ ] scroll_x, scroll_y
    - [ ] content_size
  - [ ] 结构体方法:
    - [ ] `fn new(name) Page`
    - [ ] `fn add(self, child) &GuiObj`
    - [ ] `fn remove(self, child) void`
    - [ ] `fn set_layout(self, layout) void`
    - [ ] `fn scroll_to(self, x, y) void`
    - [ ] `fn render(self, ctx) void`
- [ ] `widget/panel.uya` - 面板
  - [ ] `struct Panel: IContainer`
    - [ ] `widget: Widget`
  - [ ] 支持背景、边框、阴影、圆角

#### Day 3-4: 列表视图
- [ ] `widget/list.uya` - 列表组件
  - [ ] `struct ListView: IContainer, IScrollable, ISelectable`
    - [ ] `widget: Widget`
    - [ ] items: `[ListItem: MAX_ITEMS]`
    - [ ] item_count, selected_idx
    - [ ] scroll_y, content_h
    - [ ] item_height
  - [ ] 接口实现:
    - [ ] `IScrollable`: scroll_to, scroll_by, content_size
    - [ ] `ISelectable`: select, selected, select_next, select_prev
  - [ ] 结构体方法:
    - [ ] `fn add_item(self, text) void`
    - [ ] `fn remove_item(self, index) void`
    - [ ] `fn clear(self) void`
    - [ ] `fn render(self, ctx) void` (虚拟滚动优化)
- [ ] 单元测试 (1000+ 项性能)

#### Day 5: 网格视图
- [ ] `widget/grid_view.uya` - 网格视图
  - [ ] `struct GridView: IContainer`
    - [ ] `widget: Widget`
    - [ ] columns: u8
    - [ ] item_size: Size
    - [ ] items 数组
  - [ ] 虚拟滚动支持
  - [ ] 单元测试

### Week 13: 高级组件

#### Day 1-2: 图表组件
- [ ] `widget/chart.uya` - 图表
  - [ ] `enum ChartType: u8` (Line/Bar/Area/Scatter/Pie)
  - [ ] `struct Chart`
    - [ ] `widget: Widget`
    - [ ] data_points: `[i32: MAX_POINTS]`
    - [ ] point_count, min_val, max_val
    - [ ] chart_type
  - [ ] 结构体方法:
    - [ ] `fn add_point(self, value) &Chart`
    - [ ] `fn clear(self) &Chart`
    - [ ] `fn render_line(self, ctx) void`
    - [ ] `fn render_bar(self, ctx) void`
    - [ ] `fn render_area(self, ctx) void`
    - [ ] `fn render(self, ctx) void`
- [ ] 单元测试

#### Day 3: 画布组件
- [ ] `widget/canvas.uya` - 画布
  - [ ] `struct Canvas`
    - [ ] `widget: Widget`
    - [ ] draw_buffer: `&FrameBuffer`
    - [ ] dirty: bool
  - [ ] 结构体方法:
    - [ ] `fn begin_draw(self) &RenderCtx` - 获取绘制上下文
    - [ ] `fn end_draw(self) void` - 标记需要刷新
    - [ ] `fn clear(self, color) void`
    - [ ] `fn render(self, ctx) void` (将缓冲复制到屏幕)

#### Day 4-5: 自定义组件示例
- [ ] `examples/custom/gauge.uya` - 仪表盘示例
  - [ ] 演示如何实现自定义组件
  - [ ] 使用所有核心特性
- [ ] `examples/custom/keyboard.uya` - 虚拟键盘
- [ ] 组件集成测试

---

## Phase 4: 高级特性 (Week 14-15)

### Week 14: 动画系统

#### Day 1-2: 补间动画
- [ ] `anim/tween.uya` - 补间动画
  - [ ] `enum EasingType: u8` (25+ 缓动函数)
  - [ ] `enum AnimProp: u8` (X, Y, Width, Height, Opacity, ...)
  - [ ] `union AnimValue` (i32, f32, Color)
  - [ ] `struct Tween`
    - [ ] target, property, from, to
    - [ ] duration, delay, easing
    - [ ] 运行时状态 (elapsed, running, reversed)
    - [ ] repeat, yoyo
  - [ ] 结构体方法:
    - [ ] `fn new(target, prop, to, duration) Tween`
    - [ ] `fn with_easing(self, easing) Tween`
    - [ ] `fn with_delay(self, delay) Tween`
    - [ ] `fn yoyo(self, enabled) Tween`
    - [ ] `fn repeat(self, count) Tween`
    - [ ] `fn on_complete(self, cb) Tween`
    - [ ] `fn on_update(self, cb) Tween`
    - [ ] `fn start(self) void`
    - [ ] `fn update(self, dt) bool`
    - [ ] `fn apply_value(self, value) void`
- [ ] `anim/easing.uya` - 缓动函数实现
  - [ ] 线性、二次、三次、四次
  - [ ] Sine、Expo、Elastic、Back、Bounce
  - [ ] 每种都有 In/Out/InOut 变体

#### Day 3: 动画管理器
- [ ] `anim/timeline.uya` - 动画管理
  - [ ] `struct AnimManager`
    - [ ] tweens: `[Tween: MAX_ANIMATIONS]`
    - [ ] active: `[bool: MAX_ANIMATIONS]`
  - [ ] 结构体方法:
    - [ ] `fn start_tween(self, tween) !i32`
    - [ ] `fn stop_tween(self, id) void`
    - [ ] `fn stop_all(self) void`
    - [ ] `fn update_all(self, dt) void`
    - [ ] `fn is_animating(self) bool`
  - [ ] `@async_fn fn animation_loop(self: &Self, tick_ms: u32) Future<!void>`
- [ ] 性能测试 (32 个并发动画)

#### Day 4-5: 页面过渡
- [ ] 页面切换动画
  - [ ] Slide (Left/Right/Up/Down)
  - [ ] Fade
  - [ ] Scale
  - [ ] Custom
- [ ] 导航栈管理
  - [ ] Push/Pop 动画
  - [ ] 手势返回 (Edge Swipe)

### Week 15: 资源管理与输入

#### Day 1-2: 资源系统
- [ ] `res/cache.uya` - 图像缓存
  - [ ] LRU 策略
  - [ ] 引用计数
  - [ ] 内存预算
  - [ ] 异步加载
- [ ] `res/fs.uya` - 文件系统抽象
  - [ ] `interface IFileSystem`
  - [ ] 内置 ROM FS
  - [ ] FAT FS 适配
  - [ ] 异步读取接口

#### Day 3-4: 输入系统完善
- [ ] `platform/indev.uya` - 输入设备
  - [ ] 触摸屏驱动
    - [ ] 校准算法 (4点/9点)
    - [ ] 中值滤波
    - [ ] 手势识别集成
  - [ ] 鼠标驱动
  - [ ] 键盘驱动
  - [ ] 编码器驱动
- [ ] 输入设备管理器
  - [ ] 多设备同时支持
  - [ ] 设备热插拔

#### Day 5: 平台抽象
- [ ] `platform/tick.uya` - 时钟接口
  - [ ] `fn get_tick_ms() u32`
  - [ ] `fn get_tick_us() u64`
  - [ ] `fn sleep_ms(ms) void`
- [ ] `platform/disp.uya` - 显示驱动接口
  - [ ] 常见驱动适配
    - [ ] ILI9341 (320x240)
    - [ ] ST7789 (240x320)
    - [ ] ST7735 (128x160)
    - [ ] SSD1963 (480x272/800x480)
    - [ ] RA8875 (800x480)

---

## Phase 5: 优化与测试 (Week 16-18)

### Week 16: 性能优化

#### Day 1-2: 渲染优化
- [ ] 脏矩形合并算法优化
- [ ] 绘制批处理增强
- [ ] GPU 加速集成
- [ ] DMA 零拷贝
- [ ] 目标: 60fps @ 480x320 (120MHz MCU)

#### Day 3-4: 内存优化
- [ ] 内存占用分析
- [ ] 结构体字段对齐优化
- [ ] 位域使用
- [ ] 只读数据放 Flash
- [ ] 目标: 空框架 < 8KB RAM

#### Day 5: 启动优化
- [ ] 延迟初始化
- [ ] 预计算查找表
- [ ] 编译期常量展开
- [ ] 目标: 启动 < 10ms

### Week 17: 全面测试

#### Day 1-2: 单元测试
- [ ] 核心模块测试 (覆盖率 > 90%)
  - [ ] Rect 运算 (50+ 用例)
  - [ ] Color 混合 (30+ 用例)
  - [ ] 事件系统 (40+ 用例)
  - [ ] 布局引擎 (50+ 用例)
  - [ ] 动画系统 (30+ 用例)
- [ ] 组件测试
  - [ ] 每个组件 20+ 用例
  - [ ] 状态转换测试
  - [ ] 事件处理测试

#### Day 3-4: 集成测试
- [ ] 完整 UI 场景测试
  - [ ] 10+ 页面复杂应用
  - [ ] 长时间运行稳定性 (24h+)
  - [ ] 内存泄漏检测
- [ ] 并发安全测试
  - [ ] 多线程事件注入
  - [ ] 动画 + 输入并发

#### Day 5: 性能基准
- [ ] 与 LVGL 对比测试
  - [ ] 相同场景帧率对比
  - [ ] 内存占用对比
  - [ ] 启动时间对比
  - [ ] 代码体积对比
- [ ] 编写性能报告

### Week 18: 兼容性测试

#### Day 1-2: 目标平台测试
- [ ] STM32F4xx (Cortex-M4, 168MHz)
- [ ] STM32H7xx (Cortex-M7, 480MHz)
- [ ] ESP32-S3 (Xtensa LX7, 240MHz)
- [ ] Raspberry Pi Pico (RP2040, 133MHz)
- [ ] RISC-V GD32V (108MHz)

#### Day 3-4: 显示适配测试
- [ ] 128x160 (1.8" SPI)
- [ ] 240x320 (3.2" SPI)
- [ ] 320x480 (3.5" SPI)
- [ ] 480x320 (3.5" Parallel)
- [ ] 800x480 (5" RGB)

#### Day 5: 修复与稳定
- [ ] Bug 修复
- [ ] 回归测试
- [ ] 性能微调

---

## Phase 6: 文档与示例 (Week 19-20)

### Week 19: 文档完善

#### Day 1-2: API 文档
- [ ] 自动生成 API 文档
- [ ] 每个公共函数文档注释
- [ ] 使用示例代码
- [ ] 类型说明

#### Day 3-4: 开发指南
- [ ] 快速入门指南
- [ ] 自定义组件教程
- [ ] 主题定制指南
- [ ] 性能优化指南
- [ ] 移植指南

#### Day 5: 架构文档
- [ ] 系统架构图
- [ ] 数据流图
- [ ] 模块依赖图
- [ ] 内存布局图

### Week 20: 示例与发布

#### Day 1-3: 示例应用
- [ ] `examples/demo_clock.uya` - 时钟应用
- [ ] `examples/demo_music.uya` - 音乐播放器
- [ ] `examples/demo_settings.uya` - 设置界面
- [ ] `examples/demo_dashboard.uya` - 数据仪表板
- [ ] `examples/demo_game.uya` - 简单游戏

#### Day 4: 性能演示
- [ ] 帧率计数器
- [ ] 内存使用显示
- [ ] 渲染统计

#### Day 5: 发布准备
- [ ] 版本标记 v1.0.0
- [ ] 发布说明
- [ ] 二进制分发
- [ ] 社区公告

---

## 持续集成与质量保障

### 自动化测试
- [ ] 每次提交自动运行单元测试
- [ ] 每晚集成测试
- [ ] 每周性能基准
- [ ] 代码覆盖率报告

### 代码质量
- [ ] 代码格式化 (uyaFmt)
- [ ] 静态分析
- [ ] 文档完整性检查
- [ ] API 兼容性检查

### 性能监控
- [ ] 帧率追踪
- [ ] 内存使用追踪
- [ ] 渲染时间分析
- [ ] 回归检测

---

## 风险与缓解

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| Uya 编译器 Bug | 中 | 高 | 及时反馈社区，准备 C 回退方案 |
| 目标平台内存不足 | 中 | 高 | 分级组件，可选特性编译开关 |
| GPU 驱动不稳定 | 低 | 中 | 完善的软件回退路径 |
| 性能未达预期 | 中 | 高 | 持续 Profiling，算法优化 |
| 编译时间过长 | 低 | 低 | 增量编译，模块化设计 |

---

## 度量指标

### 代码度量
- 总代码行数目标: < 15000 行 (不含测试和示例)
- 测试代码行数: > 5000 行
- 文档行数: > 3000 行
- 公共 API 数量: ~200 个函数

### 质量度量
- 单元测试覆盖率: > 90%
- 集成测试场景: > 20 个
- 文档覆盖率: 100% 公共 API
- 零编译警告

### 性能度量
- 空框架内存: < 8KB
- 启动时间: < 10ms
- 渲染帧率: 60fps @ 480x320
- 最大组件数: 256+
- 最大动画数: 32 (并发)

---

*TODO 文档结束 - 最后更新: 2026-04-24*
