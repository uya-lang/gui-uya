# GUI Uya Compiler Fixlist

本清单整理了实现 `Phase 0` 到 `Phase 3` 过程中实际遇到、并且值得在 Uya 编译器侧修复的问题。

目标不是记录“理想语义”，而是记录：

- 真实复现点
- 当前仓库里的临时绕法
- 建议修复顺序
- 修复后应补的编译器回归测试

> 2026-04-26 之后，这份文档已经主要转为“历史问题 + 当前回归状态”记录。
> 除 `#7` 外，其余条目已在编译器 / driver 侧修复；GUI 仓库是否立即撤销绕法，取决于组件侧重构节奏。

## 优先级

以下优先级是历史修复顺序，保留用于追溯，不代表当前仍待实现。

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

> 2026-04-26 更新：当前 GUI 仓库已经可以通过 `make test` / `make build`；下表中的“状态”描述的是编译器 / 语言层问题是否已解决，不是 GUI 仓库是否还能继续开发。当前编译器侧已修复 `1/2/3/4/5/6/8/9/10/11`，GUI 仓库里的对应绕法可以按需逐步撤销。

| 编号 | 问题 | 当前状态 | 对当前仓库影响 | 当前仓库处理方式 |
|------|------|----------|----------------|------------------|
| 1 | 泛型 `union` / `Option<T>` C99 代码生成 | 已修复（编译器） | 低 | 历史绕法，可回撤 `EventOption` |
| 2 | `union` 作为结构体字段的 C99 代码生成 | 已修复（编译器） | 低 | 历史绕法，可恢复真正 `union` 字段 |
| 3 | 泛型构造器 / 泛型返回值实例化 | 已修复（编译器） | 低 | 历史绕法，可回到通用工厂 / 返回类型 |
| 4 | 结构体比较表达式 C99 代码生成 | 已修复（编译器） | 低 | 历史绕法，可移除字段级 helper |
| 5 | 全局结构体常量依赖其他全局结构体常量 | 已修复（编译器） | 低 | 历史绕法，可回到全局组合常量 |
| 6 | const receiver 限定传播 | 已修复（编译器） | 低 | 历史绕法，可去掉局部规避 |
| 7 | 关键字与方法名冲突 | 语言限制（当前不支持） | 低 | 使用 `union_rect` 等替代命名 |
| 8 | `interface` 值的全局 / 字段初始化 | 已修复（编译器） | 低 | 历史绕法，可回到接口对象字段 / 全局绑定 |
| 9 | `&Self` 返回值链式方法 lowering | 已修复（编译器） | 低 | 历史绕法，可恢复 Fluent API |
| 10 | 跨模块同名 `enum` 的 C99 命名空间冲突 | 已修复（编译器） | 低 | 历史绕法，可恢复原始枚举命名 |
| 11 | split-C 下相对 `-o` 路径解析 | 已修复（driver 层） | 低 | 历史绕法，可直接使用相对 `-o` |

## 1. 泛型 `union` / `Option<T>` 的 C99 代码生成

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；可回到通用 `Option<T>` 设计

### 现象

`Option<Event>` 这一类“用户自定义结构体作为泛型 union 变体”的实例，在类型检查阶段可以通过，但 C99 代码生成后会出现不完整类型或错误的包装类型。

### GUI 侧复现点

- [event.uya](/home/winger/gui-uya/gui/core/event.uya)
- 当前实现保留了专用 `EventOption`，见 [event.uya](/home/winger/gui-uya/gui/core/event.uya#L57)

### 当时的失败表现

- `Option<Event>` 返回值对应的 C 结构/标签包装未完整生成
- 结果是宿主 C 编译阶段报“不完整类型”

### 历史绕法

- 不使用 `Option<Event>`
- 改为局部专用的 `EventOption`

### 修复结果

- `Option<T>` 对结构体 payload 的单态化已打通
- split-C 下声明 / 定义一致性已通过回归验证

### 已补的编译器测试

- `uya/tests/test_option_struct.uya`
- `tests/verify_gui_fixlist_codegen.sh`
- 已覆盖：
  - `Option<MyStruct>` 作为返回值
  - `Option<MyStruct>` 作为局部变量
  - `Option<MyStruct>` 跨模块导入使用
  - split-C 路径

## 2. `union` 作为结构体字段的 C99 代码生成

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；可恢复事件负载的真正 `union` 字段

### 现象

普通 `union` 本身可用，但把 `union` 放进结构体字段里后，C99 代码生成会产出不完整字段类型。

### GUI 侧复现点

- 目标形态原本是 `Event { data: EventData, ... }`
- 当前稳定实现退回到扁平字段，见 [event.uya](/home/winger/gui-uya/gui/core/event.uya#L44)

### 当时的失败表现

- 生成的 C 中类似 `struct tagged_union data;`，但对应定义缺失或不匹配
- 宿主 C 编译时报 “field has incomplete type”

### 历史绕法

- 把事件负载拆成 `point` / `key_code` / `encoder_diff` / `value`
- 仅在出队层面保留 `EventOption`

### 修复结果

- `union` 作为结构体字段时，C99 后端已能在使用点前生成完整承载类型
- 结构体内嵌 `union` 的初始化 / match / 返回路径已可正常落地

### 已补的编译器测试

- `uya/tests/test_union_struct_field_codegen.uya`
- 已覆盖：
  - `union` 字段
  - 多个结构体变体
  - 结构体字段上的初始化与解构

## 3. 泛型构造器/泛型返回值的实例化与代码生成

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；通用工厂函数 / 泛型返回值可恢复

### 现象

泛型类型定义通常能过，但泛型构造函数、泛型工厂函数、泛型返回值在部分场景下不稳定。

### GUI 侧复现点

- `ObjPool<T: IGuiObj>` 本体保留下来了，见 [obj_pool.uya](/home/winger/gui-uya/gui/core/obj_pool.uya#L16)
- 但没有保留通用 `obj_pool_new<T>()` 工厂函数
- 使用点改为直接字面量初始化，见 [test_pool.uya](/home/winger/gui-uya/gui/tests/test_pool.uya#L99) 和 [core_bench.uya](/home/winger/gui-uya/gui/benchmarks/core_bench.uya#L14)

### 相关表现

- 泛型工厂函数 `obj_pool_new<T>() -> ObjPool<T>` 会在约束检查或生成阶段失败
- `slice_from_ptr(&values[0], 3)` 这类泛型返回值推断不稳定

### 历史绕法

- `ObjPool<T>` 直接字面量初始化
- `Slice<T>` 直接字面量初始化
- `Buffer.as_slice()` 仍返回专用 `ByteSlice`，见 [buf.uya](/home/winger/gui-uya/gui/res/buf.uya#L63)

### 修复结果

- 泛型工厂函数与泛型方法返回值的单态化路径已统一
- 带接口约束的泛型返回值已能正常代码生成

### 已补的编译器测试

- `uya/tests/test_generic_factory_return.uya`
- `uya/tests/test_generic_struct_method_return.uya`
- 已覆盖：
  - `fn new<T>() Container<T>`
  - 泛型结构体方法返回值
  - 带接口约束的泛型返回值

## 4. 结构体比较表达式的 C99 代码生成

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；可直接使用结构体比较表达式

### 现象

语言层允许结构体比较，但部分生成路径会直接输出非法的 C `struct == struct`。

### GUI 侧复现点

- [test_color.uya](/home/winger/gui-uya/gui/tests/test_color.uya#L15)
- [test_rect.uya](/home/winger/gui-uya/gui/tests/test_rect.uya#L8)
- [test_core_types.uya](/home/winger/gui-uya/gui/tests/test_core_types.uya#L15)

### 历史绕法

- 全部改成字段级比较 helper：`color_eq`、`rect_eq`、`point_eq`

### 修复结果

- 结构体 `==` / `!=` 已在 lowering / codegen 路径上展开为可落地的 C99 比较
- 嵌套结构体与数组字段场景已纳入回归

### 已补的编译器测试

- 扩充现有 `uya/tests/test_struct_comparison.uya`
- 已覆盖：
  - 普通结构体比较
  - 嵌套结构体比较
  - 数组字段比较

## 5. 全局结构体常量初始化依赖其他全局结构体常量

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；可直接回到全局组合结构体常量

### 现象

多个结构体常量互相引用时，生成的 C 初始化式不一定是宿主编译器认可的常量表达式。

### GUI 侧复现点

- [style.uya](/home/winger/gui-uya/gui/style/style.uya)
- 当前用 `style_default()` 返回默认样式，而不是 `DEFAULT_STYLE = { background: TRANSPARENT, ... }`

### 历史绕法

- 把静态结构体组合常量改成普通函数返回

### 修复结果

- 全局结构体常量对其他全局结构体常量的引用已可正确展开
- C99 初始化式不再退化成宿主编译器拒绝的非法聚合引用

### 已补的编译器测试

- `uya/tests/test_global_struct_const_init.uya`

## 6. const 值调用实例方法时的限定传播

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；const receiver warning 已可清理

### 现象

`BLACK.lerp(...)`、`POINT_ZERO.add(...)`、`RECT_ZERO.union_rect(...)` 可以工作，但生成 C 时会留下不少 `discarded-qualifiers` 警告。

### GUI 侧复现点

- [test_color.uya](/home/winger/gui-uya/gui/tests/test_color.uya#L44)
- [test_core_types.uya](/home/winger/gui-uya/gui/tests/test_core_types.uya#L16)
- [test_rect.uya](/home/winger/gui-uya/gui/tests/test_rect.uya#L41)

### 修复后效果

- 相关调用仍可正常工作
- `discarded-qualifiers` 定向回归已通过，宿主 warning 污染已清理

### 修复结果

- const receiver 的生成路径已调整，避免再丢失限定信息

### 已补的编译器测试

- `uya/tests/test_const_receiver_codegen.uya`

## 7. 关键字与方法名冲突

### 当前状态

- 状态: 语言限制（当前不支持）
- 阻塞性: 低；当前按语言限制处理，若后续希望支持再进入语言设计

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

- 状态: 已修复（编译器）
- 阻塞性: 已解除；可恢复接口对象字段 / 全局初始化

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

### 历史绕法

- 不在全局或结构体字段里保存接口对象
- `GuiObj` 的 `render_cb` / `input_cb` 统一置空
- 容器层改为 `type_tag + user_data` 的显式分发，见：
  - [panel.uya](/home/winger/uya/gui-uya/gui/widget/panel.uya)
  - [page.uya](/home/winger/uya/gui-uya/gui/widget/page.uya)
  - [grid_view.uya](/home/winger/uya/gui-uya/gui/widget/grid_view.uya)

### 修复结果

- `interface` 的全局初始化、结构体字段初始化、返回值初始化已经统一走可落地的 `{data, vtable}` lowering
- 具体实现装箱到接口值的路径已在单文件与定向脚本中验证

### 已补的编译器测试

- `uya/tests/test_interface_global_init.uya`
- `uya/tests/test_interface_field_init.uya`
- `tests/verify_gui_fixlist_codegen.sh`
- 已覆盖：
  - `var cb: IFoo = FooImpl{}`
  - `struct S { cb: IFoo }`
  - `return Holder{ cb: FooImpl{} }`

## 9. `&Self` 返回值的链式方法调用 lowering

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；Fluent API 可直接恢复

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

### 历史绕法

- 不写链式调用
- 拆成多条独立语句：

```uya
_ = chart.add_point(2);
_ = chart.add_point(6);
```

### 修复结果

- `&Self` 风格的链式方法调用已可稳定 lower 到 C99
- 结果被丢弃、泛型链式、字面量链式场景已纳入回归

### 已补的编译器测试

- `uya/tests/test_struct_method_chain.uya`
- 已覆盖：
  - `a.bump().bump().bump()`
  - 结构体方法返回 `&Self`
  - 泛型 / 字面量 / 括号链式调用

## 10. 跨模块同名 `enum` 的 C99 命名空间冲突

### 当前状态

- 状态: 已修复（编译器）
- 阻塞性: 已解除；跨模块同名 `enum` 可共存

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

### 历史绕法

- 避免跨模块使用同名 `enum`
- 组件侧显式改名为 `LabelAlign`

### 修复结果

- C99 后端已为 `enum` 类型名 / 枚举值生成稳定模块前缀
- `use ... as ...` 导入别名与跨模块同名 `enum` 已可同时工作

### 已补的编译器测试

- `tests/verify_gui_fixlist_codegen.sh`
- 已覆盖：
  - 两个模块各自导出 `TextAlign`
  - 第三个模块同时导入并使用两者
  - `use ... as ...` 别名路径

## 11. split-C / `.uyacache` 构建时相对输出路径 `-o` 的解析基准

### 当前状态

- 状态: 已修复（driver 层）
- 阻塞性: 已解除；相对 `-o` 可直接稳定使用

### 现象

在 split-C 路径下执行 `uya build ... -o build/app`，编译器会通过 `make -C .uyacache` 链接；如果 `UYA_OUT` 仍保留相对路径，就会相对 `.uyacache/` 而不是项目根目录解析。

### GUI 侧复现点

- `Phase 3` 默认 smoke 切换到 [Makefile](/home/winger/uya/gui-uya/Makefile)
- 本次显式使用绝对路径 `$(ABS_BUILD_DIR)` 规避

### 当时的失败表现

- 宿主链接阶段报：
  - `cannot open output file build/phase3_smoke: No such file or directory`

### 历史绕法

- 对 split-C 输出统一传绝对路径
- 当前 Makefile 已使用 `ABS_BUILD_DIR`

### 修复结果

- driver 进入 `.uyacache` 前已把相对 `-o` 展开成绝对路径
- 单文件 / split-C 两条构建路径都会预创建输出目录

### 已补的编译器测试

- `tests/verify_gui_fixlist_codegen.sh`
- 已覆盖：
  - 单文件 C 路径
  - split-C 路径
  - `-o build/foo`
  - 相对项目根输出目录

## 历史修复顺序

当前只剩 `#7` 处于语言设计待决状态；以下顺序保留作追溯。

1. 修 `Option<T>` / 泛型 union 单态化
2. 修 union 结构体字段
3. 修泛型工厂函数/泛型返回值
4. 修接口值初始化 / interface lowering
5. 修结构体比较 lowering
6. 修全局结构体常量初始化
7. 修 `&Self` 链式方法 lowering
8. 修跨模块同名 enum 的命名空间隔离
9. 修 const receiver warning

## 当前验收结论

以下目标中，除 `#7` 关联的关键字命名策略外，其余项在编译器侧都已经具备撤销 GUI 绕法的条件。

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
