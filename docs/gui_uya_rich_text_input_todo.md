# UyaGUI 富文本输入框 TODO 文档

> 版本: v0.1.0  
> 日期: 2026-05-23  
> 状态: 执行中（W2）

> 说明: 本文是 `RichTextInput` 控件的实施路线图，目标是在 UyaGUI 现有 `Widget` / `RenderCtx` / `platform/*` 架构上落地一个参考 Quill 的轻量富文本输入框。本文强调分阶段实施、真实验收和风险前置，不把“未来可能有”的能力当作已完成项。

---

## 目标

- 新增跨后端 `RichTextInput` 控件。
- 提供 Quill 风格的 Delta 变更能力，但内部仍以文档树为真源。
- 覆盖大多数应用场景需要的富文本编辑能力。
- Web 侧补齐 hidden `textarea`、剪贴板和 IME 能力。
- SDL2 / FB / Web 尽量共享同一套编辑、布局、渲染和命令逻辑。

## 非目标

- 当前阶段不做表格、分页、脚注、批注。
- 当前阶段不做协同编辑。
- 当前阶段不做任意 HTML 的高保真导入。
- 当前阶段不做图片、视频、公式等完整 embed 体系。
- 当前阶段不做超大文档虚拟化。

## 当前基线

| 条目 | 状态 | 说明 |
|------|------|------|
| 单行文本输入 | [x] 已有基线 | `gui/widget/lbl.uya` 已有 `TextInput` |
| 输入事件抽象 | [x] 已有基线 | `TextInput` / `KeyDown` / `FocusIn` 等事件已存在 |
| 文本渲染 | [x] 已有基线 | `RenderCtx` / `font.uya` 已可绘制 UTF-8 文本 |
| 多段落编辑模型 | [ ] 缺失 | 还没有文档树与 Delta |
| 富文本布局缓存 | [ ] 缺失 | 还没有 block/line 增量布局 |
| 选区与多行光标 | [ ] 缺失 | 只有单行 cursor |
| 剪贴板抽象 | [ ] 缺失 | 尚无统一 clipboard bridge |
| Web hidden textarea | [ ] 缺失 | 当前 Web 仅有 commit 文本输入 |
| RichText widget API | [ ] 缺失 | 尚无控件对外接口 |
| 富文本测试集 | [ ] 缺失 | 尚无文档/布局/HTML 回归测试 |

## 关键决策

- 主控件命名：`RichTextInput`
- 内部真源：`RichDocument`
- 变更与历史：`RichDelta`
- 内建格式首版只覆盖常用格式，不开放无限样式面
- 工具栏是可选配套，不与编辑器核心强绑定
- Web bridge 是高优先级增强项

## 里程碑总览

| 阶段 | 目标 | 当前状态 |
|------|------|----------|
| W0 | 方案收口、目录与接口定稿 | 已完成 |
| W1 | 文档树、Delta、normalize 基线 | 已完成 |
| W2 | 选择区、编辑命令、历史记录 | 进行中 |
| W3 | 布局与渲染 MVP | 未开始 |
| W4 | `RichTextInput` 控件 API 与交互 MVP | 未开始 |
| W5 | 纯文本粘贴、HTML/Delta 导出 | 未开始 |
| W6 | Web hidden textarea、clipboard、IME | 未开始 |
| W7 | SDL2/桌面增强与示例/测试闭环 | 未开始 |
| W8 | 工具栏、扩展点、性能收尾 | 未开始 |

---

## W0: 方案收口与文件布局

### 目标

在写任何实现前，把文件布局、命名、MVP 能力边界和宿主桥职责先定住，避免后续返工。

### TODO

- [x] 确认最终目录布局
  - [x] `gui/richtext/document.uya`
  - [x] `gui/richtext/delta.uya`
  - [x] `gui/richtext/selection.uya`
  - [x] `gui/richtext/layout.uya`
  - [x] `gui/richtext/render.uya`
  - [x] `gui/richtext/history.uya`
  - [x] `gui/widget/rich_text_input.uya`
  - [x] `gui/platform/text_host.uya`
- [x] 确认首版内建格式集合
  - [x] `bold`
  - [x] `italic`
  - [x] `underline`
  - [x] `strike`
  - [x] `inline code`
  - [x] `link`
  - [x] `paragraph`
  - [x] `heading1/2/3`
  - [x] `quote`
  - [x] `bullet list`
  - [x] `ordered list`
  - [x] `code block`
- [x] 确认首版不做的内容
  - [x] 图片
  - [x] Markdown 双向
  - [x] 表格
  - [x] 任意颜色字号
- [x] 确认 `RichDocument` 为真源，`RichDelta` 为变更与存储格式
- [x] 确认线性文档投影规则
  - [x] 每个 block 都带尾部 `BlockBreak`
  - [x] block attrs 绑定在 `BlockBreak`
  - [x] Delta 计数单位以线性原子为准
- [x] 确认 Web bridge 是否并入现有 `web_host.c`

### 验收

- [x] 目录、命名、模块边界定稿
- [x] 文档树 vs Delta 的角色不再摇摆
- [x] 首版范围有明确边界

---

## W1: 文档树、Delta、Normalize 基线

### 目标

建立可编辑、可序列化、可回放的核心富文本数据模型。

### TODO

- [x] 新增 `RichBlockType`
- [x] 新增 `RichInlineMarks`
- [x] 新增 `RichSpan`
- [x] 新增 `RichBlock`
- [x] 新增 `RichDocument`
- [x] 实现 block/span 基础增删改
- [x] 实现 UTF-8 安全 split/merge
- [x] 实现 `RichPos <-> linear_offset` 统一转换
- [x] 定义并实现 snapshot delta 规范
  - [x] snapshot 只输出 `InsertText` / `InsertBreak`
  - [x] snapshot 始终包含尾部 `BlockBreak`
  - [x] block attrs 挂在 `InsertBreak`
- [x] 定义并实现 patch delta 规范
  - [x] patch 允许 `Retain` / `Insert*` / `Delete`
  - [x] patch 不允许 `Toggle*` 命令型 op
- [x] 实现 normalize
  - [x] 合并相邻同样式 span
  - [x] 移除非法空 span
  - [x] 保证文档至少有一个 block
- [x] 新增 `RichDelta`
- [x] 实现 `apply_delta()`
- [x] 实现 `document_to_delta()`
- [x] 新增 plain text 导出

### 验收

- [x] 可以从纯文本构造基础文档
- [x] 可以把文档导出成 Delta
- [x] 可以把 Delta 回放回文档
- [x] 线性偏移与 block/span 位置互转结果一致
- [x] snapshot delta 结果稳定、可比较、无 toggle 型语义
- [x] 复杂 UTF-8 字符不会被截断成非法字节序列

---

## W2: 选择区、编辑命令、历史记录

### 目标

让编辑器具备最小可用的“光标 + 选区 + 命令”能力，而不是只能静态渲染富文本。

### TODO

- [x] 新增 `RichPos`
- [x] 新增 `RichRange`
- [x] 实现选区归一化
- [x] 实现左右移动
- [ ] 实现上下移动
- [x] 实现段首段尾移动
- [ ] 实现选区删除
- [ ] 实现插入文本
- [ ] 实现退格 / 前删
- [ ] 实现回车拆段 / 合段
- [ ] 实现 block type 切换
- [ ] 实现 inline mark toggle
- [ ] 新增 `RichHistory`
- [ ] 实现 undo / redo
- [ ] 实现连续输入合并策略
- [ ] 命令执行后统一下沉为 patch delta

### 验收

- [ ] 用程序化命令即可完成一轮完整编辑
- [ ] undo / redo 不破坏文档结构
- [ ] 切换样式后 span 会正确归并
- [ ] 历史栈中不出现 `Toggle*` 这类命令式持久化记录
- [ ] 回车、退格在列表和普通段落中的语义明确

---

## W3: 布局与渲染 MVP

### 目标

先把多段落文本正确地排出来、选出来、画出来，保证控件具备基本可视化能力。

### TODO

- [ ] 新增 `RichLayoutCache`
- [ ] 新增 `RichVisualLine`
- [ ] 实现 block 测量
- [ ] 实现软换行
- [ ] 实现标题、引用、列表、代码块的基础段落样式
- [ ] 实现选区矩形切分
- [ ] 实现光标绘制
- [ ] 实现 placeholder 绘制
- [ ] 实现 block 级增量重排
- [ ] 实现点击命中测试
- [ ] 实现 `point -> RichPos`

### 验收

- [ ] 多段落内容能正确换行
- [ ] 选区高亮覆盖范围正确
- [ ] 光标在长文本中可定位
- [ ] 宽度变化后布局可重新计算

---

## W4: `RichTextInput` 控件 API 与交互 MVP

### 目标

把核心编辑器包成真正可用的 Widget，让业务层可以创建、设置文本、接回调。

### TODO

- [ ] 新增 `WIDGET_TYPE_RICH_TEXT_INPUT`
- [ ] 新增 `gui/widget/rich_text_input.uya`
- [ ] 提供 `new()`
- [ ] 提供 `obj()`
- [ ] 提供 `set_plain_text()`
- [ ] 提供 `plain_text()`
- [ ] 提供 `set_delta()`
- [ ] 提供 `export_delta()`
- [ ] 提供 `export_html()`
- [ ] 提供 `set_readonly()`
- [ ] 提供 `exec()`
- [ ] 绑定 render/input callback
- [ ] 处理 focus / blur
- [ ] 处理鼠标点击定位
- [ ] 处理拖拽选区
- [ ] 处理滚动跟随光标
- [ ] 更新 `gui/widget/base.uya`
  - [ ] 新增 `WIDGET_TYPE_RICH_TEXT_INPUT`
  - [ ] 更新 opaque culling 白名单
- [ ] 更新手工 render/type dispatch 点
  - [ ] `gui/widget/panel.uya`
  - [ ] `gui/widget/page.uya`
  - [ ] 其他手工 `type_tag` 分发点

### 验收

- [ ] 控件可加入现有 `Page` / `Panel`
- [ ] 业务层可读写纯文本和 Delta
- [ ] 焦点、点击、拖拽基本工作正常
- [ ] 文本变更后能稳定触发 `on_change`
- [ ] 现有容器中可正常渲染，不因遗漏 type switch 而失效

---

## W5: 导出、粘贴与内容清洗

### 目标

补齐首版可交付所需的数据出入口，避免“能编辑但无法可靠提交或预览”。

### TODO

- [ ] 实现 HTML 导出
- [ ] 受限 HTML 标签映射
- [ ] HTML URL 白名单校验
- [ ] 实现 plain text 粘贴
- [ ] 粘贴前统一清洗换行和制表符
- [ ] 首版仅保留文本，不保留外部 HTML 样式
- [ ] 实现 `Ctrl/Cmd + V` 对应命令路径
- [ ] 导出 snapshot delta

### 验收

- [ ] 可以导出稳定 HTML
- [ ] 粘贴富文本来源时不会带入危险标签
- [ ] 多段纯文本粘贴后结构正确
- [ ] 导出的 snapshot delta 为规范化格式，可直接做 golden 对比

---

## W6: Web hidden textarea、clipboard、IME

### 目标

把 Web 端体验从“可输入”提升到“像一个真实编辑器”，这是整体可用性的关键阶段。

### TODO

- [ ] 在 `gui/platform/web/shell.html` 的 overlay 内创建隐藏 `textarea`
- [ ] 焦点进入时同步激活隐藏输入层
- [ ] 焦点离开时回收隐藏输入层
- [ ] 同步 caret rect 到宿主
- [ ] 监听 `beforeinput`
- [ ] 监听 `input`
- [ ] 监听 `compositionstart`
- [ ] 监听 `compositionupdate`
- [ ] 监听 `compositionend`
- [ ] 监听 `paste`
- [ ] 监听 `copy`
- [ ] 监听 `cut`
- [ ] 为富文本新增 `IRichTextHostBridge`
- [ ] 明确输入分流
  - [ ] 短文本 commit 可继续走现有 `TextInput` 路径
  - [ ] paste / copy / cut / composition 不走当前 `WEB_EVT_TEXT_INPUT`
  - [ ] 长文本通过 bridge 直调或 side buffer 进入编辑器
- [ ] 验证移动端虚拟键盘是否能弹出

### 验收

- [ ] 中文输入法基本可用
- [ ] 复制/剪切/粘贴可在浏览器里正常工作
- [ ] 超过 `96` 字节的粘贴内容不会因现有事件载荷上限而被截断
- [ ] composition 生命周期不会被压扁成单次 `TextInput`
- [ ] 失焦后不会残留错误输入状态
- [ ] 不破坏现有 Web backend 基本输入链路

---

## W7: SDL2/桌面增强、示例与测试闭环

### 目标

把富文本控件接入现有模拟器示例与测试体系，形成可回归能力。

### TODO

- [ ] 为 SDL2 添加最小 clipboard bridge
- [ ] 评估 SDL IME caret 定位是否纳入当前版本
- [ ] 新增 demo 页面
- [ ] 在 `gui/sim/app.uya` 或独立 demo 中接入 `RichTextInput`
- [ ] 新增文档层单测
- [ ] 新增布局层单测
- [ ] 新增 HTML 序列化单测
- [ ] 新增 delta 规范化单测
- [ ] 新增 Widget 交互单测
- [ ] 新增 Web smoke case

### 验收

- [ ] demo 能实际编辑多段富文本
- [ ] 主流程测试可在本地和 CI 跑通
- [ ] Web smoke 至少覆盖输入、导出和一次粘贴路径

---

## W8: 工具栏、扩展点、性能收尾

### 目标

让控件从“能用”进化到“好集成、可演进”，同时把后续扩展点预留好。

### TODO

- [ ] 新增 `RichToolbar`
- [ ] 新增当前格式态查询 API
- [ ] 新增格式注册表
- [ ] 预留 mention/image/embed 接口
- [ ] 补齐主题 token
- [ ] 做 10 KB / 50 KB / 200 KB 文档基准
- [ ] 优化连续输入历史合并策略
- [ ] 优化局部 dirty 重绘

### 验收

- [ ] 工具栏能驱动常用格式
- [ ] 大多数常规场景编辑保持流畅
- [ ] 扩展格式不需要推翻现有文档模型

---

## 测试清单

### 单元测试

- [ ] UTF-8 多字节插入/删除
- [ ] block 拆分与合并
- [ ] span normalize
- [ ] Delta apply / invert
- [ ] undo / redo
- [ ] 段落和列表切换
- [ ] HTML 导出安全清洗

### 交互测试

- [ ] 单击定位
- [ ] 拖拽选区
- [ ] Shift 扩展选区
- [ ] `Ctrl/Cmd + B`
- [ ] `Ctrl/Cmd + Z`
- [ ] `Ctrl/Cmd + V`
- [ ] 回车拆段
- [ ] 空列表项回车退出列表

### Web 专项测试

- [ ] 中文 IME 输入
- [ ] 浏览器复制/剪切/粘贴
- [ ] hidden textarea 焦点收放
- [ ] 移动端软键盘弹出
- [ ] 页面 resize 后光标和选区仍正确

## 风险列表

- [ ] UTF-8 与逻辑字符边界处理出错
- [ ] 线性偏移与 block/span 偏移换算不一致
- [ ] 选区命中测试与布局缓存不同步
- [ ] Web composition 事件时序处理不完整
- [ ] Web 长粘贴误走现有 `TextInput` 事件主路径而被截断
- [ ] HTML 清洗过宽或过严
- [ ] 代码块/列表在拆段合段时语义紊乱

## 推荐落地顺序

1. 先做 `W1 + W2`，把编辑器核心和历史系统定稳。
2. 再做 `W3 + W4`，让控件先可视、可点、可编辑。
3. 然后做 `W5`，保证数据可提交、可导出。
4. 最后优先做 `W6`，把 Web 端真正补成可用体验。
5. `W7 + W8` 作为交付前打磨与扩展期。

## 首版发布门槛

- [ ] 支持多段落编辑
- [ ] 支持常用 inline marks
- [ ] 支持标题、引用、列表、代码块
- [ ] 支持撤销重做
- [ ] 支持 plain text / Delta / HTML 导出
- [ ] Web 支持 clipboard 和 IME 基本可用
- [ ] 有 demo、有测试、有 smoke
