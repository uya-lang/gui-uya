# OpenAI Chat 人机斗地主 Demo TODO

> 配套设计文档：`docs/openai_doudizhu_demo_design.md`  
> 状态：待开始  
> 原则：先离线可玩，再接 OpenAI；默认测试不访问网络。

## 0. 开工前确认

- [ ] 确认 MVP 不实现飞机、四带二、完整积分倍数。
- [ ] 确认 OpenAI 仅用于电脑决策，不参与规则判定。
- [ ] 确认 OpenAI HTTPS 调用只在 Linux SDL2 模拟器启用。
- [ ] 确认 `OPENAI_MODEL` 可配置，示例值使用 `gpt-5.4-mini`。
- [ ] 确认 libcurl 是可选依赖，缺失时自动编译 stub。

验收：

- [ ] 设计文档待确认项全部有结论。

## 1. 规则层

- [ ] 新建 `gui/examples/doudizhu/rules.uya`。
- [ ] 定义 rank、card、hand、combo、action、game phase、game state。
- [ ] 实现 `ddz_deck_init()`，生成 54 张唯一牌。
- [ ] 实现 `ddz_deck_shuffle()`，使用可重复 seed。
- [ ] 实现 `ddz_game_deal()`，分发 `17/17/17 + 3`。
- [ ] 实现 `ddz_hand_sort()`。
- [ ] 实现 `ddz_hand_recount()`。
- [ ] 实现 `ddz_combo_detect()`。
- [ ] 实现 `ddz_combo_can_beat()`。
- [ ] 实现 `ddz_generate_bid_actions()`。
- [ ] 实现 `ddz_generate_play_actions()`。
- [ ] 实现 `ddz_game_apply_bid()`。
- [ ] 实现 `ddz_game_apply_action()`。
- [ ] 实现 `ddz_game_restart()`。
- [ ] 实现 action label 格式化。

验收：

- [ ] 规则层不 import UI、OpenAI、sim app。
- [ ] 所有公开函数对非法输入有确定返回值。
- [ ] `DDZ_MAX_ACTIONS` 溢出时设置 `truncated`，不写越界。

测试：

- [ ] deck 唯一性。
- [ ] 发牌数量。
- [ ] rank_counts 正确。
- [ ] 单、对、三、三带一、三带一对。
- [ ] 顺子、连对、炸弹、火箭。
- [ ] 非法顺子和非法连对。
- [ ] 炸弹/火箭比较。
- [ ] pass 规则。
- [ ] apply action 后手牌和轮转正确。
- [ ] GameOver 判定。

## 2. 本地启发式 AI

- [ ] 新建 `gui/examples/doudizhu/ai.uya`。
- [ ] 定义 `DdzAiSource`：`Local | OpenAI | Fallback`。
- [ ] 定义 `DdzAiStatus`：`Idle | Waiting | Ready | Failed | Cooldown`。
- [ ] 实现叫分评分函数。
- [ ] 实现主动出牌启发式。
- [ ] 实现跟牌启发式。
- [ ] 实现炸弹/火箭保守使用策略。
- [ ] 实现 action id 校验函数。
- [ ] 实现连续失败计数和冷却计数。

验收：

- [ ] 启发式永远返回 legal_actions 中已有的 action。
- [ ] legal_actions 为空时返回明确失败状态。
- [ ] 同一 seed 和同一局面下结果稳定。

测试：

- [ ] 强牌叫 3 分。
- [ ] 弱牌不叫或叫低分。
- [ ] 主动出牌选择最小普通动作。
- [ ] 跟牌选择最小可压动作。
- [ ] 非必要场景不炸。
- [ ] 剩余牌少时允许炸。
- [ ] 非法 OpenAI action id 兜底。
- [ ] 连续失败触发冷却。

## 3. 离线 UI Demo

- [ ] 新建 `gui/examples/demo_doudizhu.uya`。
- [ ] 定义 `DdzPageRetained`。
- [ ] 实现 demo 初始化和资源分配。
- [ ] 实现 Canvas 桌面背景绘制。
- [ ] 实现玩家区域绘制。
- [ ] 实现底牌区绘制。
- [ ] 实现上一手牌绘制。
- [ ] 实现人类手牌绘制。
- [ ] 实现选中牌上移效果。
- [ ] 实现叫分按钮。
- [ ] 实现 `提示`、`出牌`、`不要`、`重开` 按钮。
- [ ] 实现点击手牌命中检测。
- [ ] 实现人类叫分交互。
- [ ] 实现人类出牌交互。
- [ ] 实现 AI 自动叫分和出牌。
- [ ] 实现状态文案。
- [ ] 实现重开时清理选牌和 AI 状态。

验收：

- [ ] 无 OpenAI 环境变量时可完整玩一局。
- [ ] UI 不展示电脑手牌。
- [ ] 当前玩家和胜负状态清晰可见。
- [ ] 选中牌、非法出牌、不能 pass 都有明确反馈。

## 4. 模拟器接入

- [ ] 修改 `gui/sim/config.uya`，新增 `SimDemoKind.Doudizhu`。
- [ ] 修改 `sim_demo_name()`，返回 `斗地主`。
- [ ] 修改命令行解析，支持 `--demo doudizhu`。
- [ ] 修改 `gui/sim/app.uya`，添加 retained state。
- [ ] 修改 `gui/sim/app.uya`，接入 render/update。
- [ ] 修改 `gui/sim/app.uya`，接入输入事件。
- [ ] 增加热键 `Z` 切换到斗地主。
- [ ] 更新 simulator help 文案。

验收：

- [ ] `make sim-run SIM_ARGS="--demo doudizhu --scale 1"` 能打开。
- [ ] `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"` 能生成截图。
- [ ] 切换到其他 demo 再切回不崩溃。

## 5. 测试接入

- [ ] 新建 `gui/tests/test_doudizhu_rules.uya`。
- [ ] 新建 `gui/tests/test_doudizhu_ai.uya`。
- [ ] 修改 `gui/test_suite.uya` 聚合新测试。
- [ ] 如需快速 smoke，补 `gui/examples` demo 渲染测试。

验收：

- [ ] `./uya/bin/uya test gui/test_suite.uya -O0 --stack-size 65536` 通过。
- [ ] `make test` 通过。

## 6. OpenAI Uya 封装

- [ ] 新建 `gui/platform/openai/chat.uya`。
- [ ] 声明 `uya_openai_chat_available()`。
- [ ] 声明 `uya_openai_chat_start()`。
- [ ] 声明 `uya_openai_chat_poll()`。
- [ ] 声明 `uya_openai_chat_cancel()`。
- [ ] 封装 Uya 侧状态码 enum。
- [ ] 封装 start/poll/cancel 的薄包装。
- [ ] 实现 `{ "action_id": n }` 解析辅助。

验收：

- [ ] OpenAI 封装不 import 斗地主规则。
- [ ] 返回码映射清晰。
- [ ] JSON 解析失败返回错误，不产生未定义 action。

## 7. OpenAI Stub

- [ ] 新建 `gui/platform/openai/openai_chat_stub.c`。
- [ ] 实现 `uya_openai_chat_available()` 返回 0。
- [ ] 实现 `start()` 返回负值。
- [ ] 实现 `poll()` 返回 `-4`。
- [ ] 实现 `cancel()` 幂等空操作。

验收：

- [ ] 无 libcurl 开发包时 `make sim-build` 仍通过。
- [ ] 未设置 API Key 时 demo 自动离线。

## 8. libcurl Host Bridge

- [ ] 新建 `gui/platform/openai/openai_chat_host.c`。
- [ ] 读取 `OPENAI_API_KEY`。
- [ ] 读取 `OPENAI_MODEL`。
- [ ] 读取 `OPENAI_BASE_URL`，默认 `https://api.openai.com/v1`。
- [ ] 读取 `UYA_DDZ_USE_OPENAI`。
- [ ] 实现单 in-flight 请求槽。
- [ ] `start()` 中拷贝 request body。
- [ ] 后台线程执行 libcurl 请求。
- [ ] 设置 `Authorization: Bearer` header。
- [ ] 设置 `Content-Type: application/json`。
- [ ] 设置连接超时 `1500ms`。
- [ ] 设置总超时 `3500ms`。
- [ ] 限制最大 HTTP body。
- [ ] 解析 Chat Completions envelope。
- [ ] 提取 `choices[0].message.content`。
- [ ] 处理 refusal 或缺失 content。
- [ ] `poll()` 成功时复制 assistant content JSON。
- [ ] `poll()` 终态释放资源。
- [ ] `cancel()` 可取消未完成请求。
- [ ] 不打印 API Key。

验收：

- [ ] 有 libcurl 时 `make sim-build` 链接成功。
- [ ] 无 API Key 时 `available() == 0`。
- [ ] 请求失败不会阻塞 UI 主循环。
- [ ] 终态 handle 不可重复消费。

## 9. 构建脚本接入

- [ ] 修改 `tools/build_gui_sim.sh` 检测 `pkg-config --exists libcurl`。
- [ ] 有 libcurl 时加入 cflags/libs。
- [ ] 有 libcurl 时编译 `openai_chat_host.c`。
- [ ] 无 libcurl 时编译 `openai_chat_stub.c`。
- [ ] 保持 SDL2 检测逻辑不变。
- [ ] 保持默认 core/test 构建不链接 libcurl。

验收：

- [ ] `make sim-build` 在有 libcurl 环境通过。
- [ ] `make sim-build` 在无 libcurl 环境应可通过 stub 路径。
- [ ] `make test` 不因 libcurl 缺失受影响。

## 10. OpenAI 决策接入

- [ ] 在 `doudizhu.ai` 中构造 prompt payload。
- [ ] 固定 prompt 缓冲 `DDZ_PROMPT_BYTES`。
- [ ] 叫分阶段生成 bid legal_actions prompt。
- [ ] 出牌阶段生成 play legal_actions prompt。
- [ ] 调用 `openai_chat_start()`。
- [ ] 每帧 poll。
- [ ] 成功后解析 `action_id`。
- [ ] 校验 action id 仍然属于当前 legal_actions。
- [ ] 失败时启发式兜底。
- [ ] 更新 AI 状态 UI 文案。
- [ ] 重开和切 demo 时 cancel。

验收：

- [ ] OpenAI 成功时电脑动作来自模型选择。
- [ ] OpenAI 失败时同一回合仍能继续。
- [ ] 模型返回非法 id 时不会出非法牌。
- [ ] UI 主循环不卡住。

## 11. 文档更新

- [ ] 更新 `README.md`，添加斗地主 demo 运行命令。
- [ ] 更新 `README.md`，添加 OpenAI 环境变量说明。
- [ ] 更新 `docs/gui_uya_linux_sim.md`，添加 `--demo doudizhu`。
- [ ] 更新 `docs/gui_uya_linux_sim.md`，添加热键说明。
- [ ] 如有 API docs 生成清单，确认无需手写修改生成文件。

验收：

- [ ] 文档说明离线模式是默认安全路径。
- [ ] 文档不要求用户必须安装 libcurl 才能运行离线 demo。
- [ ] 文档不展示真实 API Key。

## 12. 最终验证

- [ ] `make test`
- [ ] `make sim-build`
- [ ] `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"`
- [ ] 无 `OPENAI_API_KEY` 运行一局 smoke。
- [ ] 设置 `UYA_DDZ_USE_OPENAI=0` 运行一局 smoke。
- [ ] 可选：有 API Key 时 live smoke。

可选 live smoke：

```bash
ALLOW_OPENAI_LIVE=1 OPENAI_API_KEY=... UYA_DDZ_USE_OPENAI=1 \
make sim-run SIM_ARGS="--demo doudizhu --max-frames 300"
```

最终验收：

- [ ] 离线 demo 可玩。
- [ ] OpenAI 可用时可参与电脑决策。
- [ ] OpenAI 不可用时自动降级。
- [ ] 默认 CI 不访问网络。
- [ ] 规则测试覆盖 MVP 牌型。

## 13. 后续增强

- [ ] 增加飞机。
- [ ] 增加飞机带翅膀。
- [ ] 增加四带二。
- [ ] 增加四带两对。
- [ ] 增加倍数、春天、反春天。
- [ ] 增加更强本地 AI。
- [ ] 增加出牌动画。
- [ ] 增加更细的 OpenAI 决策解释调试面板。
- [ ] 增加录制回放样例。
