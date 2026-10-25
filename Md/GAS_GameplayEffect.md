# Unreal Engine GAS：Gameplay Effect 系统学习与开发参考

> **适用版本**：以 Unreal Engine 5.6 / 5.7 Gameplay Effect（GE）编辑器为主要参考。个别项目版本、属性定制面板和引擎小版本可能存在命名、默认值及字段显示差异。
>
> **读者对象**：希望系统学习 GAS 的 Unreal Engine 开发者，以及需要快速查阅 GE 配置、应用方式与限制的开发者。
>
> **编排原则**：按照 GE 的核心配置顺序讲解：定义与生命周期 → 周期 → 属性修改 → 组件 → Gameplay Cue → 堆叠 → 综合案例 → C++ 与调试。每个主题先说明机制与选项，随后直接给案例；字段的显示条件在相应参数处说明，不单列“如何开启”教程。

本文中的 `Health`、`MaxHealth`、`Attack`、`Intelligence`、`MoveSpeed` 是项目自定义 Attribute，`State.*`、`Effect.*`、`Ability.*`、`GameplayCue.*`、`Data.*` 为示例 Gameplay Tag。使用前须在项目内实现属性并注册标签。案例中的数值仅用于说明系统行为，不代表固定的游戏平衡方案。

---

# 1. Gameplay Effect 概述

## 1.1 GE 的职责

Gameplay Effect（GE）描述游戏效果的数据与规则，包括属性变化、持续时间、周期执行、标签授予、施加条件、免疫、技能限制、堆叠以及表现事件。

常见 GE：立即恢复生命、持续提高攻击力、中毒扣血、眩晕、免疫控制、装备被动、净化异常状态。

| GAS 概念 | 职责 | 常见对象 |
|---|---|---|
| `UAbilitySystemComponent`（ASC） | 管理属性、技能、活动 GE、Gameplay Tag 及网络相关行为 | 角色的能力系统组件 |
| `UAttributeSet` | 定义 Attribute，并处理数值限制、执行后回调等 | `Health`、`Attack` |
| `UGameplayAbility`（GA） | 编排技能激活、消耗、目标选择与应用效果 | `GA_Fireball` |
| `UGameplayEffect`（GE） | 定义效果的属性计算与生命周期等配置 | `GE_Poison` |
| Gameplay Cue | 提供特效、音效等非权威玩法表现 | `GameplayCue.Poison` |

**GE 不是 GA。** GA 可以决定何时发起一次攻击、攻击谁，以及何时应用 GE；GE 负责规定被应用的效果。无须每一种简单的属性变化都单独编写一个 GA。

## 1.2 资产、Spec 与活动实例

| 对象 | 作用 |
|---|---|
| `UGameplayEffect` | 设计阶段的定义，通常表现为数据蓝图 |
| `FGameplayEffectSpec` | 本次应用的运行时规格，保存等级、Context、Set by Caller 等信息 |
| `FActiveGameplayEffect` | 持续性 GE 在目标 ASC 上的活动状态，保存计时、堆叠等运行信息 |
| `FActiveGameplayEffectHandle` | 引用、查询或移除一个活动 GE 的句柄 |

典型调用流程：

```text
GA / Gameplay 代码
    ↓ 创建 FGameplayEffectSpec（指定 GE 类、等级、上下文和动态数值）
来源 ASC → 目标 ASC
    ↓ 应用条件 / 免疫检查
    ├─ Instant：执行数值修改
    └─ Duration / Infinite：建立或更新 Active GE
            ↓ 持续 Modifier、周期计时、标签、Cue、堆叠
            ↓ 到期或主动移除
```

`GameplayEffectSpec` 与 `ActiveGameplayEffect` 不应混淆：前者表示“这一次准备应用什么”，后者表示“目标上正在生效的什么”。

## 1.3 示例：火球术如何使用 GE

角色释放 `GA_Fireball`，命中敌人后应用 `GE_FireDamage`（Instant、Health Add -100）；如果命中后附带灼烧，再应用 `GE_Burning`（Has Duration、Period=1、Health Add -10）。受击火花和持续火焰则通过 Gameplay Cue 表现。三部分分工明确：GA 负责技能流程，GE 负责玩法数值，Cue 负责表现。

---

# 2. Duration：效果生命周期

`Duration Policy` 定义 GE 是否持续存在，以及如何确定持续时间。

## 2.1 Instant

**Instant**：应用时执行效果，不创建需要持续计时的活动 GE。对于普通属性修改，常用于改变 Attribute 的 Base Value。

适用：直接伤害、治疗、资源消耗、即时奖励。

选择 `Duration Policy = Instant` 时，不需要配置 `Duration Magnitude`，周期执行及活动 GE 堆叠等功能不适用。

## 2.2 Has Duration

**Has Duration**：GE 在指定时长内作为活动效果存在，到期后按照规则移除或更新层数。

选择 `Duration Policy = Has Duration` 后，`Duration Magnitude` 成为相关配置。其数值表示持续时间（秒），可以选择固定值、基于属性、MMC 或 Set by Caller；详见第 4 章的四种 Magnitude 计算方式。

适用：10 秒攻击 Buff、5 秒减速、持续中毒。

## 2.3 Infinite

**Infinite**：没有预定的自然到期时间，通常依靠 ASC 主动移除，或通过 GE 的移除条件结束。

选择 `Duration Policy = Infinite` 时，无须填写有限时长的 `Duration Magnitude`。它仍然可以设置 `Period` 进行周期执行。

适用：装备被动、常驻光环、持续环境效果。

## 2.4 Base Value 与 Current Value

Attribute 一般包含 Base Value 与 Current Value：

- **Base Value**：属性的基础数值。
- **Current Value**：在基础值和活动修饰器等因素作用下的当前数值。

典型情况：Instant GE 的普通属性修改会改变 Base Value；`Has Duration / Infinite` 且 `Period = 0` 的普通 Modifier 作为持续修饰参与聚合，通常改变 Current Value，移除 GE 后相应加成消失；周期性 GE 的每次周期执行更接近一次 Instant 属性修改，不会因为 GE 到期自动返还先前扣掉的生命值。

`Base` 和 `Current` 并不对应“两套永远独立的游戏属性”；它们共同参与 GAS 的属性计算。上限截断与特殊属性规则应由 AttributeSet 负责。

### 本模块案例：同一数值，三种生命周期

假设 `Attack = 100`。

| GE | Duration Policy | Modifier | 运行结果 |
|---|---|---|---|
| `GE_InstantAttack` | Instant | Attack Add +50 | Base 通常变为 150；不会等待一个活动 GE 到期再恢复 |
| `GE_TemporaryAttack` | Has Duration，10 秒，Period=0 | Attack Add +50 | 10 秒内 Current=150，结束后恢复到 100 |
| `GE_EquipmentAttack` | Infinite，Period=0 | Attack Add +50 | 装备存在期间有效；卸下时主动移除对应活动 GE |

**教学验证**：记录应用前后的 Base / Current，以及活动 GE 列表。不要仅通过屏幕上“攻击力 +50”判断三种生命周期相同。

---

# 3. Period：周期执行

`Period` 指定 GE 重复执行其周期效果的时间间隔（秒），与 `Duration` 的总持续时间是两个维度。

## 3.1 Period = 0

不启用周期执行。对于有持续时间的 GE，Modifier 通常持续参与 Attribute 聚合，例如持续 10 秒的 `Attack +50`。

## 3.2 Period > 0

启用周期执行，例如 `Period = 1` 表示以 1 秒为间隔触发周期执行。通常用于持续扣血、回血、回蓝、持续消耗资源。

周期性属性修改通常类似 Instant 修改 Base Value，不属于“到期自动退还”的临时增益。实际执行次数受是否立即执行、到期边界、抑制以及堆叠设置影响。

`Period` 用于持续型 GE；设置为大于 0 后，详情面板通常显示下面两个周期相关设置。

## 3.3 Execute Periodic Effect on Application

控制 GE **首次应用时是否立即执行一次周期效果**。

- 勾选：应用瞬间执行一次，随后继续按 `Period` 执行。
- 不勾选：通常等待第一个周期到达才执行。

该选项不是控制 Duration 是否立即开始计时。相同 Duration/Period 下，开启它可能让总执行次数增加一次。

## 3.4 Periodic Inhibition Policy

该属性用于 GE 被 **Ongoing Tag Requirements 抑制**，之后恢复生效时，如何处理周期计时。

| 选项 | 恢复生效后的行为 |
|---|---|
| `Never Reset` | 不因为解除抑制而重置原有周期节奏 |
| `Reset Period` | 从恢复时刻重新等待一个完整周期 |
| `Execute and Reset Period` | 恢复时先执行一次，再从该时刻重新计时 |

抑制不等于移除：GE 可以继续存在，有限持续时间通常继续流逝，待条件再次满足时恢复生效。

### 本模块案例 A：持续中毒

| 属性 | 配置 |
|---|---|
| GE | `GE_Poison` |
| Duration Policy | Has Duration |
| Duration Magnitude | 5 秒 |
| Period | 1 秒 |
| Execute Periodic Effect on Application | False |
| Modifiers | Health → Add → -10 |

中毒按周期扣血，GE 到期后停止新的周期执行；已经失去的生命值不会自动返还。若勾选“应用时立即执行”，命中瞬间也会扣血。

### 本模块案例 B：临时治疗与持续治疗

- `GE_MaxHealthBuff`：Has Duration，10 秒，Period=0，MaxHealth Add +100。属于临时修饰。
- `GE_Regen`：Has Duration，10 秒，Period=1，Health Add +10。属于持续恢复。

即使两个 GE 的 Duration 一样，数值作用机制也完全不同。

### 本模块案例 C：夜间恢复、白天暂停

给周期回血 GE 添加 `Require Tags to Apply/Continue This Effect`，在 Ongoing 条件中要求 `State.Night`。角色失去该标签时 GE 被抑制，重新获得标签后，周期恢复时机由 `Periodic Inhibition Policy` 决定。GE 不会因为被抑制而自动无限延长剩余时长。

---
# 4. Modifiers：属性修改规则

`Modifiers` 是 GE 修改 Attribute 的主要途径。一个 GE 可以包含多个 Modifier，每一项通常由 **Attribute、Modifier Op、Modifier Magnitude、标签要求**组成。

## 4.1 Attribute：修改哪一个属性

`Attribute` 指定本条 Modifier 作用的 Attribute，例如 `Health`、`Attack`、`MoveSpeed`。选择前，项目需要先创建相应的 `UAttributeSet` 及属性。

例如 `GE_Berserk` 可以同时包含 `Attack`、`MoveSpeed` 和 `Defense` 三条 Modifier，不必为每条属性修饰分别创建 GE。

## 4.2 Modifier Op：如何修改

| Modifier Op | 含义 | 单独作用于 100 时的示例 |
|---|---|---|
| `Add (Base)` | 加法修饰 | Magnitude=50 → 150 |
| `Multiply (Base)` | 乘法修饰 | Magnitude=1.2 → 120 |
| `Divide (Base)` | 除法修饰 | Magnitude=2 → 50 |
| `Override` | 覆盖最终计算值 | Magnitude=80 → 80 |

上表按**单个简单 Modifier、没有其他修饰器**说明。多个 Modifier 同时存在时，GAS 使用 Attribute Aggregator 聚合，不应假设所有乘法项总是逐项相乘。尤其不能把“两条 Multiply 1.5”直接认定为 2.25 倍；还需考虑聚合规则、Evaluate Parameters、通道与 Override 的情况。除法的数值也不得设为非法值。

### 例子：狂暴 GE 同时修改三个属性

`GE_Berserk`：Has Duration=8 秒，Period=0。

| Modifier | Attribute | Op | Magnitude | 效果 |
|---|---|---|---|---|
| 1 | Attack | Add | +50 | 增加攻击力 |
| 2 | MoveSpeed | Multiply | 1.2 | 提高移动速度 |
| 3 | Defense | Multiply | 0.8 | 降低防御力 |

这三项分别参与自身 Attribute 的聚合；并非先改 Attack 再改 MoveSpeed 的链式计算。

## 4.3 Modifier Magnitude：修改多少

展开一条 Modifier 后，在 `Modifier Magnitude → Magnitude Calculation Type` 选择计算来源。四个选项共用一个目标：得到当前 Modifier 的 Magnitude。选定类型后，编辑器只展示该类型适用的字段。

### 4.3.1 Scalable Float

**含义**：直接设置数值，或通过曲线让数值随 GE 等级变化。

选择 `Scalable Float` 后，可以设置 `Scalable Float Magnitude`；需要随等级变化时，再设置相应的曲线/Curve Table 信息。

**例子：不同等级的攻击 Buff**

| GE Level | 攻击加成 |
|---|---|
| 1 | +50 |
| 2 | +80 |
| 3 | +120 |

`GE_AttackBuff_Fixed`：Has Duration=10 秒；Attack Add；Magnitude=Scalable Float，通过曲线取值。应用时传入 GE Level=2，得到 +80。若未设置曲线，数值可固定为 +50。

**应用代码**（`GEClass` 对应所创建的 GE 蓝图类；后文提供通用函数）：

```cpp
FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(
    GEClass, 2.0f, SourceASC->MakeEffectContext());
if (Spec.IsValid() && TargetASC)
{
    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}
```

### 4.3.2 Attribute Based

**含义**：根据 Source 或 Target 的 Attribute 值计算 Magnitude。

选择 `Attribute Based` 后，可配置 `Backing Attribute`、`Attribute Source`、捕获方式（Snapshot）、Coefficient、Pre Multiply Additive Value、Post Multiply Additive Value 等字段。根据具体版本，还可以看到 Attribute Calculation Type、曲线相关配置。部分字段会随着计算类型切换而变化。

常用形式：

\[
Magnitude = (捕获的属性值 + PreMultiplyAdditiveValue) \times Coefficient + PostMultiplyAdditiveValue
\]

其中系数等值本身可采用 Scalable Float；若使用曲线或其他 Attribute Calculation Type，实际求值路径以对应选项为准。

- `Backing Attribute`：读取哪一个 Attribute。
- `Attribute Source`：读取 Source（效果来源）还是 Target（效果接收者）。
- `Snapshot`：使用捕获时刻的属性快照；非 Snapshot 的数据可参与后续重算，但不意味着所有 GE 都无条件逐帧重算。
- `Coefficient`、前后加值：构成属性换算公式。

**例子：每点智力提供 0.5 攻击力**

`GE_IntellectAttack`：Has Duration=10 秒、Period=0；Attack Add；Magnitude=Attribute Based；Backing Attribute=Intelligence；Attribute Source=Source；Coefficient=0.5；前后加值均为 0。施法者智力为 100 时，目标获得 +50 Attack。

调用 GE 的代码和 Scalable Float 完全相同：计算发生在 GE 机制内部，调用者无需先读取智力再赋值。适合把属性换算关系配置在资产中。

**Source 不一定是 Target。** 当角色给自己施加 Buff 时两者可能相同；治疗队友或给敌人施加 Debuff 时就可能不同。

### 4.3.3 Custom Calculation Class

**含义**：使用 `UGameplayModMagnitudeCalculation`（MMC）编写自定义 Magnitude 公式。

选择 `Custom Calculation Class` 后，在对应类字段指定 MMC（编辑器显示名称可能为 Calculation Class / Modifier Magnitude Calculation）。这个计算类返回一个数值；实际修改哪个 Attribute、进行何种运算，仍由 Modifier 的 Attribute 与 Op 决定。

适用于：读取多个属性、根据 Tag 修改系数、需要专门规则的 Buff 增益计算。

**例子：智力加成，狂暴状态时效果翻倍**

规则：普通状态 `Attack + Intelligence×0.5`；来源带 `State.Berserk` 时 `Attack + Intelligence×1.0`。创建 `MMC_AttackBonus` 捕获来源 Intelligence 并读取来源捕获标签，然后返回 Magnitude。`GE_AttackBuff_MMC` 配置 Attack Add、Custom Calculation Class=`MMC_AttackBonus`。

完整 MMC 代码紧跟本章“4.5 C++ 实现”，避免与原理部分割裂。注意 MMC 读取 Spec 捕获到的标签，不应把标签变化当作 MMC 必然自动重新计算的信号。

### 4.3.4 Set by Caller

**含义**：由创建 Spec 的外部调用者，为本次 GE 应用传入 Magnitude。

选择 `Set by Caller` 后，需要指定数据键（常见为 `Data Tag`，例如 `Data.Damage`；不要与普通 Gameplay Cue Tag 混淆）。创建 Spec 时通过 `SetSetByCallerMagnitude` 写入对应数值，再将 Spec 应用到目标。

**例子：同一个 GE 处理普通攻击、重击、暴击**

`GE_Damage`：Instant；Health Add；Magnitude=Set by Caller；Data Tag=`Data.Damage`。

| 技能类型 | 传入值 | 效果 |
|---|---:|---|
| 普通攻击 | -50 | Health 减少 50 |
| 重击 | -100 | Health 减少 100 |
| 暴击 | -200 | Health 减少 200 |

这里传入负数，是因为使用 `Health Add`。若你的项目采用 `Damage` 元属性，并在 `PostGameplayEffectExecute` 处理最终扣血，写法会不同。

```cpp
FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(
    DamageGEClass, 1.f, SourceASC->MakeEffectContext());

if (Spec.IsValid() && TargetASC)
{
    const FGameplayTag DamageTag =
        FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
    Spec.Data->SetSetByCallerMagnitude(DamageTag, -150.f);
    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}
```

**关键约束**：必须在应用 GE 前设置对应 Tag 的数值；未设置或 Tag 不匹配，可能出现默认值、错误日志或不符合预期的计算。GE 资产是可复用模板，动态值属于 Spec，而不是为不同伤害创建几十个 GE 资产。

## 4.4 Source Tags、Target Tags：Modifier 级条件

Modifier 下的 `Source Tags`、`Target Tags` 用于限制**该条 Modifier 的匹配条件**；它们不会给角色授予 Tag。

| 字段 | 含义 |
|---|---|
| `Must Have Tags` | 相应侧必须拥有指定标签 |
| `Must Not Have Tags` | 相应侧不得拥有指定标签 |
| `Query Must Match` | 使用更复杂的 Gameplay Tag Query |

`Source Tags` 检查 GE 的来源侧捕获标签；`Target Tags` 检查目标侧标签。不要把“Target Tags”错误理解成“当前玩家以后要攻击的敌人标签”。

**例子：有狂暴标签才额外加攻击力**

`GE_ConditionalBuff`：Has Duration=10 秒、Period=0，包含两个 Modifier：

| Modifier | Attack 加法值 | Source Tags |
|---|---:|---|
| 1 | +20 | 不限制 |
| 2 | +50 | Must Have `State.Berserk` |

当 GE 被应用且来源标签满足匹配条件时，额外修饰器可以参与计算。具体捕获与更新时机应结合来源/目标和运行方式测试。**对 Instant、Periodic GE 不应直接依赖 Modifier 级标签条件完成关键安全判断**；需要强约束时，优先通过 GE 应用条件或执行逻辑进行验证。

## 4.5 MMC：完整 C++ 示例

示例假定属性声明类为 `UMyAttributeSet`，已经使用属性访问器宏生成 `GetIntelligenceAttribute()`；`State.Berserk` 已在 Gameplay Tags 中注册。把 `YOURPROJECT_API` 替换为项目模块导出宏。

**MMC_AttackBonus.h**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_AttackBonus.generated.h"

UCLASS()
class YOURPROJECT_API UMMC_AttackBonus : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
public:
    UMMC_AttackBonus();

    virtual float CalculateBaseMagnitude_Implementation(
        const FGameplayEffectSpec& Spec) const override;

private:
    FGameplayEffectAttributeCaptureDefinition IntelligenceCapture;
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
    IntelligenceCapture = FGameplayEffectAttributeCaptureDefinition(
        UMyAttributeSet::GetIntelligenceAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        true /* Snapshot */);
    RelevantAttributesToCapture.Add(IntelligenceCapture);
}

float UMMC_AttackBonus::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters Params;
    Params.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    Params.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float Intelligence = 0.f;
    if (!GetCapturedAttributeMagnitude(
            IntelligenceCapture, Spec, Params, Intelligence))
    {
        return 0.f;
    }

    const FGameplayTag BerserkTag =
        FGameplayTag::RequestGameplayTag(TEXT("State.Berserk"));

    const bool bBerserk = Params.SourceTags &&
        Params.SourceTags->HasTag(BerserkTag);

    const float Coefficient = bBerserk ? 1.f : 0.5f;
    return FMath::Max(0.f, Intelligence) * Coefficient;
}
```

将此类设置为 `GE_AttackBuff_MMC` 的 Custom Calculation Class。**MMC 只返回 Modifier 数值，不直接调用 `SetHealth` 或 `SetAttack`**。若需要一次计算产生复杂的多项属性输出，或者需要执行阶段的专门控制，可进一步学习 `UGameplayEffectExecutionCalculation`（ExecCalc），它与 MMC 不是同一种接口。

## 4.6 小结：同一个属性，四种数值来源

| GE | 属性 | 计算来源 | 何时决定数值 |
|---|---|---|---|
| `GE_FixedAttack` | Attack | Scalable Float | 配置及 GE Level |
| `GE_IntellectAttack` | Attack | Attribute Based | 捕获并求值 Attribute |
| `GE_MMCBuff` | Attack | Custom Calculation Class | MMC 求值 |
| `GE_Damage` | Health | Set by Caller | 调用方为 Spec 写入数据 |

四种方式只是改变 Magnitude 的来源；应用时仍遵循 Spec → ASC → GE 计算/生命周期的统一流程。

---
# 5. Components：GE 的附加行为

Gameplay Effect Components 用于扩展 GE 的标签、应用条件、技能限制、其他 GE 交互等行为。与普通 `UActorComponent` 不同，GE Component 位于 GE 定义内部，不是附加到 Actor 上的组件，也不应保存每次应用独有的可变运行时状态。

在 GE 的 `Components` 数组中点击 `+` 并选择组件类型后，该组件的专属字段才会显示。下面使用 UE 5.6/5.7 常见的编辑器名称，按典型开发用途讲解；每项结束后直接给配置例子。

## 5.1 Tags This Effect Has (Asset Tags)

**作用**：为 GE 自身提供分类或语义标签，以便使用 Gameplay Effect Query 等机制识别该 GE。**不会因此自动授予目标角色这些标签。**

添加 `Tags This Effect Has (Asset Tags)` 后，可以配置 Asset Tag 的继承式标签集合。常见字段：

- `Add to Inherited`：在父 GE 标签基础上增加标签。
- `Remove from Inherited`：从继承的标签中排除指定标签。
- `Combined Tags`：合并后的结果，通常用于查看最终配置。

**例子：中毒效果的分类**

`GE_Poison` 添加该 Component，Asset Tag 设为 `Effect.Debuff.Poison`。以后净化 GE 可以通过查询 `Effect.Debuff` 类的 Asset Tags 来筛选该中毒 GE。这只解决“识别这是什么 GE”，不会自动让角色 ASC 拥有 `State.Poisoned`。

## 5.2 Grant Tags to Target Actor

**作用**：在 GE 有效时为目标 ASC 授予 Gameplay Tag，通常用于状态识别和其他 GAS 规则。GE 不再有效时，撤销由该 GE 授予的对应标签贡献；如果其他来源仍授予同一 Tag，该 Tag 可能依然存在。

添加 `Grant Tags to Target Actor` 后，在其标签容器内配置所授予的标签（版本中可能显示 `Add Tags`、`Combined Tags` 等继承式字段）。

**例子：进入眩晕状态**

`GE_Stun`：Has Duration=3 秒；该 Component 授予 `State.Stunned`。UI 可根据该 Tag 显示眩晕图标；GA 可以根据标签要求拒绝激活。但仅仅授予这个 Tag 并不会自动停止普通 CharacterMovement。

### Asset Tags 与 Target Tags 的区别

| 需求 | 使用的 Component | Tag 属于谁 |
|---|---|---|
| 标记 GE 是中毒 Debuff | `Tags This Effect Has (Asset Tags)` | GE 本身 |
| 标记角色正在中毒 | `Grant Tags to Target Actor` | 角色 ASC |
| 根据分类删除中毒 GE | 先有 Asset Tags，再由 `Remove Other Effects` 查询 | 查询 GE |
| 根据角色状态决定能否释放技能 | Grant Tags 后由 GA 标签规则或 Block 等机制判断 | 查询角色状态 |

## 5.3 Block Abilities with Tags

**作用**：当此 GE 生效时，禁止目标 ASC 激活匹配 Ability Tags 的 GA。它针对**技能激活**，而不是拒绝 GE。

添加此 Component 后，配置要阻止的 `Ability Tags` 容器。GA 本身需要具有相应的 Ability Tag 才能匹配。

**例子：沉默**

`GE_Silence`：Has Duration=5 秒；`Block Abilities with Tags` 配置 `Ability.Magic`。`GA_Fireball`、`GA_Heal` 都带 `Ability.Magic` 时，沉默期间无法正常激活这两个 GA。不带该标签的普通移动技能不受此 Block 直接影响。

**边界**：该 Component 主要阻止新激活，不等于必然取消已经在运行的 GA；对正在引导的法术，需要结合 GA 的取消机制处理。

## 5.4 Immunity to Other Effects

**作用**：在此 GE 提供免疫期间，拒绝符合特定查询条件的**新 GE 应用**。匹配通常依赖 Gameplay Effect Query、Asset Tags 或其他 Spec/Def 条件。

添加 `Immunity to Other Effects` 后，配置其免疫查询（Immunity Queries 等字段；具体展开名称随版本可能不同）。被免疫的 GE 必须满足该查询，免疫才生效。

**例子：五秒内免疫控制**

先给 `GE_Stun` 添加 `Tags This Effect Has (Asset Tags)` → `Effect.Control.Stun`，给 `GE_Freeze` 添加 `Effect.Control.Freeze`。再创建 `GE_ControlImmunity`（Has Duration=5 秒），添加 `Immunity to Other Effects`，查询匹配 `Effect.Control`。角色在该 GE 生效期间，不再接受匹配查询的新控制 GE。

**边界**：免疫不会主动删除免疫开启前已经存在的眩晕；普通伤害如果不匹配查询，仍会生效。

### Block、Immunity、Remove 的差异

| 行为 | 目标 | 结果 |
|---|---|---|
| `Block Abilities with Tags` | GA | 阻止技能激活 |
| `Immunity to Other Effects` | 新施加的 GE | 拒绝 GE 应用 |
| `Remove Other Effects` | 已存在的活动 GE | 查找并移除 |

三者解决不同阶段的问题，不能互相替代。

## 5.5 Remove Other Effects

**作用**：按查询条件移除目标 ASC 上已经存在的活动 GE。常用于净化、驱散、状态替换。

添加 `Remove Other Effects` 后，配置一个或多个 `Gameplay Effect Query`，用于筛选待移除的活动 GE。

**例子：净化所有常规 Debuff**

各异常效果分别具有 Asset Tags：`Effect.Debuff.Poison`、`Effect.Debuff.Burning`、`Effect.Debuff.Slow`。创建 `GE_Cleanse`（Instant），添加 `Remove Other Effects`，查询匹配 `Effect.Debuff`。应用净化时移除符合条件的已有 GE。

**边界**：净化不会阻止下一次重新中毒；如需持续免疫，还需另设 Immunity GE。不要把待移除的 GE 与净化 GE 的 Asset Tags 混淆。

## 5.6 Require Tags to Apply/Continue This Effect

**作用**：通过目标 ASC 的标签要求，控制 GE 是否可应用、应用后是否生效，以及在何种条件下被移除。添加该 Component 后通常可以看到三类 Requirements。

| 配置 | 检查内容 | 不满足/满足时的核心行为 |
|---|---|---|
| `Application Tag Requirements` | 是否允许最初应用 | 不满足则不能应用 |
| `Ongoing Tag Requirements` | 活动 GE 是否继续产生持续效果 | 不满足则抑制；以后满足可以恢复 |
| `Removal Tag Requirements` | 是否满足移除条件 | 满足要求时移除活动 GE |

各 Requirements 通常包含 Required Tags / Ignored Tags（或 Must Have / Must Not Have）一类条件。**被 Ongoing 抑制不等于被移除**；因此不要用 Ongoing 来实现“条件不满足就永久取消”的效果。

**例子 A：只有在水中才能施加水下呼吸 Buff**

`GE_UnderwaterBreathing`：Application Require `State.Underwater`。不在水中的角色无法获得该 GE。已经获得后是否继续生效，由其他规则决定。

**例子 B：夜间生效，白天暂停**

`GE_NightAttack`：Has Duration=30 秒、Period=0、Attack Add +50；Ongoing Require `State.Night`。白天 GE 保留但攻击加成被抑制；夜晚恢复。`State.Night` 必须由项目的昼夜系统授予角色 ASC。

**例子 C：进入安全区立即移除燃烧**

`GE_Burning`：Removal Require `State.SafeZone`。角色进入安全区并获得该标签时，符合移除条件的燃烧 GE 被移除，不只是暂时停用。

对于周期性 GE，Ongoing 抑制解除后如何恢复计时，参见第 3 章 `Periodic Inhibition Policy`。

## 5.7 Chance To Apply This Effect

**作用**：根据概率决定本次 GE 是否通过概率施加检查。添加该 Component 后，可以设置概率（可能使用 Scalable Float，支持等级变化）。

**例子：30% 概率中毒**

`GE_Poison` 添加 `Chance To Apply This Effect`，将应用概率配置为 0.3（30%）。本次概率检查成功后，仍需满足 GE 的其他施加限制。它不是“每秒有 30% 概率扣血”；周期效果的扣血概率若另有设计，应采用其他计算或触发方案。

## 5.8 Apply Additional Effects

**作用**：由当前 GE 的应用或结束事件触发其他 GE。添加后，可以配置关联的附加 GE 类，以及某些条件选项。常见结束事件：

- `On Complete Normal`：正常到期结束。
- `On Complete Prematurely`：提前结束。
- `On Complete Always`：无论何种结束原因。

此外，版本还可能提供应用时附加效果和相关要求。具体字段由该 Component 类型、数组内容及相应规则决定。

**例子：五秒后爆炸的标记**

`GE_ExplosiveMark`：Has Duration=5 秒；在 `Apply Additional Effects` 的 `On Complete Normal` 配置 `GE_ExplosionDamage`。正常到期时应用爆炸伤害 GE；提前净化时不触发该“正常到期”规则。爆炸粒子建议由 Cue 实现，不应把游戏逻辑藏在特效播放中。

## 5.9 Grant Gameplay Abilities

**作用**：GE 活动期间向目标 ASC 授予指定 GA，并按配置处理 GE 移除后的技能授予关系。添加后，可以配置要授予的 Gameplay Ability 与移除策略等信息。

**例子：装备魔法剑获得火球术**

`GE_MagicSword`：Infinite；添加 `Grant Gameplay Abilities`，配置 `GA_Fireball`。装备时应用 GE，卸下时通过活动句柄移除；选择合适的移除策略，让技能随装备失效。

**边界**：获得 GA 不代表自动施放 GA。若角色本来已经通过其他来源拥有同一个技能，还要考虑重复授予与撤销的设计。

## 5.10 Custom Can Apply This Effect

**作用**：通过自定义 `UGameplayEffectCustomApplicationRequirement` 判断当前 GE 是否允许应用。添加后指定自定义 Application Requirement 类。

**例子：生命值低于 30% 才能添加斩杀标记**

`GE_ExecuteMark` 添加 `Custom Can Apply This Effect`，自定义 C++ 类读取目标 Health 与 MaxHealth，返回能否应用。若目标不满足条件，GE 应用失败。

**边界**：它是应用阶段的检查，不意味着效果应用后会持续监控目标生命比例。后续自动停止或移除应设计额外规则。

## 5.11 UI Data (Text Only)

**作用**：为 GE 提供便于 UI 显示的文本信息。添加后可配置名称、说明等文本字段（具体属性依版本的 UI Data 类实现而定）。

**例子：Buff 图标提示文本**

`GE_AttackBuff` 添加 `UI Data (Text Only)`，设置显示名称“攻击强化”，描述“攻击力提高 50，持续 10 秒”。UI 可读取该数据作为说明来源。

**边界**：UI Data 本身不会自动绘制完整 Buff 图标，也不会承担剩余时间、层数与图标布局等 UI 逻辑；这些通常需要通过 ASC 的活动 GE 信息和 UI 系统实现。

## 5.12 Components 综合案例：眩晕、净化与免疫

假设项目有三个 GE：

| GE | 使用的 Component | 作用 |
|---|---|---|
| `GE_Stun` | `Grant Tags to Target Actor` → `State.Stunned` | 标记眩晕状态 |
| `GE_Stun` | `Block Abilities with Tags` → `Ability.Attack`、`Ability.Magic` | 阻止相关 GA 激活 |
| `GE_Stun` | `Tags This Effect Has (Asset Tags)` → `Effect.Control.Stun` | 允许被控制净化、免疫查询识别 |
| `GE_CleanseCC` | `Remove Other Effects` → 查询 `Effect.Control` | 清除已经存在的控制 GE |
| `GE_CCImmune` | `Immunity to Other Effects` → 查询 `Effect.Control` | 阻止未来新控制 GE 应用 |

**观察顺序**：先中眩晕 → 使用净化移除 → 开启免疫 → 再尝试中眩晕。三个阶段分别验证 Tag、活动 GE 和技能能否激活，避免把“不能施法”“不再中眩晕”“移除原有眩晕”当作一回事。

---
# 6. Gameplay Cues：视觉与声音反馈

Gameplay Cue 用于将非权威的视听表现与玩法计算分离。粒子、音效、材质变化、命中闪光等适合由 Gameplay Cue 处理；扣血、是否死亡、是否免疫等关键玩法逻辑不应依赖 Cue 回调完成。

## 6.1 Gameplay Cues 数组

在 GE 的 `Gameplay Cues` 区域添加数组元素，再选择已经注册的 `GameplayCue.*` Tag。例如 `GameplayCue.Poison` 对应一个 Gameplay Cue Notify。Gameplay Cue Manager 依据 Tag 查找实际的 Cue Notify 资产或实现。

**注意**：必须有对应的 Gameplay Cue Notify，并确保标签和 Cue 资产被正确加载/发现；只在 GE 中输入 Tag 不会凭空生成火焰或毒雾。

## 6.2 常见 Gameplay Cue 事件

| 事件 | 常见触发语义 | 使用场景 |
|---|---|---|
| `OnActive` | 持续效果开始激活 | 播放开始音效或一次性激活反馈 |
| `WhileActive` | 持续状态的激活呈现 | 持续粒子、角色光环 |
| `Executed` | GE 的瞬时或周期执行反馈 | 命中火花、扣血反馈 |
| `Removed` | 持续效果不再活动 | 停止粒子、结束音效 |

Cue Notify 的具体支持能力与事件路由取决于 Notify 类型和 GE 的执行方式。不要将每一秒的周期扣血误解成一定会完整重复持续型 Cue 的 OnActive/Removed 生命周期。

## 6.3 Require Modifier Success to Trigger Cues

控制是否要求 Modifier 成功执行才允许相应 Cue 触发。通常用在带有 Modifier 的 GE；它不等价于“最终属性确实减少了 X”或“治疗后净增生命大于 0”。

例如受到伤害时，Modifier 成功执行但 Health 被规则限制，最终变化量可能与原始 Magnitude 不同。是否播放“有效治疗”等特殊反馈应以实际业务计算结果判断。

## 6.4 Suppress Stacking Cues

控制堆叠相关的 Cue 通知抑制行为，用于减少同一叠层 GE 重复添加层数时的冗余表现。这不会禁用堆叠，也不意味着一个效果无论如何只创建一个粒子组件；实际实例数量取决于 Cue Notify 如何管理持续资源。

## 6.5 Gameplay Cue 的 Magnitude 相关配置

某些版本还会显示 `Gameplay Cue Magnitude` 相关信息，例如从某个 Attribute 计算 Cue 的强度范围或最小、最大幅度。它用于向 Cue 提供显示强度信息，不直接替代 GE 的属性 Modifier。具体字段随 Cue 数组条目及版本呈现。

### 本模块案例 A：火球命中

`GE_FireDamage`：Instant，Health Add -100；Gameplay Cues 添加 `GameplayCue.Hit.Fire`。创建对应 Gameplay Cue Notify，使用 `Executed` 播放命中火花和一次音效。伤害数值由 GE 决定，不由粒子决定。

### 本模块案例 B：持续燃烧

`GE_Burning`：Has Duration=5 秒，Period=1；Health Add -10；Gameplay Cues 添加 `GameplayCue.Burning`。Cue Notify 在持续生效期间维护火焰粒子，移除时停止。若需要“每次跳伤显示伤害数字”，可另外发送独立伤害反馈事件或 Cue；不要假设持续 Cue 的粒子等于每次伤害事件。

### 本模块案例 C：叠层毒雾

`GE_PoisonStack`：最多 5 层，Gameplay Cues 使用 `GameplayCue.Poison`。若毒雾只是表示“正在中毒”，可以避免在每次增层时重复创建粒子；若特效强度需要随层数改变，应在 Cue 逻辑中明确读取/传递层数并更新表现，而不只是打开 `Suppress Stacking Cues`。

---

# 7. Stacking：活动 GE 的聚合与层数

Stacking 决定重复应用同一种可持续 GE 时，是管理多个独立效果，还是合并为一组带有 Stack Count 的活动 GE。它不仅影响数值，还影响计时、来源归属、到期策略与溢出行为。

## 7.1 Stacking Type

| 类型 | 行为 | 常见应用 |
|---|---|---|
| `None` | 不采用堆叠聚合；重复应用可以生成多个独立活动 GE | 各自计时的药水、独立状态 |
| `Aggregate by Source` | 按来源聚合，同一来源的同类 GE 在目标上共享层数 | 玩家各自的流血/中毒 |
| `Aggregate by Target` | 按目标聚合，多个来源同类 GE 可以共享层数 | 所有攻击者共同累积的灼烧 |

当 `Stacking Type = None` 时，堆叠上限和刷新策略无实际意义；选择 `Aggregate by Source / Target` 后，编辑器通常提供 Stack Limit Count、Duration Refresh、Period Reset、Expiration、Overflow 等堆叠设置（不同版本可能包含高级字段或额外条件）。Stacking 主要面向活动 GE，Instant GE 不形成用于计时的叠层活动实例。

## 7.2 Stack Limit Count

最大层数。比如设置为 5，活动 GE 的 Stack Count 最多按该上限增长。超过上限后的行为由 Overflow 相关规则控制，并非必然默默丢弃全部请求。

## 7.3 Stack Duration Refresh Policy

控制成功重复应用时，是否刷新活动 GE 的持续时间。

| 策略 | 效果 |
|---|---|
| `Refresh on Successful Application` | 成功堆叠时重新开始持续时间 |
| `Never Refresh` | 新增层数不改变原有到期时刻 |

**例子**：持续 10 秒的攻击 Buff，在第 8 秒获得新的一层；如果 Refresh，那么新的到期时刻一般延至第 18 秒；如果 Never Refresh，可能在第 10 秒按当前到期策略结束。具体还需考虑溢出申请是否算成功等条件。

## 7.4 Stack Period Reset Policy

对 `Period > 0` 的叠层 GE，决定新增层数时是否重置周期计时。

| 策略 | 效果 |
|---|---|
| `Reset On Successful Application` | 成功堆叠后重新等待一个周期 |
| `Never Reset` | 保持已有周期节奏 |

这与第 3 章的 **Periodic Inhibition Policy** 不是同一个字段：前者处理“增加堆叠层数”，后者处理“解除抑制”。

## 7.5 Stack Expiration Policy

持续时间到期后，按何种规则处理已有层数。

| 策略 | 行为 |
|---|---|
| `Clear Entire Stack` | 一次移除整组堆叠 |
| `Remove Single Stack and Refresh Duration` | 减少一层并重新开始计时，直到层数消失 |
| `Refresh Duration` | 到期时刷新持续时间而不自动清除层数 |

**重要区别**：`Remove Single Stack and Refresh Duration` 是共享活动 GE 在到期时逐层衰减，**不是每一层从获得时都拥有自己的独立倒计时**。如果设计要求每层独立计时，可能需要 `None` 下的独立 GE 或自定义层管理。

## 7.6 Overflow Effects 与相关策略

达到堆叠上限后再次申请堆叠，称为 Overflow。相关配置通常包括：

- `Overflow Effects`：溢出时额外应用哪些 GE。
- `Deny Overflow Application`：是否拒绝本次超出上限的堆叠申请。
- `Clear Stack on Overflow`：根据设置决定溢出时是否清除现有堆叠。

这些行为会与申请是否成功、Duration Refresh 等产生交互，制作复杂叠层机制时应验证活动 GE 的实际变化。

### 本模块案例 A：越打越强（攻击叠层）

`GE_AttackStack`：Has Duration=10 秒、Period=0；Attack Add +10；Aggregate by Target；Stack Limit=5；Duration Refresh=`Refresh on Successful Application`；Expiration=`Clear Entire Stack`。

| 层数 | 基础 Attack=100 时的示意值 |
|---|---:|
| 1 | 110 |
| 2 | 120 |
| 3 | 130 |
| 4 | 140 |
| 5 | 150 |

每次成功命中再应用一次同类 GE；10 秒内未刷新则整组清除。

### 本模块案例 B：周期中毒随层数增强

`GE_PoisonStack`：Has Duration=6 秒；Period=1；Health Add -5；Aggregate by Target；Stack Limit=5；Stack Period Reset=`Never Reset`。

1 层每跳 -5，3 层每跳 -15，5 层每跳 -25。观察周期计时器不会因为每次增层必然从 0 开始。由于可能存在来源归属与数值修饰情况，建议用 ASC 调试功能核对层数与实际扣血量。

### 本模块案例 C：两名玩家独立流血

`GE_Bleed`：Has Duration=8 秒；Period=1；Health Add -10；Aggregate by Source；Stack Limit=5。

玩家 A 在 Boss 上造成 3 层流血，玩家 B 造成 2 层。两组流血应分别归属对应 Source，独立刷新和到期；A 的流血消失不等于 B 的流血也消失。前提是 Source ASC 正确设置、身份保持稳定。

### 本模块案例 D：逐层衰减的减速

`GE_Slow`：Has Duration=5 秒；Period=0；MoveSpeed Multiply=0.9；Aggregate by Target；Stack Limit=5；Expiration=`Remove Single Stack and Refresh Duration`。

可以在每次到期时按 5→4→3→2→1→0 层衰减。为了说明聚合，可使用“每层提供一个 -10% 的线性减速”的预期进行简单演示，但实际 Multiply 聚合需遵循 GAS 规则，不能未经验证就写为 `0.9^层数`。

### 本模块案例 E：燃烧溢出爆炸

`GE_Burning`：Has Duration=10 秒；Period=1；Aggregate by Target；Stack Limit=5；Overflow Effects=`GE_Explosion`；Deny Overflow Application=True。

第 1～5 次施加使层数累积到 5；已满 5 层后，第 6 次申请触发溢出效果。**达到第 5 层与第 6 次溢出不是同一个事件。** 如果设计要求“到 5 层立即爆炸”，需另做阈值触发逻辑。

### 本模块案例 F：三瓶药水各自倒计时

`GE_AttackPotion`：Has Duration=10 秒；Attack Add +20；Stacking Type=`None`。第 0、2、4 秒各喝一瓶，三份 GE 分别于第 10、12、14 秒到期。它们可能同时让 Attack +60，但并不共享一组 Stack Count 与计时器。

---
# 8. 综合实战：组合多个 GE 模块

本章不重复每个字段的定义，而是按照真实需求组合之前的机制。每个案例均写明基础设置、所需 Component 和运行结果。

## 8.1 可驱散的 10 秒攻击 Buff

**需求**：攻击力 +50 持续 10 秒；可以通过净化移除；UI 可以判断当前是否存在 Buff。

**GE：`GE_AttackBuff`**

| 模块 | 配置 | 作用 |
|---|---|---|
| Duration | Has Duration = 10 秒 | 十秒后自然结束 |
| Period | 0 | 持续参与属性聚合 |
| Modifier | Attack Add +50 | 提升攻击力 |
| Component：`Tags This Effect Has (Asset Tags)` | `Effect.Buff.Attack` | 供驱散查询识别 |
| Component：`Grant Tags to Target Actor` | `State.AttackBoosted` | 供 UI / 其他逻辑查询 |
| Gameplay Cues | `GameplayCue.Buff.Attack` | 持续光环表现 |

如果需要驱散，另创建 `GE_DispelAttack`（Instant），添加 `Remove Other Effects`，查询 `Effect.Buff.Attack`。直接从当前 GE 的 Target Tags 判断是否属于可驱散 Buff 并不恰当，因为分类标签与角色状态标签是不同用途。

**验证**：施加 Buff → 观察 Current Attack、角色 Tag、Cue → 施加驱散 → 确认活动 GE 移除、Attack 恢复、Tag 与 Cue 清理。

## 8.2 有概率、可叠层、可净化的中毒

**需求**：毒箭命中有 30% 概率使目标中毒；每层每秒 -5 生命，最多 5 层；每次成功叠加刷新 6 秒持续时间；可通过净化移除。

**GE：`GE_Poison`**

| 模块 | 配置 | 作用 |
|---|---|---|
| Duration | Has Duration=6 秒 | 控制存续 |
| Period | 1 秒；首次不立即执行 | 周期扣血 |
| Modifier | Health Add -5 | 每跳基础扣血量 |
| Stacking | Aggregate by Target，Limit=5 | 多次中毒共享层数 |
| Duration Refresh | Refresh on Successful Application | 重新获得中毒时刷新剩余时间 |
| Stack Period Reset | Never Reset | 新增层数时维持周期节奏 |
| Component：`Chance To Apply This Effect` | 0.3 | 30% 概率施加 |
| Component：`Tags This Effect Has (Asset Tags)` | `Effect.Debuff.Poison` | 供净化识别 |
| Component：`Grant Tags to Target Actor` | `State.Poisoned` | 供状态 UI 查询 |
| Gameplay Cues | `GameplayCue.Poison` | 毒雾表现 |

另创建 `GE_Cleanse`（Instant），添加 `Remove Other Effects`，查询 `Effect.Debuff`。净化成功后活动中毒 GE 移除，中毒 Tag 与毒雾表现应相应结束。

**验证**：多次命中统计触发率、观察活动 GE 层数与每跳伤害、在不同层数时施加净化。30% 是单次概率，短时间少量实验不会严格等于 30%。

## 8.3 眩晕、净化和免疫的完整交互

**需求**：被眩晕 3 秒无法施放普通攻击和魔法；净化能解除已存在眩晕；开启霸体 5 秒后，不能再被施加新眩晕。

| GE | 模块/组件 | 配置 |
|---|---|---|
| `GE_Stun` | Duration | Has Duration=3 秒 |
| `GE_Stun` | `Grant Tags to Target Actor` | `State.Stunned` |
| `GE_Stun` | `Block Abilities with Tags` | `Ability.Attack`、`Ability.Magic` |
| `GE_Stun` | `Tags This Effect Has (Asset Tags)` | `Effect.Control.Stun` |
| `GE_CleanseCC` | `Remove Other Effects` | 查询 `Effect.Control` |
| `GE_CCImmunity` | Duration | Has Duration=5 秒 |
| `GE_CCImmunity` | `Immunity to Other Effects` | 查询 `Effect.Control` |

运行验证：先施加眩晕，尝试激活 GA；然后施加净化观察解除；再施加免疫，重新尝试应用眩晕，观察 GE 是否被拒绝。若要让眩晕“打断正在施放的魔法”，还需补充 GA 的取消逻辑；不能仅依靠 Block。

## 8.4 达到最大层数后爆炸

**需求**：灼烧最多 5 层；达到上限后再命中一次触发爆炸，之后可按设计清除燃烧层数。

**GE：`GE_Burning`**：Has Duration=10、Period=1、Health Add -5、Aggregate by Target、Stack Limit=5；Overflow Effects 配置 `GE_ExplosionDamage`；根据需要配置 Deny Overflow Application / Clear Stack on Overflow。

**GE：`GE_ExplosionDamage`**：Instant；通过 Modifier 或执行计算扣血；添加 `GameplayCue.Explosion` 展示爆炸特效。

**验证**：统计第 1～5 次叠层、第 6 次溢出，检查爆炸触发次数以及燃烧层数是否按溢出策略保留或清除。这个设计和“达到 5 层立刻爆炸”不同。

## 8.5 装备赋予技能并提供属性

**需求**：装备法杖时攻击力 +20，获得火球术；卸下时失去加成与技能。

**GE：`GE_StaffEquipped`**：Infinite、Period=0、Attack Add +20；添加 `Grant Gameplay Abilities`，授予 `GA_Fireball`；可选择添加 `Tags This Effect Has (Asset Tags)` → `Effect.Equipment.Staff` 以便分类。

装备逻辑应用 GE 后**保存返回的 `FActiveGameplayEffectHandle`**；卸下时用该句柄移除，避免误删同类但来自其他装备或效果的 GE。技能撤销行为取决于该组件的移除策略及其他来源授予情况。

---

# 9. C++ 使用：构造、应用、查询与移除

本章以一个通用的 C++ 使用接口为例。示例默认 `SourceASC`、`TargetASC` 已经初始化并且非空，并且网络游戏中的权威操作由服务端处理。

## 9.1 模块依赖与头文件

在项目 `.Build.cs` 的模块依赖中包含 `GameplayAbilities`、`GameplayTags`、`GameplayTasks`。常用头文件：

```cpp
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
```

## 9.2 创建 Spec 并应用到目标

```cpp
FActiveGameplayEffectHandle ApplyEffect(
    UAbilitySystemComponent* SourceASC,
    UAbilitySystemComponent* TargetASC,
    TSubclassOf<UGameplayEffect> GEClass,
    float Level)
{
    if (!SourceASC || !TargetASC || !GEClass)
    {
        return FActiveGameplayEffectHandle();
    }

    const FGameplayEffectContextHandle Context =
        SourceASC->MakeEffectContext();

    FGameplayEffectSpecHandle Spec =
        SourceASC->MakeOutgoingSpec(GEClass, Level, Context);

    if (!Spec.IsValid())
    {
        return FActiveGameplayEffectHandle();
    }

    return SourceASC->ApplyGameplayEffectSpecToTarget(
        *Spec.Data.Get(), TargetASC);
}
```

`Level` 用于相关 Scalable Float 等随等级变化的求值；并不必然等于角色等级。对于 Instant GE，返回句柄通常不对应一个可供移除的长期活动实例；不要把它保存后期待可以撤销已经发生的 Instant 伤害。

## 9.3 Set by Caller 的正确调用顺序

```cpp
FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(
    DamageGEClass, 1.f, SourceASC->MakeEffectContext());

if (Spec.IsValid() && TargetASC)
{
    const FGameplayTag Key =
        FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));

    Spec.Data->SetSetByCallerMagnitude(Key, -150.f);
    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}
```

核心顺序为 **创建 Spec → 设置本次动态参数 → 应用 Spec**。不要试图直接修改 `UGameplayEffect` 资产的类默认配置，以表达单次攻击的动态数值。

## 9.4 保存活动句柄并移除 GE

```cpp
// 装备时：保存句柄
FActiveGameplayEffectHandle EquippedHandle = ApplyEffect(
    SourceASC, TargetASC, StaffGEClass, 1.f);

// 卸下时：在目标 ASC 上移除该活动 GE
if (TargetASC && EquippedHandle.IsValid())
{
    TargetASC->RemoveActiveGameplayEffect(EquippedHandle);
}
```

同一种 GE 可能存在多个活动实例或堆叠组，因此按句柄精确移除通常比按类盲目移除更安全。若是由 GE Component、到期或净化完成移除，句柄可能已经失效，代码应容忍此情况。

## 9.5 将 GE 应用给自己

```cpp
if (ASC && BuffGEClass)
{
    ASC->ApplyGameplayEffectToSelf(
        BuffGEClass->GetDefaultObject<UGameplayEffect>(),
        1.f,
        ASC->MakeEffectContext());
}
```

这种方式适合无需额外修改 Spec 的简单 Buff。需要 Set by Caller 时，改用 `MakeOutgoingSpec`，先写入动态值，再应用。

## 9.6 计算类与执行类的边界

| 需求 | 常见实现方式 |
|---|---|
| 单一 Modifier 数值来自自定义公式 | MMC：`UGameplayModMagnitudeCalculation` |
| 执行时综合处理多个属性并输出更复杂结果 | ExecCalc：`UGameplayEffectExecutionCalculation` |
| 调用方已算好具体伤害数值 | Set by Caller |
| 按 Attribute 的线性公式计算增益 | Attribute Based |

MMC 与 ExecCalc 解决不同层级的问题。不要因为 MMC 能执行 C++ 逻辑，就在 MMC 内直接调用游戏逻辑的伤害、生成 Actor 或修改多个 Attribute。

## 9.7 网络与权限

持续 GE、堆叠、属性和 Gameplay Tag 的复制依赖 GAS 的网络配置、ASC 所在 Actor、Replication Mode 与权限设计。**游戏结果由服务端权威确定**；Gameplay Cue 更适合视听反馈，不应代替服务器伤害判定。客户端预测也不是所有 GE 类型都能以同样方式预测执行。

---

# 10. 调试与验证方法

学习 GAS 不能只看编辑器中配置的数值，还要检查实际 ASC 状态。

## 10.1 每种 GE 优先观察什么

| 效果类型 | 重点检查 |
|---|---|
| Instant 伤害/治疗 | Attribute Base/Current，执行前后数值 |
| 有限持续 Buff | 活动 GE、剩余时间、Current、到期后恢复 |
| Periodic DoT/HoT | 第一次执行时机、周期次数、每次变化量 |
| 标签组件 | ASC 拥有的 Tag、来源贡献与移除时的变化 |
| Block | GA 是否有对应 Ability Tag、激活是否被阻止 |
| Immunity | 目标是否新增活动 GE、免疫 Query 是否匹配 |
| 净化 | 被查询并移除的活动 GE 范围 |
| Stacking | Stack Count、Duration、Period、Overflow 触发次数 |
| Gameplay Cue | Tag 与 Notify 是否匹配、激活/结束事件 |

可以使用引擎 GAS 调试 HUD（常见命令为 `showdebug abilitysystem`）、ASC 属性/活动效果调试输出、Gameplay Debugger 或自定义日志。命令可用性和界面会随版本与运行环境发生变化。

## 10.2 高发问题

**GE 应用成功但属性没变**：检查 Attribute 是否选对、Op/Magnitude、GE 是否受 Ongoing 抑制、是否为错误的 Source/Target ASC、是否被 Override 或其他 Modifier 影响。

**中毒第一次扣血时机不符**：检查 `Period` 和 `Execute Periodic Effect on Application`；再检查时间边界、堆叠时重置周期和抑制恢复策略。

**Set by Caller 得不到伤害数值**：核对 GE 配置的 Data Tag 与代码写入的 Tag 是否完全一致，是否在 Apply 前写入 Spec。

**叠层没有增长**：核对 Stacking Type、GE 定义是否相同、Source ASC 归属、Stack Limit、Overflow 策略和概率施加条件。

**免疫没有生效**：核对查询匹配的是新施加 GE 的 Asset Tags 等正确条件，而不是角色被授予的 `State.*`；另检查免疫 GE 是否仍处于活动生效状态。

**Cue 不出现**：检查 `GameplayCue.*` 标签是否注册、Notify 是否被扫描加载、事件类型是否正确、客户端是否收到相关 Cue 事件。

## 10.3 技术准确性检查清单

发布 GE 技术材料前，建议在目标 UE 版本中重点核对：

1. Instant、持续、周期 GE 对 Base / Current 的具体影响。
2. 多个 Add/Multiply/Divide/Override 的聚合顺序与规则。
3. Attribute Based 的 Source/Target、Snapshot 和非 Snapshot 更新时机。
4. Modifier Tags 对不同 GE 执行类型的适用差异。
5. 周期首次执行、到期边界、抑制恢复的执行次数。
6. 堆叠后的 Duration/Period 刷新、Overflow 与到期处理。
7. Gameplay Cue 的 OnActive、WhileActive、Executed、Removed 是否如预期发生。
8. 网络模式下应用权限、客户端复制和 Cue 表现的一致性。

---

# 11. 需求速查索引

| 开发需求 | 主要配置位置 |
|---|---|
| 立即扣血/回血 | Instant + Health Modifier |
| 临时增加攻击力 | Has Duration + Period=0 + Attack Modifier |
| 装备永久加成 | Infinite + Modifier，卸下时移除 |
| 每秒中毒 | Has Duration + Period>0 + Health Modifier |
| 立即跳一次中毒伤害 | Execute Periodic Effect on Application |
| 智力决定 Buff 强度 | Modifier Magnitude → Attribute Based |
| 复杂自定义倍率 | Modifier Magnitude → MMC |
| 同一 GE 支持多种动态伤害 | Modifier Magnitude → Set by Caller |
| 给 GE 分类 | `Tags This Effect Has (Asset Tags)` |
| 让角色拥有状态 Tag | `Grant Tags to Target Actor` |
| 阻止某类 GA 激活 | `Block Abilities with Tags` |
| 阻止新控制 GE | `Immunity to Other Effects` |
| 移除已有 Debuff | `Remove Other Effects` |
| 按状态决定 GE 是否生效 | `Require Tags to Apply/Continue This Effect` |
| 30% 概率施加 GE | `Chance To Apply This Effect` |
| Buff 结束后触发另一个 GE | `Apply Additional Effects` |
| 装备时授予 GA | `Grant Gameplay Abilities` |
| 自定义复杂施加条件 | `Custom Can Apply This Effect` |
| Buff 说明文本 | `UI Data (Text Only)` |
| 伤害火花与持续特效 | Gameplay Cues + Gameplay Cue Notify |
| 重复叠层 | Stacking Type + Stack Limit |
| 每次叠层刷新时间 | Stack Duration Refresh Policy |
| 堆叠到期逐层下降 | Stack Expiration Policy |
| 满层后再次命中引爆 | Overflow Effects |

---

# 12. 参考资料与版本说明

- Epic Games：Gameplay Effects for the Gameplay Ability System  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine
- Epic Games：Gameplay Ability System  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine
- Epic Games：UGameplayEffect C++ API  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/UGameplayEffect
- Epic Games：UGameplayEffectComponent C++ API  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayAbilities/UGameplayEffectComponent

这份文档围绕 UE 5.6/5.7 常见 GE 配置组织，优先解释系统机制和开发用途，随后给出可重现的配置与代码。对于引擎源码版本差异、复杂聚合和服务器预测等细节，应以所使用的具体引擎版本、引擎源码和项目内的运行验证为最终依据。
