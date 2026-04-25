# GUI Uya Compiler Fixlist

本清单整理了实现 `Phase 0` 过程中实际遇到、并且值得在 Uya 编译器侧修复的问题。

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

### P1

5. 全局结构体常量初始化依赖其他全局结构体常量
6. const 值调用实例方法时的 `const` 限定传播

### P2

7. 方法名与关键字冲突时的解析策略（如 `union`）

## 1. 泛型 `union` / `Option<T>` 的 C99 代码生成

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

## 推荐修复顺序

1. 修 `Option<T>` / 泛型 union 单态化
2. 修 union 结构体字段
3. 修泛型工厂函数/泛型返回值
4. 修结构体比较 lowering
5. 修全局结构体常量初始化
6. 修 const receiver warning

## 修复完成的验收标准

- GUI 仓库可以把这些绕法撤掉：
  - `EventOption` 改回通用 `Option<Event>`
  - `Event` 改回真正的 union 负载字段
  - `Buffer.as_slice()` 改回 `Slice<byte>`
  - `obj_pool_new<T>()` 恢复为普通泛型工厂函数
  - 测试里移除 `color_eq` / `rect_eq` / `point_eq` 这类字段级比较 helper
  - `Style` 默认值可安全回到全局结构体常量
