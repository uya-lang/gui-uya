# OpenAI Chat 人机斗地主 Demo 详细设计文档

> 状态：待确认  
> 目标：在 UyaGUI Linux 模拟器里新增一个“1 名玩家 + 2 名电脑玩家”的斗地主 demo。电脑玩家可通过 OpenAI Chat Completions API 从本地生成的合法动作列表中选择动作，并在任何 API 不可用或结果异常时自动回退到本地启发式 AI。

## 1. 背景与约束

当前仓库是 UyaGUI 嵌入式 GUI/模拟器工程，已有 `2048`、`novel`、`widgets` 等 retained-state demo。斗地主 demo 应沿用现有 demo 的结构：示例逻辑放在 `gui/examples/*`，模拟器入口由 `gui/sim/config.uya` 和 `gui/sim/app.uya` 接入，运行通过 `make sim-run` / `make sim-headless` 验证。

用户明确要求使用 OpenAI 的 chat 接口，因此本设计使用 `POST /v1/chat/completions`，不改用 Responses API。OpenAI 官方文档当前说明 Chat Completions 接收一组 conversation messages 并返回模型响应；Structured Outputs 支持用 `response_format: { "type": "json_schema" }` 约束输出。官方文档也持续推荐新项目优先考虑 Responses API，但本 demo 为满足需求固定走 Chat Completions。

参考资料：

- Chat Completions API: `https://platform.openai.com/docs/api-reference/chat/create`
- Structured Outputs: `https://platform.openai.com/docs/guides/structured-outputs`
- Latest model guidance: `https://developers.openai.com/api/docs/guides/latest-model`

## 2. 设计目标

1. 新增可运行的 UyaGUI 斗地主 demo：`make sim-run SIM_ARGS="--demo doudizhu --scale 1"`。
2. 支持单局完整流程：发牌、叫地主、发底牌、出牌、不要、轮转、胜负判定、重开。
3. 电脑玩家在可用时通过 OpenAI Chat Completions API 选择动作。
4. OpenAI 只允许从本地生成的 `legal_actions` 中返回 `action_id`，不能直接决定牌面或绕过规则。
5. 任何失败场景都必须使用本地启发式 AI 兜底，包括无 API Key、无 libcurl、网络超时、HTTP 错误、JSON 解析失败、非法 `action_id`。
6. 默认测试与 CI 不依赖网络，不依赖真实 OpenAI Key。
7. 保持 UyaGUI 当前风格：固定容量数组、明确生命周期、无默认动态 GC、可 headless 截图验证。

## 3. 非目标

1. 不实现联网多人对战。
2. 不把 OpenAI Key 写入代码、日志、截图或 UI 文本。
3. MVP 不做完整积分体系、倍数、春天、反春天、音效和复杂动画。
4. MVP 不覆盖全部斗地主边角牌型，先覆盖能稳定完成对局的核心牌型。
5. 不把 OpenAI HTTPS 调用放入裸机目标路径；首版只在 Linux SDL2 模拟器中启用。
6. 不追求 AI 强度，首版重点是稳定、合法、可解释、可回退。

## 4. 用户体验

运行命令：

```bash
make sim-run SIM_ARGS="--demo doudizhu --scale 1"
```

启用 OpenAI：

```bash
OPENAI_API_KEY=... \
OPENAI_MODEL=gpt-5.4-mini \
UYA_DDZ_USE_OPENAI=1 \
make sim-run SIM_ARGS="--demo doudizhu --scale 1"
```

离线模式：

- 不设置 `OPENAI_API_KEY`。
- 或设置 `UYA_DDZ_USE_OPENAI=0`。
- 或当前构建环境没有 libcurl 开发包。

用户看到的行为：

- 进入 demo 后自动发牌。
- 人类玩家在叫分阶段点击 `不叫`、`1分`、`2分`、`3分`。
- 出牌阶段点击手牌选中或取消，选中牌上移。
- 点击 `提示` 时只选择推荐牌，不自动提交。
- 点击 `出牌` 提交选中牌。
- 点击 `不要` 在规则允许时跳过。
- 点击 `重开` 取消当前 OpenAI 请求并重新开始。
- AI 回合显示 `Thinking...`、`OpenAI`、`Offline`、`Timeout` 等短状态。

## 5. MVP 规则范围

牌组：

- 54 张牌：`3 4 5 6 7 8 9 10 J Q K A 2 BJ RJ`。
- 花色只用于显示和区分实体牌，比较时只看 rank。
- 三家各 17 张，底牌 3 张。
- 地主拿底牌后最大手牌数为 20。

叫地主：

- 简化为一轮叫分，每家一次机会。
- 动作为 `bid_0`、`bid_1`、`bid_2`、`bid_3`。
- 最高分者为地主；无人叫分则自动重新发牌。
- 若多人同分，先叫到最高分者保持领先。
- 地主拿底牌后进入出牌阶段，地主先出。

出牌牌型：

- `Pass`
- 单张
- 对子
- 三张
- 三带一
- 三带一对
- 顺子，至少 5 张，不含 `2` 和大小王
- 连对，至少 3 对，不含 `2` 和大小王
- 炸弹
- 火箭

暂不纳入 MVP 的牌型：

- 飞机
- 飞机带翅膀
- 四带二
- 四带两对

这些牌型会在数据结构中预留 enum 值空间，但首版识别和生成不启用。

## 6. 总体架构

```text
SDL2 input / keyboard
  -> sim.app
    -> demo_doudizhu retained page
      -> doudizhu.rules       本地规则、牌型、合法动作
      -> doudizhu.ai          决策调度、启发式、OpenAI 状态机
        -> platform.openai.chat.uya
          -> openai_chat_host.c / openai_chat_stub.c
            -> libcurl
            -> POST /v1/chat/completions
```

核心原则：

- 规则层不依赖 UI、不依赖 OpenAI，必须可单元测试。
- AI 层只拿规则层生成的 `DdzActionList`，不能手写绕过规则的出牌。
- UI 层只负责展示、输入、状态同步，不复制牌型判断。
- OpenAI host bridge 不知道斗地主规则，只负责 HTTPS 调用、外层响应解析、生命周期管理。

## 7. 文件与模块

新增文件：

- `gui/examples/demo_doudizhu.uya`
  - retained demo 状态、Canvas 绘制、按钮、点击选牌、每帧更新。
- `gui/examples/doudizhu/rules.uya`
  - 牌、手牌、牌型识别、牌型比较、合法动作生成、发牌、胜负判定。
- `gui/examples/doudizhu/ai.uya`
  - 本地启发式 AI、OpenAI request 构造、请求轮询、返回动作校验、失败冷却。
- `gui/platform/openai/chat.uya`
  - Uya 侧 host bridge 轻封装，统一 available/start/poll/cancel。
- `gui/platform/openai/openai_chat_host.c`
  - libcurl 真实 HTTPS 实现。
- `gui/platform/openai/openai_chat_stub.c`
  - 无 libcurl 时的同名 stub 实现。
- `gui/tests/test_doudizhu_rules.uya`
  - 规则单测。
- `gui/tests/test_doudizhu_ai.uya`
  - 启发式、返回解析和兜底单测。
- `docs/openai_doudizhu_demo_todo.md`
  - 实现任务拆分。

修改文件：

- `gui/sim/config.uya`
  - 增加 `SimDemoKind.Doudizhu`、`sim_demo_name()`、`--demo doudizhu` 解析。
- `gui/sim/app.uya`
  - 增加 demo retained state、输入分发、渲染/更新、热键切换。
- `gui/test_suite.uya`
  - 聚合新测试。
- `tools/build_gui_sim.sh`
  - 可选检测并链接 libcurl，缺失时编译 stub。
- `README.md`
  - 增加运行说明与 OpenAI 环境变量。

## 8. 常量与数据结构

固定容量常量：

```text
DDZ_PLAYER_COUNT = 3
DDZ_DECK_COUNT = 54
DDZ_BOTTOM_COUNT = 3
DDZ_INITIAL_HAND_COUNT = 17
DDZ_MAX_CARDS_PER_HAND = 20
DDZ_RANK_COUNT = 15
DDZ_MAX_COMBO_CARDS = 20
DDZ_MAX_ACTIONS = 128
DDZ_ACTION_LABEL_BYTES = 48
DDZ_PROMPT_BYTES = 8192
DDZ_OPENAI_RESPONSE_BYTES = 1024
```

Rank 映射：

```text
0  -> 3
1  -> 4
2  -> 5
3  -> 6
4  -> 7
5  -> 8
6  -> 9
7  -> 10
8  -> J
9  -> Q
10 -> K
11 -> A
12 -> 2
13 -> BJ
14 -> RJ
```

核心结构草案：

```text
DdzCard
  id: i32
  rank: i32
  suit: i32

DdzHand
  cards: [DdzCard; DDZ_MAX_CARDS_PER_HAND]
  count: i32
  rank_counts: [i32; DDZ_RANK_COUNT]

DdzComboKind
  Invalid | Pass | Single | Pair | Triple | TripleSingle | TriplePair |
  Straight | PairStraight | Bomb | Rocket

DdzCombo
  kind: DdzComboKind
  main_rank: i32
  sequence_len: i32
  cards: [DdzCard; DDZ_MAX_COMBO_CARDS]
  count: i32

DdzAction
  id: i32
  combo: DdzCombo
  label: [byte; DDZ_ACTION_LABEL_BYTES]

DdzActionList
  actions: [DdzAction; DDZ_MAX_ACTIONS]
  count: i32
  truncated: bool

DdzGamePhase
  Deal | Bid | RevealBottom | Play | GameOver

DdzGame
  hands: [DdzHand; 3]
  bottom: [DdzCard; 3]
  landlord: i32
  current_player: i32
  last_combo: DdzCombo
  last_player: i32
  pass_count: i32
  bid_values: [i32; 3]
  highest_bid: i32
  highest_bidder: i32
  bid_turn: i32
  phase: DdzGamePhase
  winner: i32
  rng_state: u32
```

UI 状态结构：

```text
DdzPageRetained
  game: DdzGame
  ai: DdzAiController
  selected_card_ids: [i32; DDZ_MAX_CARDS_PER_HAND]
  selected_count: i32
  last_play_label: [byte; 96]
  status_label: [byte; 128]
  ai_status_label: [byte; 96]
  canvas + buttons + retained widget tree
```

## 9. 规则层详细设计

### 9.1 发牌与排序

发牌流程：

1. `ddz_deck_init(deck)` 生成 54 张唯一牌。
2. `ddz_deck_shuffle(deck, rng_state)` 使用当前工程已采用的轻量 LCG 风格随机。
3. 前 51 张按 `player = index % 3` 分发。
4. 最后 3 张存入 `bottom`。
5. 每个手牌调用 `ddz_hand_sort()`，按 rank 升序，rank 相同按 suit/id 排序。
6. 更新每手牌的 `rank_counts`。

不变量：

- 任意时刻 54 张牌总数守恒。
- 任意 `DdzHand.count` 不超过 `DDZ_MAX_CARDS_PER_HAND`。
- `rank_counts` 必须由 `cards` 重新计算，避免局部维护出错。

### 9.2 牌型识别

`ddz_combo_detect(cards, count)` 输入一组实体牌，输出 `DdzCombo`。

识别顺序：

1. `count == 0` -> `Pass`
2. `count == 1` -> `Single`
3. `count == 2` 且双王 -> `Rocket`
4. `count == 2` 且同 rank -> `Pair`
5. `count == 3` 且同 rank -> `Triple`
6. `count == 4` 且四张同 rank -> `Bomb`
7. `count == 4` 且存在三张 -> `TripleSingle`
8. `count == 5` 且存在三张和一对 -> `TriplePair`
9. `count >= 5` 且连续单 rank、每 rank 一张、不含 rank >= 12 -> `Straight`
10. `count >= 6` 且偶数、连续 rank、每 rank 两张、不含 rank >= 12 -> `PairStraight`
11. 否则 `Invalid`

`main_rank` 规则：

- 单、对、三、三带、炸弹为主体 rank。
- 顺子、连对为最高 rank。
- 火箭 main_rank 固定为 14。
- Pass main_rank 为 -1。

### 9.3 牌型比较

`ddz_combo_can_beat(candidate, last)`：

- `last.kind == Pass` 时，任何非 Pass 合法牌都可出。
- `candidate.kind == Rocket` 时，除 candidate 本身非法外永远可压。
- `last.kind == Rocket` 时不可压。
- `candidate.kind == Bomb` 且 `last.kind != Bomb` 时可压。
- 双方都是 Bomb 时比较 `main_rank`。
- 普通牌必须 kind 相同、count 相同、sequence_len 相同，且 `candidate.main_rank > last.main_rank`。
- Pass 不能压任何牌，只能在不是主动出牌时作为跳过动作。

### 9.4 合法动作生成

`ddz_generate_legal_actions(game, player, out_actions)`：

1. 清空 action list。
2. 如果当前不是主动出牌，先加入 `pass`。
3. 根据手牌 rank_counts 枚举 MVP 牌型。
4. 对每个候选调用 `ddz_combo_can_beat()`。
5. 通过则加入 action list。
6. 按“保守优先”排序：
   - pass
   - 普通小牌
   - 顺子/连对
   - 三带
   - 炸弹
   - 火箭
7. 超过 `DDZ_MAX_ACTIONS` 时丢弃排序靠后的动作，置 `truncated = true`。

主动出牌定义：

- `last_player == current_player`
- 或 `last_combo.kind == Pass`
- 或连续两个其他玩家 pass 后清空上一手。

### 9.5 应用动作

`ddz_game_apply_action(game, player, action)`：

- 校验 `player == current_player`。
- 校验 action 在当前合法列表中。
- Pass：
  - 仅允许非主动出牌。
  - `pass_count += 1`。
  - 若 `pass_count >= 2`，清空 `last_combo`，下一个轮到 `last_player` 主动出。
- 非 Pass：
  - 从手牌移除 action.cards。
  - 设置 `last_combo = action.combo`、`last_player = player`、`pass_count = 0`。
  - 若手牌为空，设置 `phase = GameOver`、`winner = player`。
- 正常轮转到下一位玩家。

## 10. AI 层设计

### 10.1 决策入口

AI 层只暴露一个主入口：

```text
ddz_ai_request_or_choose(game, player, action_list, ai_controller) -> DdzAiDecisionState
```

状态：

- `ReadyLocal`：已经用启发式选出动作。
- `WaitingOpenAI`：已发起 OpenAI 请求，等待 poll。
- `ReadyOpenAI`：OpenAI 返回合法动作。
- `Fallback`：OpenAI 不可用或失败，已用启发式选出动作。

UI 每帧调用：

```text
ddz_ai_update(controller, game, action_list)
```

### 10.2 本地启发式

叫分评分：

- 火箭 +5
- 每个炸弹 +4
- 每个王 +2
- 每张 `2` +1
- A/K 较多 +1
- score >= 8 叫 3，>= 5 叫 2，>= 3 叫 1，否则不叫。

出牌策略：

- 主动出牌：优先出最小非炸弹动作，若只剩少量牌可出更长组合。
- 跟牌：选择能压过上家的最小动作。
- 默认不使用炸弹/火箭压普通牌。
- 若自己剩余牌数 <= 2，允许使用炸弹/火箭收尾。
- 若上家是地主且地主剩余牌数 <= 2，允许更积极压制。

### 10.3 OpenAI 决策输入

OpenAI 接收的是压缩后的局面摘要，不接收 API Key 或无关日志。

prompt payload 草案：

```json
{
  "phase": "play",
  "seat": 1,
  "role": "peasant",
  "hand": "3 3 4 7 K A 2",
  "hand_count": 7,
  "landlord": 0,
  "current_player": 1,
  "last_player": 0,
  "last_play": "pair 9",
  "card_counts": [20, 7, 11],
  "legal_actions_truncated": false,
  "legal_actions": [
    { "id": 0, "label": "pass" },
    { "id": 1, "label": "pair K" }
  ]
}
```

叫分阶段也使用同一 schema：

```json
{
  "phase": "bid",
  "seat": 2,
  "role": "unknown",
  "hand": "3 4 4 7 8 9 J Q K A 2 BJ",
  "highest_bid": 1,
  "highest_bidder": 0,
  "legal_actions": [
    { "id": 0, "label": "bid_0" },
    { "id": 2, "label": "bid_2" },
    { "id": 3, "label": "bid_3" }
  ]
}
```

### 10.4 Chat Completions 请求

环境变量：

```bash
OPENAI_API_KEY=...
OPENAI_MODEL=gpt-5.4-mini
OPENAI_BASE_URL=https://api.openai.com/v1
UYA_DDZ_USE_OPENAI=1
```

默认模型使用 `gpt-5.4-mini` 作为低延迟 demo 示例值，并允许 `OPENAI_MODEL` 覆盖。实现不得依赖具体模型名。启用 structured output 时应选择支持 Chat Completions `response_format.json_schema` 的模型。

请求体：

```json
{
  "model": "gpt-5.4-mini",
  "messages": [
    {
      "role": "system",
      "content": "You are a Dou Dizhu AI. Choose exactly one legal action id from the user-provided legal_actions. Do not invent cards. Return JSON only."
    },
    {
      "role": "user",
      "content": "{\"phase\":\"play\",\"seat\":1,\"role\":\"peasant\",\"hand\":\"3 3 4 7 K A 2\",\"last_play\":\"pair 9\",\"legal_actions\":[{\"id\":0,\"label\":\"pass\"},{\"id\":1,\"label\":\"pair K\"}]}"
    }
  ],
  "response_format": {
    "type": "json_schema",
    "json_schema": {
      "name": "doudizhu_action",
      "strict": true,
      "schema": {
        "type": "object",
        "properties": {
          "action_id": { "type": "integer" }
        },
        "required": ["action_id"],
        "additionalProperties": false
      }
    }
  }
}
```

期望 assistant content：

```json
{ "action_id": 1 }
```

Uya 侧只使用 `action_id`。调试说明由本地状态记录，例如 `openai action 1`、`fallback timeout`。

### 10.5 返回校验

OpenAI 返回动作必须经过三层校验：

1. C bridge 校验 HTTP 和 Chat envelope。
2. Uya `platform.openai.chat` 校验返回 JSON 长度与格式。
3. `doudizhu.ai` 校验 `action_id` 是否存在于当前 `DdzActionList`。

任意一层失败都进入本地启发式兜底。

## 11. Host Bridge 设计

Uya 标准库已有 JSON 和 socket 相关能力，但 OpenAI API 必须走 HTTPS。首版不在 Uya 中实现 TLS/HTTP 客户端，而是在 Linux 模拟器侧提供 C host bridge。

Uya extern 接口：

```uya
extern fn uya_openai_chat_available() i32;
extern fn uya_openai_chat_start(req_json: &const byte, req_len: usize) i32;
extern fn uya_openai_chat_poll(handle: i32, out: &byte, out_cap: usize) i32;
extern fn uya_openai_chat_cancel(handle: i32) void;
```

返回约定：

- `available() == 1`：当前构建和环境可尝试 OpenAI。
- `available() == 0`：不可用，直接离线模式。
- `start > 0`：请求句柄。
- `start < 0`：启动失败，立即启发式兜底。
- `poll == 0`：仍在等待。
- `poll > 0`：已把 assistant content JSON 写入 `out`，返回 JSON 字节数。
- `poll == -1`：网络、DNS、TLS、HTTP 或 API 错误。
- `poll == -2`：超时。
- `poll == -3`：`out_cap` 不足。
- `poll == -4`：无效或已消费的 handle。

生命周期与并发：

- `start()` 必须在返回前把 `req_json` 拷贝到 C 侧缓冲区，不能保存 Uya 栈/数组指针。
- 首版只允许 1 个 in-flight 请求；若已有请求未完成，新的 `start()` 返回负值。
- `poll() > 0`、`poll() < 0` 都是终态。终态后 bridge 释放该 handle 持有的请求、响应和线程资源。
- 终态后的同一 handle 再次 `poll()` 返回 `-4`。
- `cancel()` 幂等；对无效 handle、已完成 handle 调用不产生副作用。
- demo 切换、重开、退出时必须调用 `cancel()`。
- `out_cap` 首版固定为 `DDZ_OPENAI_RESPONSE_BYTES = 1024`。

Chat envelope 处理：

- OpenAI Chat Completions 的原始 HTTP body 是外层 envelope。
- 动作 JSON 位于 `choices[0].message.content`。
- `openai_chat_host.c` 负责解析外层 envelope，只把 assistant content 中的 JSON 对象复制给 Uya。
- 如果响应包含 refusal、没有 `choices[0].message.content`、content 不是 JSON object，bridge 返回错误。

libcurl 构建策略：

- `tools/build_gui_sim.sh` 用 `pkg-config --exists libcurl` 检测 curl。
- 可用时编译 `openai_chat_host.c`，加入 libcurl cflags/libs。
- 不可用时编译 `openai_chat_stub.c`，该文件不得 include curl 头文件。
- 默认 Uya core/test 构建不链接 libcurl，也不访问网络。

运行参数：

- 连接超时：`1500ms`
- 总请求超时：`3500ms`
- 最大响应体：`16384` 字节
- assistant content 最大：`DDZ_OPENAI_RESPONSE_BYTES`
- 连续失败冷却：同一局内连续 3 次 OpenAI 失败后，冷却 5 个电脑决策回合。

安全要求：

- `Authorization: Bearer` header 只在 C bridge 内构造。
- 不打印完整请求 header。
- 不打印 API Key。
- UI 不显示 API Key。
- `OPENAI_BASE_URL` 只用于开发/代理场景，默认 `https://api.openai.com/v1`。

## 12. UI 详细设计

默认 640x480 布局：

```text
┌────────────────────────────────────────────────────────────┐
│ P2 电脑  牌数:17  农民/地主  AI:OpenAI/Offline             │
│                                                            │
│       底牌: [??] [??] [??]      上一手: pair 9             │
│                                                            │
│ P1 电脑 牌数:12              当前: 你                      │
│                                                            │
│ 状态: 请选择要出的牌 / 电脑思考中 / 地主获胜               │
│                                                            │
│ 你的手牌: 3 3 4 7 K A 2 BJ ...                            │
│                                                            │
│ [不叫] [1分] [2分] [3分]                                  │
│ 或 [提示] [出牌] [不要] [重开]                             │
└────────────────────────────────────────────────────────────┘
```

绘制策略：

- 背景使用深绿色桌面色，避免和已有蓝色/灰色 demo 过于接近。
- 卡牌用紧凑矩形，圆角保持小于等于 8px。
- 红色显示红桃/方块与大小王，黑色显示黑桃/梅花。
- 电脑手牌只显示背面和牌数。
- 底牌在叫地主前显示背面，地主确定后显示明牌。
- 当前玩家用细边框或高亮条提示。
- 选中手牌上移 12px。

控件策略：

- 叫分阶段只显示叫分按钮。
- 出牌阶段显示 `提示`、`出牌`、`不要`、`重开`。
- 当前不是人类回合时禁用 `提示`、`出牌`、`不要`。
- `出牌` 按钮在选中牌非法时保持可点击但显示错误状态，便于用户理解。
- `不要` 只在非主动出牌时有效。

输入命中：

- 手牌从右向左或从上层到下层检测，解决重叠卡牌点击。
- 点击按钮优先级高于点击手牌。
- `R` 可重开，`H` 可提示，`Space` 可出牌，`P` 仍保留模拟器截图语义。

## 13. 模拟器接入

`SimDemoKind` 增加：

```text
Doudizhu
```

命令行：

```bash
make sim-run SIM_ARGS="--demo doudizhu --scale 1"
make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"
```

热键建议：

- `Z`：切换到 doudizhu demo。
- `R`：demo 内重开。
- `H`：demo 内提示。
- `Space`：demo 内出牌。

`README.md` 和 `docs/gui_uya_linux_sim.md` 后续补充 `--demo doudizhu`，但不阻塞首版功能。

## 14. 错误处理

| 场景 | 行为 |
| --- | --- |
| 未设置 `OPENAI_API_KEY` | 离线启发式模式 |
| `UYA_DDZ_USE_OPENAI=0` | 离线启发式模式 |
| libcurl 不可用 | 离线启发式模式 |
| 已有 in-flight 请求 | 本回合启发式兜底 |
| 请求超时 | 本回合启发式兜底，状态显示 `timeout` |
| HTTP 非 2xx | 本回合启发式兜底，保留错误码 |
| API response 含 refusal | 本回合启发式兜底，状态显示 `refused` |
| envelope 缺少 content | 本回合启发式兜底 |
| assistant content 不是 JSON object | 本回合启发式兜底 |
| JSON 解析失败 | 本回合启发式兜底 |
| `action_id` 非法 | 本回合启发式兜底 |
| 连续失败 3 次 | 冷却 5 个电脑决策回合 |
| 用户重开/切 demo | cancel 未完成请求 |

## 15. 测试计划

默认测试不访问网络。

规则测试：

- deck 初始化后 54 张唯一。
- 发牌数量为 `17/17/17 + 3`。
- hand sort 与 rank_counts 正确。
- 牌型识别：单、对、三、三带一、三带一对、顺子、连对、炸弹、火箭。
- 牌型拒绝：含 `2` 的顺子、含王的顺子、断裂顺子、长度不足连对。
- 牌型比较：同牌型按主牌、炸弹压普通牌、火箭最大。
- pass 规则：主动出牌不能 pass，跟牌可以 pass。
- apply action 后手牌减少、last_combo 更新、pass_count 更新。
- 胜负判定：任一玩家手牌归零进入 GameOver。

合法动作测试：

- 主动出牌生成普通动作。
- 跟牌只生成可压动作和 pass。
- 炸弹/火箭在必要时出现。
- `DDZ_MAX_ACTIONS` 截断只保留合法动作并设置 `truncated`。
- action id 稳定从 0 递增。

AI 测试：

- 启发式叫分可重复。
- 启发式出牌永远从 legal_actions 选择。
- 合法 OpenAI JSON 接收。
- 坏 JSON、非法 id、超时码走兜底。
- 连续失败触发冷却。

Bridge 测试：

- stub `available() == 0`。
- 提取 `choices[0].message.content`。
- 处理 refusal。
- 处理过小 `out_cap`。
- 终态释放 handle。
- `cancel()` 幂等。

模拟器 smoke：

```bash
make test
make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"
```

可选 live 测试：

```bash
ALLOW_OPENAI_LIVE=1 OPENAI_API_KEY=... UYA_DDZ_USE_OPENAI=1 \
make sim-run SIM_ARGS="--demo doudizhu --max-frames 300"
```

live 测试不进入默认 CI。

## 16. 实现里程碑

1. 规则层与单测。
2. 离线启发式 AI。
3. 离线可玩的 UyaGUI demo。
4. 模拟器 `--demo doudizhu` 接入。
5. OpenAI bridge stub 与构建接入。
6. libcurl host bridge。
7. OpenAI 决策接入与兜底。
8. 文档、README、headless smoke。

## 17. 风险与缓解

| 风险 | 缓解 |
| --- | --- |
| Uya 侧 JSON 构造容易越界 | 使用固定缓冲、长度检查、失败即兜底 |
| 合法动作过多 | `DDZ_MAX_ACTIONS` 截断并测试 |
| OpenAI 延迟影响 UI | 后台线程 + 非阻塞 poll + 超时 |
| libcurl 缺失导致构建失败 | stub 文件不 include curl |
| 模型返回不可用内容 | strict JSON schema + 本地二次校验 |
| API Key 泄漏 | 不记录 header，不显示 key |
| 牌型 MVP 与真实斗地主有差异 | UI/README 标注 MVP 规则，后续扩展 |

## 18. 待确认项

1. MVP 先不做飞机、四带二、完整积分倍数。
2. 首版 OpenAI 只为电脑决策服务，规则与胜负判定全部在本地。
3. OpenAI 调用只在 Linux SDL2 模拟器里启用，目标板路径走离线 AI。
4. 默认模型为可配置的 `OPENAI_MODEL`，示例值使用 `gpt-5.4-mini`。
5. 可接受 libcurl 作为可选模拟器依赖；无 libcurl 时自动离线。

确认后按 `docs/openai_doudizhu_demo_todo.md` 执行实现。
