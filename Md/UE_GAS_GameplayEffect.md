# Unreal Engine GAS：Gameplay Effect 完整学习手册

> **适用范围**：UE 5.6 / 5.7 的常见 GE 详情面板；适合初学者系统学习，也适合已有 GAS 基础的开发者快速查阅。不同小版本的显示名称、条件显示规则可能略有差别。
>
> **阅读路线**：整体认识 → 生命周期和周期 → Modifiers → Components → Gameplay Cues → Stacking → 综合案例 → C++ → 速查表与常见错误。
>
> 本文用 `Attack`、`Health`、`Intelligence`、`MoveSpeed` 演示，假定这些 Attribute 已经定义在自己的 AttributeSet 中。`State.*`、`Effect.*`、`Ability.*`、`GameplayCue.*`、`Data.*` 都是**示例 Gameplay Tag，需先在项目中注册**。

---

## 1. GE 是什么？与 GA、ASC、Attribute 的关系

**Gameplay Effect（GE）是 GAS 用来描述游戏效果的一组数据及规则**。它不仅能修改属性，也能提供状态标签、阻止技能、授予技能、移除其他效果、管理堆叠与触发表现。

典型例子：瞬时伤害、治疗、攻击力 Buff、中毒、减速、沉默、免疫、净化、装备被动效果。

| 概念 | 负责什么 | 例子 |
|---|---|---|
| `UAbilitySystemComponent`（ASC） | 管理技能、效果、Gameplay Tags 等 | 角色身上的技能系统 |
| `UAttributeSet` | 声明和管理属性 | Health、Attack |
| `UGameplayAbility`（GA） | 组织技能的施放流程 | 火球术、翻滚、治疗术 |
| `UGameplayEffect`（GE） | 定义效果及其计算、条件和生命周期 | -100 血、+50 攻击力 |
| Gameplay Cue（GC） | 表现层反馈 | 火焰特效、声音、命中闪光 |

### 1.1 运行时为什么还会看到 Spec？

GE 蓝图资产通常是可复用的**配置模板**，真正的一次应用由 `FGameplayEffectSpec` 携带上下文、等级、Set by Caller 数值等运行时信息。

```text
GA / 游戏逻辑
  → 构造 GameplayEffectSpec（GE 配置 + 等级 + 来源 + 动态参数）
  → Source ASC 向 Target ASC 施加
  → 检查应用限制、免疫等
  → 修改 Attribute / 创建 Active GE
  → 管理标签、周期、堆叠与 Gameplay Cue
  → 到期或移除
```

### 1.2 详情面板的五大块

| 模块 | 核心问题 |
|---|---|
| Duration + Period | 效果存在多久、是否按周期执行？ |
| Modifiers | 改哪个属性、采用哪种运算、改多少、满足什么标签条件？ |
| Components | 附加哪些标签、施加限制、免疫、授予技能等？ |
| Gameplay Cues | 要播放什么声音、粒子和表现？ |
| Stacking | 重复施加时独立存在还是合并层数？ |

**注意：** Components 不是 Actor Component。GE Component 是附属于 GE 配置的对象，也不应该保存每次施加的独立运行时状态。

---

## 2. Duration Policy：GE 的生命周期

| Duration Policy | 含义 | 常见例子 |
|---|---|---|
| `Instant` | 立即执行并结束，不形成有持续时间的活动 GE | 一次扣血、一次回血 |
| `Has Duration` | 持续指定时间，到期移除 | 10 秒攻击 Buff、5 秒中毒 |
| `Infinite` | 一直存在，直到被主动移除或由其他规则处理 | 装备被动、长期光环 |

### 2.1 Base Value 与 Current Value

Attribute 常见地维护 **Base Value（基础值）** 与 **Current Value（当前值）**：

- `Instant GE` 修改属性时，通常改变 Base Value；例如 Health 从 500 扣到 450，效果结束不会自动恢复。
- `Has Duration / Infinite GE` 且 `Period = 0` 时，对普通 Modifier 的临时增益通常体现在 Current Value，移除后重新聚合恢复。
- 带 `Period` 的 GE 会**周期执行类似 Instant 的属性修改**；不能简单当作“临时 +10、到期归还”。

例：Attack 基础值 100，施加 10 秒 `Attack +50`（Period=0），期间 Current=150、Base=100，结束后 Current=100。若是 Instant `Attack +50`，Base 通常变为 150。

> 真实项目应在 AttributeSet 中做 Health 限幅、死亡处理等；GE 本身不会自动确保血量不小于 0。

### 2.2 Duration Magnitude：持续时间从哪里来？

只有 `Has Duration` 才需要计算持续时长，其计算来源与 Modifier Magnitude 的四种选项一致：

1. **Scalable Float**：配置固定秒数或 Curve Table 按等级取值。
2. **Attribute Based**：根据来源或目标 Attribute 计算。
3. **Custom Calculation Class**：用自定义 MMC 返回秒数。
4. **Set by Caller**：应用 GE 之前，由调用者写入秒数。

例如 5 秒控制、按抗性计算持续时间、技能等级越高持续越久。**这里的结果单位是秒，而不是攻击力或伤害。**

---

## 3. Period：周期执行

**Duration 管存在多久，Period 管多久执行一次。**

| Duration | Period | 结果 |
|---|---|---|
| 5 秒 | 0 | 持续维持属性加成，结束时撤销临时修饰 |
| 5 秒 | 1 秒 | 在有效期内每隔 1 秒执行周期效果 |
| Infinite | 1 秒 | 持续周期执行，直到 GE 被移除 |

### 3.1 常见例子：中毒

`GE_Poison`：Duration=5，Period=1，Modifier=`Health Add -10`。

- 不是“临时把生命值减 10、结束再还原”。
- 而是每次周期到来执行扣血，已经扣掉的生命不会因 GE 到期自动返还。

### 3.2 Execute Periodic Effect on Application

- **不勾选**：等待首个周期到达，再执行第一次。
- **勾选**：应用时立即执行一次，然后按 Period 继续。

UE 编辑器通常在 Period 设置为大于 0 后显示此项。不要把“5 秒 / 1 秒”机械理解成必然执行 5 次；实际事件数还受立即执行、到期边界、刷新、抑制等影响。

### 3.3 Periodic Inhibition Policy

当持续 GE 因标签条件**被抑制**，后来又恢复生效，如何处理周期计时？

| 选项 | 恢复生效时 |
|---|---|
| `Never Reset` | 保留原周期计时节奏 |
| `Reset Period` | 重新开始等待完整周期 |
| `Execute and Reset Period` | 恢复瞬间执行一次，然后重新计时 |

例：每 2 秒回血，在第 3 秒受到 `State.NoHealing` 限制，第 5.5 秒解除。三种策略分别对应继续原节奏、从 5.5 秒开始重新等 2 秒、或者 5.5 秒立即回血并重新计时。

**抑制 ≠ 移除**：GE 可以还在 ASC 内，其持续时间可能继续流逝，但某些作用暂不生效。

---

## 4. Modifiers：真正修改 Attribute 的规则

一个 Modifier 可以理解为：

```text
Attribute（改什么）
+ Modifier Op（怎么改）
+ Magnitude（改多少）
+ Source / Target Tags（哪些标签条件下改）
```

一个 GE 能有多个 Modifier，例如攻击 +50、移速增益、防御减益同时生效。

### 4.1 Modifier Op

| 选项 | 意义 | 单个修饰器作用于 100 的示例 |
|---|---|---|
| `Add (Base)` | 加法 | +50 → 150 |
| `Multiply (Base)` | 乘法 | 1.2 → 120 |
| `Divide (Base)` | 除法 | 2 → 50 |
| `Override` | 覆盖 | 200 → 200 |

**重点：** 20% 攻击加成的单个 Multiply 值为 `1.2`，不是 `20` 或 `0.2`。

**多个修饰器的聚合是重点陷阱**：GAS 中多个 Multiply / Divide 修饰器的聚合通常不是简单的连续连乘/连除，而涉及聚合器计算规则、偏移及求值顺序；复杂 BD 不能直接靠小学算术猜结果，应在当前引擎版本中验证。

### 4.2 四种 Magnitude Calculation Type

| 类型 | 从哪里得到数值 | 典型用途 | 是否必须自写计算代码 |
|---|---|---|---|
| `Scalable Float` | 常量或等级曲线 | 固定 +50 攻击力 | 否 |
| `Attribute Based` | 捕获 Source/Target Attribute | 智力 × 0.5 | 否 |
| `Custom Calculation Class` | MMC 计算返回 float | 属性 + Tag 的复合公式 | 通常需要 MMC 类 |
| `Set by Caller` | 外部写入 Spec 的 Data Tag 对应值 | 动态伤害、动态治疗 | 需要调用方传值 |

#### 例 A：Scalable Float — 等级攻击 Buff

`GE_AttackBuff_Fixed`：Duration=10，Period=0，Modifier=`Attack / Add / Scalable Float=50`。

- 固定版：10 秒攻击 +50。
- 曲线版：使用 Curve Table，例如 Level 1→50、Level 2→80、Level 3→120；应用 Spec 时设置 Level。
- **理解重点**：计算规则预先在 GE 里配置，代码无需知道数值。

#### 例 B：Attribute Based — 智力转换攻击力

`GE_AttackBuff_Intelligence`：Modifier=`Attack / Add / Attribute Based`。

| 设置 | 值 |
|---|---|
| Backing Attribute | Intelligence |
| Attribute Source | Source |
| Coefficient | 0.5 |
| Pre Multiply Additive Value | 0 |
| Post Multiply Additive Value | 0 |
| Snapshot | 按示例需要开启 |

通常的线性关系为：`(捕获属性 + 前置加值) × 系数 + 后置加值`（等级曲线可影响系数等字段）。智力 100 → +50 攻击力。

- **Source** 是效果施加者；**Target** 是效果接收者。
- **Snapshot** 决定是否捕获快照；关闭 Snapshot 不等于所有情形都无条件逐帧重算，应结合 GE 类型和属性依赖机制理解。

#### 例 C：Custom Calculation Class — 智力加成 + 狂暴倍率

`GE_AttackBuff_Custom`：Modifier=`Attack / Add / Custom Calculation Class=MMC_AttackBonus`。

```text
攻击力加成 = 智力 × (Source 有 State.Berserk ? 1.0 : 0.5)
```

MMC 负责**返回 Magnitude**，不是直接修改 Attack；最终仍由 Modifier Op 应用。MMC 需要在构造函数中注册要捕获的 Attribute。标签影响计算并不保证标签变化后 MMC 自动重算，须考虑捕获类型、依赖和 GE 重新应用的策略。

#### 例 D：Set by Caller — 同一 GE 处理不同伤害

`GE_Damage`：Duration=Instant，Modifier=`Health / Add / Set by Caller`，Data Tag=`Data.Damage`。

| 技能 | 写入 Data.Damage | 效果 |
|---|---:|---|
| 普通攻击 | -50 | Health -50 |
| 重击 | -100 | Health -100 |
| 暴击 | -200 | Health -200 |

Set by Caller 的**Tag 必须与 GE 中配置一致**，并且要在应用 Spec 前赋值。对于 `Add` 扣血示例使用负数；也可根据工程约定把正伤害写入专用 `Damage` 元属性，由执行计算处理。

### 4.3 Source Tags / Target Tags（Modifier 内）

它们属于**单条 Modifier 的条件**，不是添加标签。

| 字段 | 功能 |
|---|---|
| Source Tags → Must Have Tags | 来源须拥有指定标签 |
| Source Tags → Must Not Have Tags | 来源不能拥有指定标签 |
| Source Tags → Query Must Match | 更复杂的查询 |
| Target Tags → 同名三个字段 | 针对 GE 接收者检查 |

**正确例子**：`GE_BerserkBonus` 施加给玩家自身，第 1 个 Modifier 总是 Attack +20，第 2 个 Modifier 要求 **Target** 拥有 `State.Berserk` 才再加 Attack +50。

**重要提醒**：这里的 Target 是当前 GE 接收者，**不等于玩家将来攻击的敌人**。另需验证目标引擎版本对于 Instant / Periodic GE 中 Modifier 标签条件的实际支持；不要用它替代严谨的伤害计算逻辑。

---

## 5. Components：GE 的额外行为规则

在详情面板中：`Components → + → 选择 Component`。你所使用版本的下拉框中有以下 **11 类组件**（不含 None）。

| 编辑器中的完整名称 | 功能 | 常见用途 |
|---|---|---|
| `Apply Additional Effects` | 应用额外 GE，包含特定结束条件 | 印记到期爆炸 |
| `Block Abilities with Tags` | 阻止匹配 Ability Tags 的 GA 激活 | 沉默、禁攻击 |
| `Chance To Apply This Effect` | 按概率允许当前 GE 应用 | 30% 中毒 |
| `Custom Can Apply This Effect` | 自定义 GE 应用资格判断 | 只对低血量目标施加 |
| `Grant Gameplay Abilities` | 随 GE 授予 GA | 装备给予火球术 |
| `Grant Tags to Target Actor` | 把标签授予目标 ASC | State.Poisoned |
| `Immunity to Other Effects` | 阻止匹配的其他 GE 新施加 | 火焰免疫、控制免疫 |
| `Remove Other Effects` | 移除匹配的已存在 GE | 净化、驱散 |
| `Require Tags to Apply/Continue This Effect` | 应用、持续、移除条件 | 夜间 Buff、状态抑制 |
| `Tags This Effect Has (Asset Tags)` | 标识 GE 自身 | Effect.Debuff.Poison |
| `UI Data (Text Only)` | 携带 UI 展示文本等数据 | Buff 名称与描述 |

### 5.1 Asset Tags 和 Target Tags 到底在哪里？

这是两个不同 Component，**不是 Modifier 下的 Source/Target Tags**：

- `Tags This Effect Has (Asset Tags)`：添加 `Effect.Buff.Attack`，表明“这个 GE 是攻击 Buff”。其他查询/净化规则可以据此筛选 GE。
- `Grant Tags to Target Actor`：添加 `State.AttackBoosted`，表明“目标角色当前处于攻击强化状态”。对应 GE 有效期间可由 ASC 查询；GE 撤销它授予的标签时，不会消除其他来源独立授予的同名标签。

`Grant Tags to Target Actor` 中的：

| 项目 | 含义 |
|---|---|
| `Combined Tags` | 最终组合结果（只读） |
| `Add to Inherited` | 在父 GE 继承来的标签基础上加标签 |
| `Remove from Inherited` | 从继承标签里排除指定标签 |

可记为：**最终 = 继承集合 - 移除集合 + 增加集合**。

### 5.2 Block 和 Immunity 的区别

| | Block Abilities with Tags | Immunity to Other Effects |
|---|---|---|
| 作用对象 | **GA** 技能激活 | **GE** 效果应用 |
| 用来匹配 | Ability Tags | GE 查询（例如 Asset Tags） |
| 不会自动做什么 | 不保证取消已运行 GA | 不移除已经存在的 GE |
| 例子 | 沉默后无法释放火球 | 火焰免疫后拒绝火焰伤害 GE |

**Remove Other Effects** 才负责清理已有 GE。技能打断还可能需要 GA 自身的取消机制。Immunity 对于没有走 GE 施加流程的直接伤害代码不会凭空生效。

### 5.3 Require Tags to Apply/Continue 的三个条件

| 条件 | 典型作用 |
|---|---|
| Application Tag Requirements | 施加 GE 时必须满足，否则拒绝 |
| Ongoing Tag Requirements | GE 活动期间控制生效/抑制 |
| Removal Tag Requirements | 满足移除要求时将 GE 移除 |

例：`GE_NightBuff` 持续 30 秒、Attack +50，Ongoing 要求目标拥有 `World.Night`；白天 GE 可存在但暂时不提供加成。`World.Night` 必须由游戏逻辑授予到目标 ASC，并非自动读取世界时间。

### 5.4 组件常见组合

| 需求 | 必须/常用组件 | 对应作用 |
|---|---|---|
| 攻击强化状态 | `Grant Tags to Target Actor`、可选 `Tags This Effect Has (Asset Tags)` | 状态标记与 GE 分类；攻击增加由 Modifier 完成 |
| 沉默 3 秒 | `Block Abilities with Tags`、可选 `Grant Tags to Target Actor` | 禁止指定新 GA、状态可用于 UI |
| 净化中毒 | 被净化 GE 上 `Tags This Effect Has (Asset Tags)`；净化 GE 上 `Remove Other Effects` | 标记可被筛选的 GE，并移除它 |
| 控制免疫 | 被免疫 GE 上 `Tags This Effect Has (Asset Tags)`；免疫 GE 上 `Immunity to Other Effects` | 用查询拒绝新控制 GE |
| 装备技能 | `Grant Gameplay Abilities` | GE 存在时授予，移除策略负责撤销 |
| 印记自然到期爆炸 | `Apply Additional Effects` | 在 Normal Completion 施加爆炸 GE |
| 随机附加中毒 | `Chance To Apply This Effect` | 决定当前中毒 GE 是否被应用 |
| 低血目标才能被标记 | `Custom Can Apply This Effect` | 自定义条件代码返回能否应用 |

> `Apply Additional Effects` 的结束场景通常分为 **On Complete Normal**（自然结束）、**On Complete Prematurely**（提前移除）、**On Complete Always**（无论结束原因）。如果想在“角色死亡”时爆炸，不要误把 GE 的自然到期等同于角色死亡。

---

## 6. Gameplay Cues：把玩法效果与表现分开

GE 管数值和状态；Cue 管粒子、音效、角色表现。Cue 是表现通道，不应承载权威游戏逻辑（尤其是联机）。

| 编辑器配置 | 解释 |
|---|---|
| `Require Modifier Success to Trigger Cues` | 要求相关 Modifier 的应用满足成功条件才触发 Cue；**不等于最终 Health 一定发生变化** |
| `Suppress Stacking Cues` | 抑制堆叠导致的部分重复 Cue 通知；不禁止 GE 堆叠，也不保证每种粒子永远只有一个 |
| `Gameplay Cues` | 添加 Cue Tag，关联对应的 Gameplay Cue Notify |

### 6.1 Gameplay Cue 的常见事件

| 事件 | 适用表现 |
|---|---|
| `OnActive` | Buff 开始瞬间 |
| `WhileActive` | 持续性光环、火焰 |
| `Executed` | 瞬时/周期表现、命中反馈 |
| `Removed` | 停止持续特效、结束音效 |

不同 Notify 类型可实现的事件有所差别。Cue 也可以不依附于 GE，而由 GA / ASC 直接触发。

### 6.2 例：中毒粒子

`GE_Poison`：Duration=5，Period=1，Modifier=`Health Add -10`，Gameplay Cues 添加 `GameplayCue.Poison`。另创建对应 Gameplay Cue Notify（如 `GCN_Poison`），让目标中毒期间显示绿色毒雾、结束时停止。周期扣血的伤害数字可用 `Executed` 或独立伤害通知实现。

---

## 7. Stacking：重复应用相同 GE 怎么处理？

**堆叠不只是倍率增加，还管理聚合身份、层数、持续时间、周期以及上限溢出。**

### 7.1 三种 Stacking Type

| 类型 | 含义 | 典型用途 |
|---|---|---|
| `None` | 不启用堆叠聚合；多个施加独立存在、分别计时 | 多瓶独立药水 |
| `Aggregate by Source` | 同 GE 对同目标，按来源分别聚合 | 玩家 A、B 各自流血层数 |
| `Aggregate by Target` | 同 GE 对同目标，合并为目标统一层数 | 多来源统一中毒层数 |

*上述行为是针对同一 GE 定义的常见情形；实例匹配与来源 ASC 识别要正确。Instant GE 不以活动 GE 方式进行这种层数管理。*

### 7.2 常用堆叠选项

| 属性 | 含义 |
|---|---|
| `Stack Limit Count` | 层数上限 |
| `Stack Duration Refresh Policy` | 成功再次施加时是否刷新持续时间 |
| `Stack Period Reset Policy` | 成功再次施加时是否重置周期计时器 |
| `Stack Expiration Policy` | 计时到期时整组清除、减一层或刷新 |
| `Overflow Effects` | 达到上限后**再次尝试施加**时触发额外 GE |
| `Deny Overflow Application` | 拒绝超出上限的那次应用 |
| `Clear Stack on Overflow` | 按设置决定溢出时是否清除原堆叠 |

三个典型到期政策：

- `Clear Entire Stack`：到期清除全部层数。
- `Remove Single Stack and Refresh Duration`：减 1 层，并给剩余层数重新计时。
- `Refresh Duration`：不减少层数、重新开始持续时间。

**注意：逐层到期并不意味着每层天然拥有独立计时器。** 需要各层独立剩余时间时，常常需要另外的设计。

### 7.3 六个常见堆叠例子

#### A. 越打越强：攻击 Buff 叠到 5 层

`GE_AttackStack`：Duration=10，Period=0，Attack Add 10；`Aggregate by Target`、Limit=5、成功施加时 Refresh Duration、到期 Clear Entire Stack。

基础 Attack=100：1 层→110、3 层→130、5 层→150。持续命中刷新这组 GE 的计时，10 秒内不再叠层就全部清除。

#### B. 中毒层数提高每跳伤害

`GE_PoisonStack`：Duration=6，Period=1，Health Add -5；`Aggregate by Target`，Limit=5；Period Reset=`Never Reset`。

1 层通常每跳 -5，3 层每跳 -15，5 层每跳 -25。实际执行次数看首次执行设置和计时边界。

#### C. 多玩家独立流血

`GE_Bleed`：Duration=8，Period=1，Health Add -10，`Aggregate by Source`。A 施加 3 层、B 施加 2 层，两人的堆叠分别管理，可以分别到期和归属伤害。

#### D. 减速逐层衰减

`GE_SlowStack`：Duration=5，Period=0，MoveSpeed Multiply 0.9；`Aggregate by Target`，Limit=5，到期选择 `Remove Single Stack and Refresh Duration`。5 层时约 50% 的乘法减速修饰（**具体聚合需遵循 GAS 规则并验证**），到期依次减为 4、3、2、1 层。

#### E. 5 层燃烧后再次命中引爆

`GE_Burning`：Duration=10、Period=1、Health Add -5，Limit=5；`Overflow Effects=GE_Explosion`、`Deny Overflow Application=true`。**第 5 次到达 5 层，第 6 次尝试才触发 Overflow**。如要求“刚到 5 层立即爆炸”，需另写阈值机制。

#### F. 三瓶独立攻击药水

`GE_AttackPotion`：Duration=10、Period=0、Attack Add 20、Stacking Type=`None`。在 0、2、4 秒分别饮用，攻击力先 120→140→160，再在 10、12、14 秒分别到期退回 140→120→100。

---

## 8. 从需求反推 GE：八个可照着做的案例

每个案例都明确写出**基础 GE 配置**和**使用的 Component 完整名称**。

### 8.1 10 秒攻击力 +50

- **GE**：`GE_AttackBuff`；Duration=10，Period=0；Modifier=`Attack / Add / 50`。
- **组件**：`Grant Tags to Target Actor` → `State.AttackBoosted`（角色状态）；`Tags This Effect Has (Asset Tags)` → `Effect.Buff.Attack`（效果分类，可选）。
- **效果**：Attack 100→150→100；角色可查询攻击强化标签。

### 8.2 5 秒中毒，每秒 -10

- **GE**：`GE_Poison`；Duration=5，Period=1，Modifier=`Health / Add / -10`。
- **组件**：`Grant Tags to Target Actor` → `State.Poisoned`；`Tags This Effect Has (Asset Tags)` → `Effect.Debuff.Poison`。
- **可选**：`Chance To Apply This Effect` → 0.3，改成 30% 概率成功施加。
- **表现**：`Gameplay Cues` 添加 `GameplayCue.Poison`。

### 8.3 3 秒眩晕

- **GE**：`GE_Stun`；Duration=3、Period=0。
- **组件**：`Grant Tags to Target Actor` → `State.Stunned`；`Block Abilities with Tags` → `Ability.Attack`、`Ability.Skill`；`Tags This Effect Has (Asset Tags)` → `Effect.Control.Stun`。
- **注意**：禁用新 GA ≠ 取消当前 GA ≠ 禁用普通 CharacterMovement，后两者要设计独立处理。

### 8.4 净化全部 Debuff

- **被净化方**：`GE_Poison`、`GE_Burning`、`GE_Slow` 各自通过 `Tags This Effect Has (Asset Tags)` 添加 `Effect.Debuff.*`。
- **净化方 GE**：`GE_Cleanse`，Duration=`Instant`；组件 `Remove Other Effects` → 查询具有 `Effect.Debuff` Asset Tags 的活动 GE。
- **效果**：施加净化时立即移除已有匹配 GE，不保证之后免疫再次中毒。

### 8.5 5 秒控制免疫

- **GE**：`GE_ControlImmunity`；Duration=5。
- **组件**：`Immunity to Other Effects` → 查询 `Effect.Control`；可选 `Grant Tags to Target Actor` → `State.ControlImmune`。
- **前提**：被免疫的眩晕、冰冻 GE 已通过 `Tags This Effect Has (Asset Tags)` 分类为 `Effect.Control.Stun` / `Effect.Control.Freeze`。
- **效果**：拒绝新控制 GE；已有控制不因此自动被移除。

### 8.6 夜晚才有效的攻击 Buff

- **GE**：`GE_NightBuff`；Duration=30、Period=0、Attack Add 50。
- **组件**：`Require Tags to Apply/Continue This Effect` → `Ongoing Tag Requirements: Must Have World.Night`；Application 和 Removal 不设限制。
- **效果**：只在接收者 ASC 有该标签期间提供攻击加成，白天抑制而非移除。

### 8.7 装备后获得火球术

- **GE**：`GE_MagicSword`；Duration=`Infinite`。
- **组件**：`Grant Gameplay Abilities` → `GA_Fireball`；可选 `Tags This Effect Has (Asset Tags)` → `Effect.Equipment.Weapon`。
- **效果**：装备时施加 GE，卸下时移除对应 Active GE；技能去留依据 Ability 授予/移除策略。

### 8.8 5 秒后印记爆炸

- **GE**：`GE_ExplosiveMark`；Duration=5。
- **组件**：`Apply Additional Effects` → `On Complete Normal = GE_Explosion`。
- **效果**：自然到期后施加爆炸 GE；提前驱散时不触发。死亡触发爆炸属于另一个需求，不要和自然到期混淆。

---

## 9. C++ 实战：四种 Magnitude 最后怎么用？

以下示例假设已存在：

- `UAbilitySystemComponent* SourceASC` 和 `TargetASC`，都有效且已初始化。
- `TSubclassOf<UGameplayEffect> GEClass`，已在编辑器引用相应 GE 蓝图。
- `UMyAttributeSet` 含 `Attack`、`Intelligence`、`Health`，并生成 `GetIntelligenceAttribute()` 等属性访问函数。
- `.Build.cs` 依赖包含 `GameplayAbilities`、`GameplayTags`、`GameplayTasks`。
- 联机项目由具有权威性的逻辑施加 Gameplay Effect。

### 9.1 通用：创建 Spec，应用到目标

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void ApplyExample(
    UAbilitySystemComponent* SourceASC,
    UAbilitySystemComponent* TargetASC,
    TSubclassOf<UGameplayEffect> GEClass,
    float Level)
{
    if (!IsValid(SourceASC) || !IsValid(TargetASC) || !GEClass)
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        GEClass,
        Level,
        SourceASC->MakeEffectContext());

    if (SpecHandle.IsValid())
    {
        SourceASC->ApplyGameplayEffectSpecToTarget(
            *SpecHandle.Data.Get(), TargetASC);
    }
}
```

### 9.2 Scalable Float

`GE_AttackBuff_Fixed` 配置一个随等级变化的 Curve Table。只需：

```cpp
ApplyExample(SourceASC, TargetASC, GE_AttackBuff_Fixed, 2.0f);
```

Level=2 时读取 GE 中相应曲线值（如 +80）。**等级和数值的对应关系由 GE 配置决定。**

### 9.3 Attribute Based

`GE_AttackBuff_Intelligence` 配置 `Source Intelligence × 0.5`。调用仍是：

```cpp
ApplyExample(SourceASC, TargetASC, GE_AttackBuff_Intelligence, 1.0f);
```

区别在于 GAS 根据 Spec 的属性捕获自动计算，而不是代码额外传入 Intelligence。

### 9.4 Custom Calculation Class（MMC 完整关键代码）

在 `GE_AttackBuff_Custom` 中将 Magnitude Calculation Type 设为 `Custom Calculation Class`，选择 `UMMC_AttackBonus`。

**MMC_AttackBonus.h**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_AttackBonus.generated.h"

UCLASS()
class YOURPROJECT_API UMMC_AttackBonus
    : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()

public:
    UMMC_AttackBonus();

    virtual float CalculateBaseMagnitude_Implementation(
        const FGameplayEffectSpec& Spec) const override;

private:
    FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
};
```

**MMC_AttackBonus.cpp**

```cpp
#include "MMC_AttackBonus.h"
#include "MyAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

UMMC_AttackBonus::UMMC_AttackBonus()
{
    IntelligenceDef = FGameplayEffectAttributeCaptureDefinition(
        UMyAttributeSet::GetIntelligenceAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        true); // 捕获 Source 的快照

    RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_AttackBonus::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters Params;
    Params.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    Params.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float Intelligence = 0.f;
    if (!GetCapturedAttributeMagnitude(
        IntelligenceDef, Spec, Params, Intelligence))
    {
        return 0.f;
    }

    const FGameplayTag BerserkTag = FGameplayTag::RequestGameplayTag(
        FName(TEXT("State.Berserk")));

    const bool bBerserk = Params.SourceTags &&
        Params.SourceTags->HasTag(BerserkTag);

    return FMath::Max(0.f, Intelligence) * (bBerserk ? 1.0f : 0.5f);
}
```

> 将 `YOURPROJECT_API` 换成真实项目的模块导出宏；`MyAttributeSet.h` 与实际类名一致。狂暴 Tag 必须预先注册。MMC 的生命周期和重算需要理解 Snapshot、属性依赖以及标签快照；上面的演示计算的是**创建和求值时所拥有的标签条件**，不承诺狂暴切换自动即时重算。

应用方式还是：

```cpp
ApplyExample(SourceASC, TargetASC, GE_AttackBuff_Custom, 1.0f);
```

### 9.5 Set by Caller

在 `GE_Damage` 的 Health Modifier 中设置 `Set by Caller`，Data Tag=`Data.Damage`，Add 操作。下面示例传入 -150：

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

void ApplyDamageWithGE(
    UAbilitySystemComponent* SourceASC,
    UAbilitySystemComponent* TargetASC,
    TSubclassOf<UGameplayEffect> DamageGE)
{
    if (!IsValid(SourceASC) || !IsValid(TargetASC) || !DamageGE)
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        DamageGE, 1.0f, SourceASC->MakeEffectContext());

    if (!SpecHandle.IsValid())
    {
        return;
    }

    const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(
        FName(TEXT("Data.Damage")));

    SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, -150.f);

    SourceASC->ApplyGameplayEffectSpecToTarget(
        *SpecHandle.Data.Get(), TargetASC);
}
```

**为什么是 -150？** 因为这个示例用 `Health Add` 表示扣血。项目也可以统一传**正伤害值**到 `Damage` 元属性，通过 ExecCalc/AttributeSet 再转换为扣血，以方便防御、抗性、护盾等复杂逻辑。

### 9.6 四种方式归纳

```text
Scalable Float    → 配置/曲线决定 Magnitude
Attribute Based  → 被捕获的属性决定 Magnitude
Custom MMC       → 自定义函数返回 Magnitude
Set by Caller    → Spec 的调用方写入 Magnitude
                       ↓
                  Modifier Op
                       ↓
                    Attribute
```

---

## 10. 新手最容易混淆的十件事

1. **GE 不等于 GA**：GA 是技能流程，GE 是效果规则。
2. **Instant 不等于 Duration=0**：Instant 直接执行，Duration GE 有活动生命周期。
3. **Duration 与 Period 不同**：一个管寿命，一个管执行频率。
4. **周期扣血不会因 GE 结束自动返还**：它通常改的是基础属性，而不是临时修饰。
5. **Asset Tags ≠ Target Granted Tags**：前者描述 GE，后者授予接收者 ASC。
6. **Modifier 下的 Source/Target Tags ≠ 授予标签组件**：前者用于筛选 Modifier 的生效条件。
7. **Block GA ≠ Immunity GE ≠ Remove GE**：禁止技能 / 拒绝效果 / 移除已有效果是三件事。
8. **MMC ≠ ExecCalc**：MMC 求一个 Modifier Magnitude；Execution Calculation 更适合进行复杂的即时属性执行逻辑。
9. **Stacking ≠ 每层独立定时器**：是否整组到期、逐层退、独立计时要明确设计。
10. **Gameplay Cue 不负责权威伤害计算**：它适用于视听反馈，尤其多人网络中不可拿它维护关键状态。

### 10.1 视频演示时建议同步观察什么？

- `Attack` / `Health` 的 BaseValue 与 CurrentValue。
- Target ASC 当前拥有的 Gameplay Tags。
- Active Gameplay Effects 的层数、持续时间与来源。
- Period 初次执行是否在 0 秒发生。
- 同一 GE `Stacking=None` 与 `Aggregate by Target` 的实例及到期变化。
- GE 移除后 Cue 和授予标签是否正确收尾。

---

## 11. 快速需求索引

| 你想做 | 主要配置位置 |
|---|---|
| 打一下扣 100 血 | Instant + Modifier Health Add -100 |
| 10 秒攻击 +50 | Has Duration 10 + Period 0 + Attack Add 50 |
| 每秒扣 10 血 | Has Duration + Period 1 + Health Add -10 |
| 技能等级影响加成 | Modifier Magnitude → Scalable Float + Curve Table |
| 智力转换加成 | Modifier Magnitude → Attribute Based |
| 复杂加成公式 | Modifier Magnitude → Custom Calculation Class |
| GA 自己传伤害 | Modifier Magnitude → Set by Caller |
| 标记角色中毒 | Components → Grant Tags to Target Actor |
| 给中毒 GE 分类 | Components → Tags This Effect Has (Asset Tags) |
| 禁止释放指定 GA | Components → Block Abilities with Tags |
| 免疫新施加的 GE | Components → Immunity to Other Effects |
| 净化已有 Debuff | Components → Remove Other Effects |
| 只在特定状态生效 | Components → Require Tags to Apply/Continue This Effect |
| 30% 机会附加中毒 | Components → Chance To Apply This Effect |
| 让装备授予技能 | Components → Grant Gameplay Abilities |
| 到期触发额外 GE | Components → Apply Additional Effects |
| 绿色毒雾与音效 | Gameplay Cues + Gameplay Cue Notify |
| 每次命中 +1 层 Buff | Stacking → Aggregate by Target |
| 不同玩家各自叠流血 | Stacking → Aggregate by Source |
| 叠满后下一次触发额外效果 | Stacking → Overflow Effects |

---

## 12. 推荐学习和讲解顺序

1. **最简单的 GE**：Instant 扣血、10 秒 Attack +50，先建立生命周期与 Attribute 修改概念。
2. **Period**：中毒、立刻执行、抑制后重置计时。
3. **Modifier**：Add/Multiply/Override、四种 Magnitude、Source/Target Tags。
4. **Components**：先 Asset/Target Tags，然后 Block、Immunity、Remove、Requirements，最后其他组件。
5. **Gameplay Cue**：把数值与表现拆开。
6. **Stacking**：None / Source / Target、到期策略、溢出。
7. **综合实战**：中毒、眩晕、净化、控制免疫、装备技能、爆炸印记。
8. **C++ 与网络**：Spec、Set by Caller、MMC、AttributeSet 回调、权威施加与调试。

---

## 13. 参考资料

- [Epic 官方：Gameplay Effects](https://dev.epicgames.com/documentation/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine)
- [Epic 官方：Gameplay Attributes and Attribute Sets](https://dev.epicgames.com/documentation/unreal-engine/gameplay-attributes-and-attribute-sets-for-the-gameplay-ability-system-in-unreal-engine)
- [Epic 官方：Understanding the Gameplay Ability System](https://dev.epicgames.com/documentation/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system)
- [Epic API：UGameplayEffectComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/UGameplayEffectComponent)

**版本提示**：GE 详情面板、不同 Component 的具体条件字段和实现会随 UE 小版本演变。本文以你使用的 5.6/5.7 编辑器菜单为主，以引擎源码和当前项目实测为最终依据。建议视频配套工程保留各案例的 GE 蓝图、AttributeSet、测试按钮及 ASC 调试输出。
