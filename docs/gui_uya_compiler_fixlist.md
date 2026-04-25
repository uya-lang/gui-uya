# GUI Uya Compiler Fixlist

本清单整理了实现 `Phase 0` 到 `Phase 3` 过程中实际遇到、并且值得在 Uya 编译器侧修复的问题。

目标不是记录“理想语义”，而是记录：

- 真实复现点
- 当前仓库里的临时绕法
- 建议修复顺序
- 修复后应补的编译器回归测试

## 优先级

### P0

1. 泛型 `union` / `Option<T>` 的 C99 代码生成
2. `union` 作为结构体字段的 C99 代码生成
3. 泛型构造器/泛型返回值的实例化与代码生成
4. 结构体比较表达式的 C99 代码生成
8. 接口值（`interface`）的全局/字段初始化与 C99 代码生成

### P1

5. 全局结构体常量初始化依赖其他全局结构体常量
6. const 值调用实例方法时的 `const` 限定传播
9. `&Self` 返回值的链式方法调用 lowering
10. 跨模块同名 `enum` 的 C99 命名空间冲突

### P2

7. 方法名与关键字冲突时的解析策略（如 `union`）
11. split-C / `.uyacache` 构建时相对输出路径 `-o` 的解析基准

## 当前状态总览

> 2026-04-26 更新：当前 GUI 仓库已经可以通过 `make test` / `make build`；下表中的“状态”描述的是编译器 / 语言层问题是否已解决，不是 GUI 仓库是否还能继续开发。

| 编号 | 问题 | 当前状态 | 对当前仓库影响 | 当前仓库处理方式 |
|------|------|----------|----------------|------------------|
| 1 | 泛型 `union` / `Option<T>` C99 代码生成 | 未修复 | 中 | 用专用 `EventOption` 规避 |
| 2 | `union` 作为结构体字段的 C99 代码生成 | 未修复 | 中 | 事件负载改扁平字段 |
| 3 | 泛型构造器 / 泛型返回值实例化 | 未修复 | 中 | 改字面量初始化 / 专用返回类型 |
| 4 | 结构体比较表达式 C99 代码生成 | 未修复 | 低 | 测试使用字段级 helper |
| 5 | 全局结构体常量依赖其他全局结构体常量 | 未修复 | 低 | 默认值改函数返回 |
| 6 | const receiver 限定传播 | 未修复（非阻塞） | 低 | 接受 warning / 局部回避 |
| 7 | 关键字与方法名冲突 | 待语言设计决策 | 低 | 使用 `union_rect` 等替代命名 |
| 8 | `interface` 值的全局 / 字段初始化 | 未修复 | 高 | P3 改为 `type_tag + user_data` 分发 |
| 9 | `&Self` 返回值链式方法 lowering | 未修复 | 中 | Fluent API 拆成多句 |
| 10 | 跨模块同名 `enum` 的 C99 命名空间冲突 | 未修复 | 中 | 组件侧重命名为 `LabelAlign` |
| 11 | split-C 下相对 `-o` 路径解析 | 未修复（driver 层） | 低 | Makefile 统一传绝对路径 |

## 1. 泛型 `union` / `Option<T>` 的 C99 代码生成

### 当前状态

- 状态: 未修复
- 阻塞性: 已有稳定绕法，但仍阻塞回到通用 `Option<T>` 设计

### 现象

`Option<Event>` 这一类“用户自定义结构体作为泛型 union 变体”的实例，在类型检查阶段可以通过，但 C99 代码生成后会出现不完整类型或错误的包装类型。

### GUI 侧复现点

- [event.uya](/home/winger/gui-uya/gui/core/event.uya)
- 当前实现保留了专用 `EventOption`，见 [event.uya](/home/winger/gui-uya/gui/core/event.uya#L57)

### 当时的失败表现

- `Option<Event>` 返回值对应的 C 结构/标签包装未完整生成
- 结果是宿主 C 编译阶段报“不完整类型”

### 当前绕法

- 不使用 `Option<Event>`
- 改为局部专用的 `EventOption`

### 建议修复点

- 单态化时，确保 `Option<T>` 对任意结构体 `T` 都能生成完整的 tagged wrapper
- split-C 场景下，声明与定义必须在所有 TU 中一致可见

### 建议补的编译器测试

- `uya/tests/test_option_struct.uya`
- 覆盖：
  - `Option<MyStruct>` 作为返回值
  - `Option<MyStruct>` 作为局部变量
  - `Option<MyStruct>` 跨模块导入使用

## 2. `union` 作为结构体字段的 C99 代码生成

### 当前状态

- 状态: 未修复
- 阻塞性: 已有稳定绕法，但仍阻塞事件负载恢复为真正 `union` 字段

### 现象

普通 `union` 本身可用，但把 `union` 放进结构体字段里后，C99 代码生成会产出不完整字段类型。

### GUI 侧复现点

- 目标形态原本是 `Event { data: EventData, ... }`
- 当前稳定实现退回到扁平字段，见 [event.uya](/home/winger/gui-uya/gui/core/event.uya#L44)

### 当时的失败表现

- 生成的 C 中类似 `struct tagged_union data;`，但对应定义缺失或不匹配
- 宿主 C 编译时报 “field has incomplete type”

### 当前绕法

- 把事件负载拆成 `point` / `key_code` / `encoder_diff` / `value`
- 仅在出队层面保留 `EventOption`

### 建议修复点

- union 成员作为 struct 字段时，确保布局类型在使用点前完整声明
- 如果语言语义是“tag 仅编译期可见”，C 后端仍需稳定选择可落地的承载表示

### 建议补的编译器测试

- `uya/tests/test_union_struct_field_codegen.uya`
- 结构体中包含：
  - `union` 字段
  - 多个结构体变体
  - 跨模块导出/导入

## 3. 泛型构造器/泛型返回值的实例化与代码生成

### 当前状态

- 状态: 未修复
- 阻塞性: 中等；当前不影响仓库继续开发，但会持续限制 API 设计

### 现象

泛型类型定义通常能过，但泛型构造函数、泛型工厂函数、泛型返回值在部分场景下不稳定。

### GUI 侧复现点

- `ObjPool<T: IGuiObj>` 本体保留下来了，见 [obj_pool.uya](/home/winger/gui-uya/gui/core/obj_pool.uya#L16)
- 但没有保留通用 `obj_pool_new<T>()` 工厂函数
- 使用点改为直接字面量初始化，见 [test_pool.uya](/home/winger/gui-uya/gui/tests/test_pool.uya#L99) 和 [core_bench.uya](/home/winger/gui-uya/gui/benchmarks/core_bench.uya#L14)

### 相关表现

- 泛型工厂函数 `obj_pool_new<T>() -> ObjPool<T>` 会在约束检查或生成阶段失败
- `slice_from_ptr(&values[0], 3)` 这类泛型返回值推断不稳定

### 当前绕法

- `ObjPool<T>` 直接字面量初始化
- `Slice<T>` 直接字面量初始化
- `Buffer.as_slice()` 仍返回专用 `ByteSlice`，见 [buf.uya](/home/winger/gui-uya/gui/res/buf.uya#L63)

### 建议修复点

- 统一泛型返回值的单态化路径
- 约束型泛型 `T: IGuiObj` 在构造函数返回值位置应正常工作
- 推断失败时要么给出明确诊断，要么支持显式类型参数调用

### 建议补的编译器测试

- `uya/tests/test_generic_factory_return.uya`
- `uya/tests/test_generic_struct_method_return.uya`
- 覆盖：
  - `fn new<T>() Container<T>`
  - `fn wrap<T>(x: &T) Slice<T>`
  - 带接口约束的泛型返回值

## 4. 结构体比较表达式的 C99 代码生成

### 当前状态

- 状态: 未修复
- 阻塞性: 低；当前主要影响测试和表达力，不阻塞默认构建

### 现象

语言层允许结构体比较，但部分生成路径会直接输出非法的 C `struct == struct`。

### GUI 侧复现点

- [test_color.uya](/home/winger/gui-uya/gui/tests/test_color.uya#L15)
- [test_rect.uya](/home/winger/gui-uya/gui/tests/test_rect.uya#L8)
- [test_core_types.uya](/home/winger/gui-uya/gui/tests/test_core_types.uya#L15)

### 当前绕法

- 全部改成字段级比较 helper：`color_eq`、`rect_eq`、`point_eq`

### 建议修复点

- 对结构体 `==` / `!=` 在 lowering 时展开成逐字段比较
- 嵌套结构体需要递归展开

### 建议补的编译器测试

- 扩充现有 `uya/tests/test_struct_comparison.uya`
- 再加一个 split-C 回归，确保不是单文件路径才成立

## 5. 全局结构体常量初始化依赖其他全局结构体常量

### 当前状态

- 状态: 未修复
- 阻塞性: 低；当前主要影响默认值组织方式

### 现象

多个结构体常量互相引用时，生成的 C 初始化式不一定是宿主编译器认可的常量表达式。

### GUI 侧复现点

- [style.uya](/home/winger/gui-uya/gui/style/style.uya)
- 当前用 `style_default()` 返回默认样式，而不是 `DEFAULT_STYLE = { background: TRANSPARENT, ... }`

### 当前绕法

- 把静态结构体组合常量改成普通函数返回

### 建议修复点

- 对全局只读结构体常量做常量折叠或内联展开
- 避免在 C 里生成“另一个非 constexpr 聚合对象”作为初始化项

### 建议补的编译器测试

- `uya/tests/test_global_struct_const_init.uya`

## 6. const 值调用实例方法时的限定传播

### 当前状态

- 状态: 未修复（非阻塞）
- 阻塞性: 低；当前主要是 warning 污染和代码生成质量问题

### 现象

`BLACK.lerp(...)`、`POINT_ZERO.add(...)`、`RECT_ZERO.union_rect(...)` 可以工作，但生成 C 时会留下不少 `discarded-qualifiers` 警告。

### GUI 侧复现点

- [test_color.uya](/home/winger/gui-uya/gui/tests/test_color.uya#L44)
- [test_core_types.uya](/home/winger/gui-uya/gui/tests/test_core_types.uya#L16)
- [test_rect.uya](/home/winger/gui-uya/gui/tests/test_rect.uya#L41)

### 当前影响

- 不阻塞 build/test
- 但会污染宿主编译输出，后续更难发现真正的 warning

### 建议修复点

- 对只读 receiver 优先生成 `const T*`
- 或在 lowering 时先复制到局部非常量临时值

### 建议补的编译器测试

- `uya/tests/test_const_receiver_codegen.uya`

## 7. 关键字与方法名冲突

### 当前状态

- 状态: 待语言设计决策
- 阻塞性: 低；如果语言层明确“不支持”，则应从 bug 清单转为语言限制说明

### 现象

如果按直觉把矩形并集方法命名为 `union`，语法层会和关键字冲突。

### GUI 侧复现点

- 当前用了 `union_rect`，见 [rect.uya](/home/winger/gui-uya/gui/core/rect.uya#L81)

### 当前结论

- 这更像语言设计选择，不一定是“bug”
- 但如果语言希望允许方法名与关键字重名，需要在成员访问上下文里收窄词法/语法限制

### 建议

- 若不打算支持，文档要明确保留字不能作方法名
- 若打算支持，优先只支持 `obj.union(...)` 这类成员访问位点

## 8. 接口值（`interface`）的全局/字段初始化与 C99 代码生成

### 当前状态

- 状态: 未修复
- 阻塞性: 高；直接影响更自然的组件回调/适配器架构

### 现象

`interface` 类型在参数、返回值、局部变量层面通常可用，但一旦进入“全局接口值初始化”或“结构体字段初始化为具体实现”路径，当前 C99 后端会产出非法初始化代码。

### GUI 侧复现点

- `Phase 3` 组件库初版尝试在 `widget/*` 内保存 `IGuiRenderCallback` / `IGuiInputCallback`
- 具体踩点在：
  - [btn.uya](/home/winger/uya/gui-uya/gui/widget/btn.uya)
  - [panel.uya](/home/winger/uya/gui-uya/gui/widget/panel.uya)
  - [page.uya](/home/winger/uya/gui-uya/gui/widget/page.uya)
  - [grid_view.uya](/home/winger/uya/gui-uya/gui/widget/grid_view.uya)

### 当时的失败表现

- 全局接口值初始化报 `invalid initializer`
- 结构体字段初始化报 `incompatible types when initializing type 'void *' using type 'struct XxxAdapter'`
- 类型检查能通过，但宿主 C 编译失败

### 当前绕法

- 不在全局或结构体字段里保存接口对象
- `GuiObj` 的 `render_cb` / `input_cb` 统一置空
- 容器层改为 `type_tag + user_data` 的显式分发，见：
  - [panel.uya](/home/winger/uya/gui-uya/gui/widget/panel.uya)
  - [page.uya](/home/winger/uya/gui-uya/gui/widget/page.uya)
  - [grid_view.uya](/home/winger/uya/gui-uya/gui/widget/grid_view.uya)

### 建议修复点

- 明确 `interface` 在 C 后端的承载表示（通常是 `{data, vtable}` 一类聚合）
- 保证：
  - 全局接口值可由具体实现安全初始化
  - 结构体字段可由具体实现安全初始化
  - 字面量初始化与赋值初始化使用一致的 lowering 路径

### 建议补的编译器测试

- `uya/tests/test_interface_global_init.uya`
- `uya/tests/test_interface_field_init.uya`
- 覆盖：
  - `var cb: IFoo = FooImpl{}`
  - `struct S { cb: IFoo }`
  - `return Holder{ cb: FooImpl{} }`

## 9. `&Self` 返回值的链式方法调用 lowering

### 当前状态

- 状态: 未修复
- 阻塞性: 中等；会影响 Fluent API 的可用性和代码风格稳定性

### 现象

链式 Fluent API 在类型检查阶段可以通过，但某些“返回 `&Self` 且结果被丢弃”的路径，会在 C99 生成后退化成 `unknown(...)` 之类无意义调用。

### GUI 侧复现点

- `Chart.add_point(self) &Chart`
- 本次 `Phase 3` 中最初写法类似：

```uya
_ = chart.add_point(2).add_point(6).add_point(3).add_point(8);
```

- 当前已拆成逐句调用，见：
  - [test_widgets.uya](/home/winger/uya/gui-uya/gui/tests/test_widgets.uya)
  - [phase3_smoke.uya](/home/winger/uya/gui-uya/gui/examples/phase3_smoke.uya)

### 当时的失败表现

- 生成的 C 中出现 `unknown(8)` 之类非法调用
- 不是类型错误，而是 lowering / 临时值拼接错误

### 当前绕法

- 不写链式调用
- 拆成多条独立语句：

```uya
_ = chart.add_point(2);
_ = chart.add_point(6);
```

### 建议修复点

- 对“方法返回 `&Self`”的链式表达式统一构建 receiver 临时值
- 明确在“结果被丢弃”的上下文里也要完整保留链式调用副作用
- 避免在中间 lowering 节点留下占位符/未解析调用名

### 建议补的编译器测试

- `uya/tests/test_struct_method_chain.uya`
- 覆盖：
  - `a.bump().bump().bump()`
  - `_ = a.bump().bump()`
  - 结构体方法返回 `&Self`
  - 跨模块方法链

## 10. 跨模块同名 `enum` 的 C99 命名空间冲突

### 当前状态

- 状态: 未修复
- 阻塞性: 中等；会影响模块边界内的常见命名选择

### 现象

不同模块里可以定义同名 `enum`，语义层面可区分，但当前 C99 后端的命名空间隔离不足，会在生成的 C 中出现枚举标签/常量名冲突或缺失。

### GUI 侧复现点

- `render/font.uya` 已有 `TextAlign`
- `widget/lbl.uya` 初版也定义了 `TextAlign`
- 当前稳定实现已把组件侧枚举改名为 `LabelAlign`，见 [lbl.uya](/home/winger/uya/gui-uya/gui/widget/lbl.uya)

### 当时的失败表现

- C 编译阶段报：
  - `TextAlign_BottomLeft undeclared`
  - `TextAlign_Right undeclared`
- 说明生成代码引用到了组件枚举常量，但对应声明/命名已被另一个模块的同名 `enum` 覆盖或污染

### 当前绕法

- 避免跨模块使用同名 `enum`
- 组件侧显式改名为 `LabelAlign`

### 建议修复点

- C 后端对 `enum` 类型名、枚举值名都带上稳定模块前缀
- 保证不同模块的同名顶层声明在 C 命名空间里不会互相覆盖

### 建议补的编译器测试

- `uya/tests/test_cross_module_enum_name_collision.uya`
- 两个模块各自导出 `TextAlign`
- 第三个模块同时导入并使用两者，验证生成的 C 仍可编译

## 11. split-C / `.uyacache` 构建时相对输出路径 `-o` 的解析基准

### 当前状态

- 状态: 未修复（driver 层）
- 阻塞性: 低；当前仓库已通过 Makefile 绝对路径规避

### 现象

在 split-C 路径下执行 `uya build ... -o build/app`，编译器会通过 `make -C .uyacache` 链接；如果 `UYA_OUT` 仍保留相对路径，就会相对 `.uyacache/` 而不是项目根目录解析。

### GUI 侧复现点

- `Phase 3` 默认 smoke 切换到 [Makefile](/home/winger/uya/gui-uya/Makefile)
- 本次显式使用绝对路径 `$(ABS_BUILD_DIR)` 规避

### 当时的失败表现

- 宿主链接阶段报：
  - `cannot open output file build/phase3_smoke: No such file or directory`

### 当前绕法

- 对 split-C 输出统一传绝对路径
- 当前 Makefile 已使用 `ABS_BUILD_DIR`

### 建议修复点

- 如果 `-o` 是相对路径，进入 `.uyacache` 前先相对项目根展开成绝对路径
- 或在 driver 层显式创建相对输出目录并确保基准一致

### 建议补的编译器测试

- `uya/tests/test_relative_output_path_driver.sh`
- 覆盖：
  - 单文件 C 路径
  - split-C 路径
  - `-o build/foo`
  - `-o /abs/path/foo`

## 推荐修复顺序

1. 修 `Option<T>` / 泛型 union 单态化
2. 修 union 结构体字段
3. 修泛型工厂函数/泛型返回值
4. 修接口值初始化 / interface lowering
5. 修结构体比较 lowering
6. 修全局结构体常量初始化
7. 修 `&Self` 链式方法 lowering
8. 修跨模块同名 enum 的命名空间隔离
9. 修 const receiver warning

## 修复完成的验收标准

- GUI 仓库可以把这些绕法撤掉：
  - `EventOption` 改回通用 `Option<Event>`
  - `Event` 改回真正的 union 负载字段
  - `Buffer.as_slice()` 改回 `Slice<byte>`
  - `obj_pool_new<T>()` 恢复为普通泛型工厂函数
  - 测试里移除 `color_eq` / `rect_eq` / `point_eq` 这类字段级比较 helper
  - `Style` 默认值可安全回到全局结构体常量
  - `widget/*` 可恢复接口对象驱动的组件回调绑定，而不是 `type_tag + user_data` 分发
  - `Chart.add_point(...).add_point(...)` 这类 Fluent API 可以直接恢复
  - `widget/lbl.uya` 可安全使用 `TextAlign` 这类与其他模块同名的枚举名
  - `uya build ... -o build/app` 在 split-C 模式下可以稳定输出到项目根的 `build/`
