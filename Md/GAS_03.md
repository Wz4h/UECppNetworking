# GASLesson2：按 GameplayEffect 面板顺序认识 GE

这份说明直接按照 UE5.6 的 GameplayEffect 编辑器从上到下阅读。打开任意 `Content/GASLesson2/Effects/GE_*`，可以一边看面板，一边运行案例。

## 1. GameplayEffect 是什么

GameplayEffect（GE）是一份“如何改变目标 ASC”的配置，常用来表达伤害、治疗、Buff、Debuff、装备加成、免疫和驱散。

GE 资产本身是模板。真正施加时，ASC 会根据模板创建 `FGameplayEffectSpec`。Spec 是“这一次施加”的运行时数据，包含等级、来源、动态参数和属性捕获结果。

```text
GameplayEffect Class（配置模板）
          + EffectContext（来源、命中等上下文）
          + Level / SetByCaller（本次参数）
                         ↓
              FGameplayEffectSpec
                         ↓
                 目标 ASC 执行
```

本课的通用流程在 `AGASLesson2Character::ApplyEffectToSelf` 和 `ApplyEffectToTarget`：

```cpp
FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
Context.AddSourceObject(this);
FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, Context);
const FGameplayEffectSpec& Spec = *SpecHandle.Data.Get();
SourceASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
```

`FGameplayEffectSpecHandle` 是安全持有 Spec 的句柄；`SpecHandle.Data` 中才是实际的 `FGameplayEffectSpec`。

## 2. 操作表

| 按键 | GE / 操作 | 主要知识点 |
|---|---|---|
| `1` / `2` | `GE_Damage` / `GE_Heal` | Instant、固定 Modifier |
| `3` | `GE_AttackBuff` | Has Duration |
| `4` / `5` | 添加/移除 `GE_Weapon` | Infinite、Active GE Handle |
| `6` | `GE_DynamicDamage` | SetByCaller |
| `7` | `GE_Poison` | Period、Debuff Asset Tags |
| `8` | `GE_StackAttack` | Stacking |
| `9` / `0` | 打印属性/清除 Demo 效果 | 观察与 GameplayEffectQuery |
| `F1` | `GE_AttackMultiply` | Modifier Op |
| `F2` | `GE_LevelDamage` | Curve Table + GE Level |
| `F3` / `F4` | Oiled / Burning | Granted Tags、Application Requirements |
| `F5` | 对准另一角色施加普通伤害 | Apply to Target |
| `Q` | `GE_AttributeBasedHeal` | MaxHealth × 25% |
| `W` | `GE_MMCHeal` | 来源 Attack × 50% |
| `E` | `GE_ExecutionDamage` | 基础伤害 + Attack - Defense |
| `R` | `GE_Cleanse` | Remove Other Effects |
| `T` | `GE_DamageImmunity` | Immunity to Other Effects |

多人测试时，所有操作最终都由服务器执行。`E` 和 `F5` 的目标也由服务器射线检测，客户端不能直接指定任意 Actor。

---

## 3. Editor Status Text

这是编辑器对当前 GE 配置的检查结果，不参与运行时计算。

- `All Ok`：暂未发现明显的结构问题；
- 警告或错误：Duration、Period、Component、Magnitude 等组合可能无效；
- 它不能证明玩法公式正确，仍需运行测试。

例如 Immunity Component 放在 Instant GE 上没有意义，因为 Instant 立即结束，不能持续阻止后续效果。

---

## 4. 持续时间（Duration）

### 4.1 Duration Policy

#### Instant

应用时立即执行，不进入 Active GameplayEffects 列表。本课的直接伤害、治疗、SetByCaller、曲线伤害、Attribute Based、MMC、Execution 和 Cleanse 都是 Instant。

常见用途：普通伤害、一次治疗、立即消耗资源、立即驱散。

#### Has Duration

效果保持指定时间，ASC 保存一条 Active GameplayEffect，时间结束后自动移除。

- `GE_AttackBuff`：5 秒 Attack +50；
- `GE_Poison`：10 秒，每秒扣血；
- `GE_StackAttack`：10 秒，可叠 5 层；
- `GE_BurningRequiresOil`：满足标签要求后持续 5 秒。

#### Infinite

不会自动结束，必须通过 Handle、查询或其他规则移除。

- `GE_Weapon`：装备期间 Attack +20，按 `5` 移除；
- `GE_Oiled`：提供 `State.Oiled`，按 `0` 清除；
- `GE_DamageImmunity`：阻止伤害类 GE，按 `0` 清除。

### 4.2 Duration Magnitude

只有 `Has Duration` 使用此项。它决定持续秒数，也可以用 ScalableFloat、Attribute Based、MMC 或 SetByCaller。

例子：固定 5 秒；按技能等级查曲线；MMC 根据目标抗性缩短控制；服务器通过 SetByCaller 指定持续时间。

### 4.3 Period

`Period > 0` 时，GE 会每隔指定秒数重新执行一次。

```text
GE_Poison：Duration=10，Period=1，Health Add Base -5
```

适合中毒、燃烧、持续治疗、周期回蓝。Period 为 0 表示不进行周期执行。

### 4.4 Execute Periodic Effect on Application

勾选后，周期 GE 在刚应用时立即执行一次，随后才按 Period 执行。

- 勾选：0、1、2……秒执行；
- 不勾选：1、2、3……秒执行。

本课 Poison 和 Burning 勾选，所以按键后立即看到第一次扣血。

### 4.5 Periodic Inhibition Policy

当 Ongoing Requirements 暂时不满足时，周期效果会被抑制。条件恢复后，可以继续原计时、重置周期，或立即执行后重新计时，具体取决于这里的策略。

本课没有独立资产。典型场景是“目标进入无敌时暂停中毒，离开无敌后决定是否立刻跳一次伤害”。

---

## 5. GameplayEffect Components

UE5.3 之后，许多 GE 功能被整理为 Components。组件负责应用条件、标签、免疫、驱散等规则。

### 5.1 Asset Tags

Asset Tags 描述 GE 自己是什么，不会传给角色。

- 普通伤害：`Effect.Type.Damage`；
- Poison：`Effect.Type.Damage`、`Effect.Type.Debuff`、`Effect.Type.Debuff.Poison`；
- Burning：`Effect.Type.Damage`、`Effect.Type.Debuff`、`Effect.Type.Debuff.Burning`。

免疫和驱散因此可以按“效果类型”工作，而不必写死具体 GE 类。

### 5.2 Target Tags（Granted Tags）

Granted Tags 是持续 GE 激活期间授予目标 ASC 的标签。移除 GE 后标签也消失。

`GE_Oiled` 授予 `State.Oiled`；`GE_DamageImmunity` 授予 `State.DamageImmune`。

```text
Asset Tag：GE 自己是什么，例如“这是伤害效果”
Granted Tag：目标现在是什么状态，例如“角色现在免疫伤害”
```

### 5.3 Target Tag Requirements

Application Requirements 在准备应用时检查一次。`GE_BurningRequiresOil` 要求目标拥有 `State.Oiled`：直接按 `F4` 会失败，先按 `F3` 再按 `F4` 才成功。

Ongoing Requirements 会持续判断，不满足时抑制效果；Removal Requirements 满足时移除 Active GE。本课只实际配置了 Application Requirements。

### 5.4 Remove Other Effects

`GE_Cleanse` 匹配 Asset Tag `Effect.Type.Debuff`。

1. 按 `7` 添加 Poison；
2. 观察持续扣血；
3. 按 `R` 应用 Cleanse；
4. Poison/Burning 被移除，但 Attack Buff 不受影响。

应用场合：净化所有 Debuff、解除移动限制、移除某类元素状态。

### 5.5 Immunity to Other Effects

`GE_DamageImmunity` 是 Infinite GE，激活期间阻止 Asset Tag 含 `Effect.Type.Damage` 的新 GE。

1. 按 `T` 开启免疫；
2. 按 `1`、`6` 或让另一玩家按 `E` 攻击；
3. 伤害 GE 被拒绝；
4. 按 `0` 清除免疫，再次攻击即可生效。

免疫阻止新效果应用。已经运行的 Poison 不会因此自动清除，清除它要用 Remove Other Effects。

### 5.6 其他常见 Components（文档案例）

- **Chance to Apply**：按概率应用。例：攻击有 20% 概率中毒，随机结果应由服务器决定。
- **Custom Can Apply**：自定义复杂条件。例：阵营、距离或关卡规则。
- **Additional Effects**：成功后附加其他 GE。例：伤害同时附带减速。
- **Block Abilities**：阻止指定 Ability Tags。例：沉默、缴械、禁止冲刺。
- **Grant Abilities**：持续期间授予 Ability。例：装备武器时获得武器技能。

---

## 6. Modifiers

Modifier 回答三个问题：改哪个 Attribute、用什么运算、数值从哪里来。

### 6.1 Attribute 与 Modifier Op

本课属性有 `Health`、`MaxHealth`、`Attack`、`Defense` 和 Meta Attribute `Damage`。

UE5.6 的聚合顺序可概括为：

```text
((Base + AddBase) * MultiplyAdditive / DivideAdditive * MultiplyCompound) + AddFinal
```

- `Add Base`：常见加减法；
- `Multiply Additive`：倍率先做加法聚合，本课 `GE_AttackMultiply` 使用 1.5；
- `Divide Additive`：除法型修正；
- `Multiply Compound`：倍率逐个相乘；
- `Add Final`：所有乘除之后再加；
- `Override`：覆盖其他结果。

### 6.2 Scalable Float

最简单的是固定数值：`GE_Damage=-20`、`GE_Heal=+20`。

`GE_LevelDamage` 则绑定 `CT_LevelDamage.Damage`：

| GE Level | 曲线值 | Health Modifier |
|---:|---:|---:|
| 1 | 10 | -10 |
| 2 | 20 | -20 |
| 3 | 40 | -40 |

按 `F2` 会依次以等级 1、2、3 创建 Spec。

### 6.3 Attribute Based

`GE_AttributeBasedHeal` 配置为：目标 `Health` Add Base，Backing Attribute 是 Target `MaxHealth`，Coefficient 为 0.25。

```text
结果 = Coefficient × (PreMultiplyAdditiveValue + 捕获属性值)
       + PostMultiplyAdditiveValue
```

MaxHealth=100 时，按 `Q` 治疗 25。简单读取一个属性并做线性公式时，优先用 Attribute Based。

### 6.4 Custom Calculation Class（MMC）

`GE_MMCHeal` 使用 `UGASLesson2AttackHealMMC`，捕获 Source.Attack，返回 `max(Attack, 0) × 0.5`。Attack=20 时，按 `W` 治疗 10。

MMC 只负责返回一个 Modifier 的 Magnitude，不直接输出多个属性。

```cpp
DECLARE_ATTRIBUTE_CAPTUREDEF(Attack)
DEFINE_ATTRIBUTE_CAPTUREDEF(UGASLesson2AttributeSet, Attack, Source, true)
```

第一行声明 `AttackProperty`、`AttackDef` 等捕获成员；第二行初始化“哪个 Attribute、来自 Source/Target、是否 Snapshot”。宏只减少固定样板代码，公式仍写在 `CalculateBaseMagnitude_Implementation`。

### 6.5 Set By Caller

`GE_DynamicDamage` 声明 `Data.Damage`，具体值由本次 Spec 写入：

```cpp
SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, -30.f);
```

适合武器伤害、蓄力值或服务器动态判定。调用方和 GE 必须约定相同 Gameplay Tag。

### 6.6 Modifier Source/Target Tags 与 Evaluation Channel

Modifier 标签条件只决定某个 Modifier 是否参与聚合，不会拒绝整个 GE。例如 Source 必须有 `State.Powered` 才启用伤害加成。拒绝整个 GE 应使用 Application Requirements。

Evaluation Channel 用于把复杂属性修正分阶段聚合，并允许 Attribute Based 读取到某个阶段。本课不配置独立案例。

---

## 7. Executions

Execution 是一次执行中的完整结算逻辑，可以捕获多个 Source/Target 属性、读取 SetByCaller、做分支并输出多个 Modifier。

`GE_ExecutionDamage` 使用 `UGASLesson2DamageExecution`：

```text
BaseDamage = Data.BaseDamage = 30
SourceAttack = 20
TargetDefense = 5
FinalDamage = max(BaseDamage + SourceAttack - TargetDefense, 0) = 45
```

Execution 把正数 45 输出到 `Damage`：

```cpp
OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
    UGASLesson2AttributeSet::GetDamageAttribute(),
    EGameplayModOp::Additive,
    FinalDamage));
```

`Damage` 是 Meta Attribute，不代表长期状态，也不复制。`PostGameplayEffectExecute` 收到它后执行 `Health -= Damage`，然后把 Damage 清零。这样护盾、受击事件和死亡判定以后都能集中在一个最终入口。

两名玩家测试时，玩家 A 对准 B 按 `E`，服务器射线选目标，预期 B 扣 45 Health。

### 数值方案怎么选

| 需求 | 选择 |
|---|---|
| 固定值或等级曲线 | ScalableFloat |
| 单属性线性公式 | Attribute Based |
| 计算一个 Modifier 的复杂数值 | MMC |
| 调用时传一个动态参数 | SetByCaller |
| 多属性、多步骤、多个输出 | Execution |

---

## 8. Gameplay Cues

GameplayCue 负责粒子、音效、材质、镜头和飘字，不应承担权威数值逻辑。

- **Require Modifier Success to Trigger Cues**：Modifier 成功时才触发表现；
- **Suppress Stacking Cues**：减少增加层数时重复触发 Cue；
- **Gameplay Cues 数组**：指定 Cue Tag，并传递 Magnitude、Level、Context 和命中信息。

Cue Notify 常见事件：`OnActive`（开始）、`WhileActive`（持续）、`Executed`（Instant/Periodic 执行）、`Removed`（结束）。

本课没有创建 Cue 资产。典型扩展是 Poison 激活时挂粒子，每次 Period 播放受伤反馈，移除时销毁粒子。

---

## 9. Stacking（堆叠）

`GE_StackAttack`：Aggregate By Target，最多 5 层，每层 Attack +10，持续 10 秒，成功叠层时刷新时间。

- `None`：不使用堆叠规则；
- `Aggregate By Source`：不同来源分别维护层数；
- `Aggregate By Target`：目标把所有来源聚合为一组。

其他常见配置：

- **Duration Refresh Policy**：新层加入时是否刷新整组时间；
- **Period Reset Policy**：新层加入时是否重置周期；
- **Expiration Policy**：到期时整组清空、只减一层或刷新剩余层；
- **Overflow**：满层后拒绝、清空，或触发额外 GE。

例：灼烧达到 5 层后清空层数并触发爆炸。此高级案例本课只讲配置关系。

---

## 10. 网络与数据流

```text
客户端按键 → Server RPC → 服务器 ASC 创建 Spec → 服务器应用 GE
→ Attribute 改变 → Health/Attack/Defense 复制 → 客户端 OnRep 和 Delegate
```

Execution 伤害的完整流程：

```text
客户端按 E
→ 服务器射线选择目标
→ Context 记录来源和 HitResult
→ Spec 写 Data.BaseDamage=30
→ Execution 捕获 Source.Attack 和 Target.Defense
→ 输出到 Damage Meta Attribute
→ AttributeSet 扣 Health、清空 Damage
→ Health 复制到客户端
```

`Defense` 是长期状态，所以复制；`Damage` 是服务器一次结算的中间值，所以不复制。

## 11. 验证顺序

1. 单人 PIE，按 `9` 确认 Health=100、MaxHealth=100、Attack=20、Defense=5。
2. 按 `1` 后按 `Q`，确认扣 20 后治疗 25。
3. 再扣血后按 `W`，确认 Attack=20 时治疗 10。
4. 按 `7` 添加 Poison，再按 `R`，确认周期伤害停止。
5. 按 `T` 后按 `1`，确认伤害被阻止；按 `0` 后再次按 `1`，确认恢复。
6. 两客户端 PIE，玩家 A 对准玩家 B 按 `E`，确认 B 扣 45。
7. 两端按 `9`，对比客户端复制值和服务器日志。

## 12. 学完应能回答

- Class、Context、Spec、ActiveGameplayEffectHandle 分别保存什么；
- Instant、Duration、Infinite 和 Period 有什么区别；
- Asset Tags 与 Granted Tags 为什么不同；
- ScalableFloat、Attribute Based、MMC、SetByCaller、Execution 怎么选；
- Damage Meta Attribute 为什么不复制；
- 驱散和免疫为什么使用 Effect Query，而不是写死具体 GE 类；
- 多人游戏中为什么必须由服务器选择目标并应用 GE。
