# UE GAS 教程 02：GameplayEffect —— GAS 如何修改属性

## 1. 这一节要解决什么问题

上一节已经定义了角色属性：

```text
Health
MaxHealth
Attack
```

这些属性保存在 `AttributeSet` 中，由 ASC 管理和同步。

但是，只定义属性还不够。

游戏中还需要处理：

- 受到伤害
- 恢复生命
- 临时增加攻击力
- 装备提供永久加成
- 持续中毒
- Buff 叠层
- 根据技能等级计算伤害
- 满足特定状态后才能施加效果
- 从攻击者向目标施加效果

这些需求都有一个共同点：

> 它们都在描述“以什么规则修改属性或状态”。

在 GAS 中，这份规则由 `GameplayEffect` 表示，通常简称 GE。

这一节结合 `GASLesson2` 的代码和资产，解决下面的问题：

1. GameplayEffect 是什么？
2. GE、ASC 和 AttributeSet 是什么关系？
3. Instant、Duration、Infinite 有什么区别？
4. Modifier 的 Attribute、Op、Magnitude 分别是什么？
5. GE Class、GameplayEffectSpec 和 ActiveGameplayEffect 有什么区别？
6. 固定数值、SetByCaller 和 Scalable Float 怎么使用？
7. Period、Stack、GameplayTag 和应用条件怎么配置？
8. GE 如何对自己和其他目标生效？
9. 为什么客户端不能直接修改属性？
10. 本课每个 GE 分别适合什么应用场合？

本节核心：

> AttributeSet 定义“角色拥有哪些属性”，GameplayEffect 定义“这些属性和状态怎样变化”，ASC 负责执行和管理这些变化。

---

# 2. GameplayEffect 是什么

GameplayEffect 可以理解为：

> 一份属性和状态修改规则。

例如，“造成 20 点伤害”可以描述为：

```text
目标属性：Health
运算方式：Add
数值：-20
生效方式：Instant
```

“攻击力提高 50 点，持续 5 秒”可以描述为：

```text
目标属性：Attack
运算方式：Add
数值：50
生效方式：Has Duration
持续时间：5 秒
```

GameplayEffect 本身一般不负责：

- 玩家什么时候按键
- 技能是否命中
- 射线检测到了谁
- 动画什么时候播放
- 角色是否死亡

这些属于角色、技能、任务或其他业务系统。

GameplayEffect 主要负责：

```text
修改什么
怎样修改
修改多少
持续多久
是否周期执行
能否叠层
应用时需要什么条件
激活期间授予什么状态
```

---

# 3. GE、ASC 和 AttributeSet 的关系

本课中的关系是：

```text
AGASLesson2Character
│
├── UAbilitySystemComponent
│      负责创建、应用、管理和复制 GE
│
└── UGASLesson2AttributeSet
       保存 Health、MaxHealth、Attack
```

Character 构造函数创建 ASC 和 AttributeSet：

```cpp
AbilitySystemComponent =
    CreateDefaultSubobject<UAbilitySystemComponent>(
        TEXT("AbilitySystemComponent"));

AttributeSet =
    CreateDefaultSubobject<UGASLesson2AttributeSet>(
        TEXT("AttributeSet"));
```

进入游戏后初始化 ActorInfo：

```cpp
AbilitySystemComponent->InitAbilityActorInfo(this, this);
```

这里两个 `this` 分别是：

```text
OwnerActor  = Character
AvatarActor = Character
```

ASC 和 AttributeSet 都是 Character 的默认子对象。ASC 初始化时会发现并注册同一 Owner 下的 AttributeSet。

因此不需要让 GE 自己查找 Character 中的 `Health` 变量。

GE 保存的是 `FGameplayAttribute`：

```text
UGASLesson2AttributeSet::Health
```

ASC 应用 GE 时，会在自己管理的 AttributeSet 中找到这个属性并执行修改。

完整关系：

```text
GameplayEffect
    描述修改规则
        ↓
AbilitySystemComponent
    执行和管理规则
        ↓
AttributeSet
    保存最终属性数据
```

---

# 4. GameplayEffect 资产不是正在运行的效果

项目中的：

```text
GE_Damage
GE_Heal
GE_Poison
```

都是 GameplayEffect 蓝图类。

它们是配置模板，不代表某个角色身上正在运行的具体效果。

需要区分四个概念：

```text
GameplayEffect Class
        ↓ 创建
GameplayEffectSpec
        ↓ 应用
ActiveGameplayEffect
        ↓ 标识
FActiveGameplayEffectHandle
```

---

# 5. GameplayEffect Class

GameplayEffect Class 是资产中保存的规则模板。

例如 `GE_Damage` 保存：

```text
Duration Policy = Instant
Health Add -20
```

Character 中用下面的类型引用它：

```cpp
UPROPERTY(EditDefaultsOnly)
TSubclassOf<UGameplayEffect> DamageEffect;
```

所有角色可以共享同一个 GE Class，因为这个类只保存默认配置。

不应该为了每一次伤害去修改 GE Class。

---

# 6. GameplayEffectSpec

Spec 表示：

> 这一次准备应用的 GameplayEffect 数据。

本课通过 ASC 创建 Spec：

```cpp
FGameplayEffectContextHandle Context =
    AbilitySystemComponent->MakeEffectContext();

Context.AddSourceObject(this);

FGameplayEffectSpecHandle SpecHandle =
    AbilitySystemComponent->MakeOutgoingSpec(
        EffectClass,
        Level,
        Context);
```

Spec 在 GE Class 默认配置的基础上，还可以携带本次执行的数据：

- GameplayEffect Level
- SetByCaller 数值
- 来源 ASC
- EffectContext
- SourceObject
- 动态标签
- 捕获的属性信息

同一个 GE Class 可以创建很多不同的 Spec。

例如都使用 `GE_DynamicDamage`：

```text
第一次 Spec：Data.Damage = -20
第二次 Spec：Data.Damage = -35
第三次 Spec：Data.Damage = -80
```

---

# 7. ActiveGameplayEffect 和 Handle

Duration 和 Infinite GE 应用成功后，会成为 ASC 中的 ActiveGameplayEffect。

它保存：

- 剩余时间
- 周期时间
- 当前层数
- Modifier
- Granted Tags
- 来源信息

应用 GE 后会返回：

```cpp
FActiveGameplayEffectHandle
```

Handle 可以理解为：

> ASC 中某个 ActiveGameplayEffect 的身份标识。

本课装备案例保存了 Handle：

```cpp
WeaponEffectHandle = ApplyEffectToSelf(WeaponEffect);
```

卸下装备时精确移除：

```cpp
AbilitySystemComponent->RemoveActiveGameplayEffect(
    WeaponEffectHandle);
```

Instant GE 执行完就结束，不会作为持续效果长期保存在 ASC 中。

---

# 8. 一个 GE 是怎样应用的

本课把常用流程封装为：

```cpp
FActiveGameplayEffectHandle ApplyEffectToSelf(
    TSubclassOf<UGameplayEffect> EffectClass,
    float Level = 1.f);
```

核心步骤：

```text
检查 ASC 和 GE Class
        ↓
ASC 创建 EffectContext
        ↓
ASC 根据 GE Class 和 Level 创建 Spec
        ↓
ApplyGameplayEffectSpecToSelf
        ↓
ASC 执行 Modifier
        ↓
AttributeSet 属性变化
```

最值得注意的是：

```cpp
ApplyEffectToSelf(DamageEffect);
ApplyEffectToSelf(HealEffect);
ApplyEffectToSelf(AttackBuffEffect);
ApplyEffectToSelf(PoisonEffect);
```

调用方式几乎一样。

它们表现不同，不是因为 Character 为每种 GE 写了完全不同的属性修改代码，而是因为 GE 资产配置不同。

这就是 GameplayEffect 数据驱动的价值。

---

# 9. Duration Policy

GE 最先需要理解的属性是：

```text
Duration Policy
```

它决定 GE 的生命周期。

一共有三种主要类型：

```text
Instant
Has Duration
Infinite
```

---

## 9.1 Instant

立即执行，执行完就结束。

常见用途：

- 直接伤害
- 立即治疗
- 一次性消耗生命或法力
- 立即增加经验
- 周期 GE 的每次结算

本课案例：

```text
GE_Damage
GE_Heal
GE_DynamicDamage
GE_LevelDamage
```

Instant Modifier 通常会实际改变属性的 BaseValue。

效果执行结束后，不会因为移除 Active GE 而恢复。

---

## 9.2 Has Duration

GE 激活一段时间，到期后自动移除。

需要配置：

```text
Duration Magnitude
```

常见用途：

- 临时 Buff
- 临时 Debuff
- 中毒
- 燃烧
- 减速
- 短时间无敌

本课案例：

```text
GE_AttackBuff
GE_Poison
GE_StackAttack
GE_AttackMultiply
GE_BurningRequiresOil
```

---

## 9.3 Infinite

GE 没有自动结束时间，需要主动移除。

常见用途：

- 装备属性
- 被动技能
- 常驻光环
- 姿态
- 永久状态
- 存档加载后的长期 Buff

本课案例：

```text
GE_Weapon
GE_Oiled
```

Infinite 不代表永远不能移除，只代表没有倒计时。

---

# 10. Modifier 是什么

Modifier 描述一条属性修改。

核心字段：

```text
Attribute
Modifier Op
Modifier Magnitude
```

例如：

```text
Attribute          = Health
Modifier Op        = Add Base
Modifier Magnitude = -20
```

表达：

```text
Health = Health + (-20)
```

一个 GE 可以没有 Modifier，也可以有多个 Modifier。

例如一个药剂可以同时：

```text
Health +20
Mana +10
MoveSpeed -50
```

本课为了让每个案例目标明确，大多数 GE 只配置一条 Modifier。

`GE_Oiled` 则没有 Modifier，只负责授予标签。

---

# 11. Modifier Op

UE 5.6 中常见的 Modifier Op 包括：

```text
Add Base
Multiply Additive
Divide Additive
Multiply Compound
Add Final
Override
```

GAS 的聚合顺序不是“按照 GE 应用顺序逐条随意计算”，而是按运算类型聚合：

```text
((BaseValue + AddBase)
    * MultiplyAdditive
    / DivideAdditive
    * MultiplyCompound)
    + AddFinal
```

本课覆盖：

```text
Add Base         → 大多数 GE
Multiply Additive → GE_AttackMultiply
```

`Multiply Additive` 的多个倍率会先按 GAS 规则聚合。

例如两个 `1.5` 的 Additive Multiplier，结果是 `2.0` 倍，不是 `2.25` 倍。

如果希望 `1.5 × 1.5 = 2.25`，需要理解 `Multiply Compound`，不能只看界面上的“Multiply”字样。

---

# 12. Modifier Magnitude

Magnitude 表示 Modifier 的数值从哪里来。

本课覆盖三种常见形式：

```text
固定 Scalable Float
SetByCaller
Scalable Float + Curve Table + GE Level
```

---

## 12.1 固定 Scalable Float

例如：

```text
Health -20
Health +20
Attack +50
Attack ×1.5
```

虽然编辑器类型叫 Scalable Float，但不绑定曲线时也可以作为固定值使用。

适合：

- 固定数值 Buff
- 固定治疗
- 固定装备加成
- 教学和原型验证

---

## 12.2 SetByCaller

GE 只声明需要哪个数据标签：

```text
Data.Damage
```

真正数值由创建 Spec 的代码传入：

```cpp
SpecHandle.Data->SetSetByCallerMagnitude(
    DamageTag,
    DamageMagnitude);
```

适合：

- 伤害由武器和技能提前计算
- 治疗量由外部逻辑决定
- 消耗量每次不同
- 同一个 GE 需要携带不同数值

SetByCaller 数据属于 Spec，不属于共享的 GE Class。

---

## 12.3 Scalable Float 和 Curve Table

本课的 `CT_LevelDamage` 包含：

```text
RowName = Damage

Level 1 → 10
Level 2 → 20
Level 3 → 40
```

`GE_LevelDamage` 使用：

```text
Value = -1
Curve = CT_LevelDamage.Damage
```

Scalable Float 的基本计算可以理解为：

```text
最终值 = Value × Curve[Level]
```

因此：

```text
Level 1：-1 × 10 = -10
Level 2：-1 × 20 = -20
Level 3：-1 × 40 = -40
```

适合：

- 技能等级成长
- 不同等级装备属性
- 随角色等级变化的 Buff
- 数据策划通过曲线调整数值

---

# 13. Period

Duration GE 可以配置：

```text
Period
Execute Periodic Effect On Application
```

Period 表示每隔多少秒执行一次 GE。

例如：

```text
Duration = 10
Period = 1
Health Add -5
```

表示在有效期内按一秒间隔执行扣血。

本课周期 GE 都启用了：

```text
Execute Periodic Effect On Application = true
```

所以应用成功时会立即执行一次，然后继续按周期执行。

周期 GE 和普通持续 Modifier 的区别：

```text
普通持续 Buff
    Modifier 在激活期间参与 CurrentValue 计算
    GE 移除后加成消失

周期伤害
    每个周期真正执行一次属性修改
    已经扣除的生命不会随 GE 移除而恢复
```

常见用途：

- DOT：持续伤害
- HOT：持续治疗
- 周期回蓝
- 光环的周期结算
- 环境区域伤害

---

# 14. Granted Tags

UE 5.6 中，GE 通过 GameplayEffect Component：

```text
Grant Tags to Target Actor
```

向目标 ASC 授予标签。

标签只在 GE 激活期间存在。

例如 `GE_Oiled` 授予：

```text
Demo.Effect
Demo.Effect.Oiled
State.Oiled
```

当 `GE_Oiled` 被移除后，`State.Oiled` 也会消失。

Granted Tags 常见用途：

- `State.Stunned`
- `State.Poisoned`
- `State.Burning`
- `State.Invincible`
- `Buff.AttackUp`
- `Debuff.Slow`

需要区分：

```text
Asset Tags
    标签属于 GE，用于描述“这是什么 Effect”

Granted Tags
    标签授予目标，用于描述“角色现在是什么状态”
```

本课主要演示 Granted Tags。

---

# 15. Application Tag Requirements

UE 5.6 中通过 GameplayEffect Component：

```text
Require Tags to Apply/Continue This Effect
```

配置应用条件。

`GE_BurningRequiresOil` 要求目标拥有：

```text
State.Oiled
```

应用过程：

```text
ASC 准备应用 Burning Spec
        ↓
检查目标是否拥有 State.Oiled
        ↓
没有：拒绝应用
拥有：应用成功
```

Application Requirements 只在应用时做通过或失败判断。

常见用途：

- 沾油后才能燃烧
- 潮湿目标才能触发导电
- 只有受伤目标才能被处决
- Boss 免疫普通控制
- 特定姿态下才能获得 Buff
- 目标已有某状态时追加后续效果

---

# 16. Stacking

叠层 GE 常见配置：

```text
Stacking Type
Stack Limit Count
Stack Duration Refresh Policy
Stack Period Reset Policy
Stack Expiration Policy
```

本课 `GE_StackAttack` 使用：

```text
Stacking Type = Aggregate By Target
Stack Limit Count = 5
成功叠层后刷新持续时间
```

连续应用时，不会创建五个完全独立的 Active GE，而是增加同一个目标上的 StackCount。

代码查询层数：

```cpp
const int32 StackCount =
    AbilitySystemComponent->GetCurrentStackCount(Handle);
```

常见用途：

- 中毒层数
- 流血层数
- 连击增伤
- 破甲层数
- 可叠加护盾
- 达到指定层数后触发爆炸

---

# 17. BaseValue 和 CurrentValue

初始 Attack：

```text
BaseValue = 20
CurrentValue = 20
```

应用持续 5 秒的 `Attack +50`：

```text
BaseValue = 20
CurrentValue = 70
```

GE 到期：

```text
BaseValue = 20
CurrentValue = 20
```

应用 Instant 的 `Health -20`：

```text
Health BaseValue：100 → 80
Health CurrentValue：100 → 80
```

因此可以先记住：

```text
Instant
    通常实际修改 BaseValue

Duration / Infinite 的非周期 Modifier
    通常作为临时聚合项影响 CurrentValue

Periodic
    每个周期按一次执行结果修改属性
```

按 `9` 可以同时打印 BaseValue 和 CurrentValue，观察这些差别。

---

# 18. AttributeSet 仍然负责属性规则

GameplayEffect 负责提出修改：

```text
Health -20
Health +20
Health -40
```

AttributeSet 负责保证 Health 合法。

修改前：

```cpp
void UGASLesson2AttributeSet::PreAttributeChange(
    const FGameplayAttribute& Attribute,
    float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(
            NewValue,
            0.f,
            GetMaxHealth());
    }
}
```

Instant 和 Periodic GE 执行后再次收口：

```cpp
void UGASLesson2AttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(
            GetHealth(),
            0.f,
            GetMaxHealth()));
    }
}
```

所以：

```text
GE 决定“修改多少”
AttributeSet 决定“属性最终是否合法”
```

治疗不会超过 MaxHealth，伤害也不会让 Health 低于 0。

---

# 19. 案例总览

| 按键 | Effect | 主要 GE 属性 | 结果 |
|---|---|---|---|
| `1` | `GE_Damage` | Instant、Add Base | Health -20 |
| `2` | `GE_Heal` | Instant、正数 Magnitude | Health +20 |
| `3` | `GE_AttackBuff` | Has Duration | Attack +50，持续 5 秒 |
| `4` | `GE_Weapon` | Infinite | Attack +20，直到主动移除 |
| `5` | 移除 Weapon | Active GE Handle | 精确移除装备效果 |
| `6` | `GE_DynamicDamage` | SetByCaller | 运行时传入 -30 |
| `7` | `GE_Poison` | Duration + Period | 每秒 Health -5，持续 10 秒 |
| `8` | `GE_StackAttack` | Stacking | 每层 Attack +10，最多 5 层 |
| `9` | 无 | 属性查询 | 打印 BaseValue 和 CurrentValue |
| `0` | 无 | Effect Query + Tags | 清除持续中的演示 GE |
| `F1` | `GE_AttackMultiply` | Multiply Additive | Attack ×1.5，持续 5 秒 |
| `F2` | `GE_LevelDamage` | Curve + Spec Level | 依次造成 10、20、40 伤害 |
| `F3` | `GE_Oiled` | Infinite + Granted Tags | 获得 State.Oiled |
| `F4` | `GE_BurningRequiresOil` | Application Requirement | 有 State.Oiled 才能燃烧 |
| `F5` | `GE_Damage` | Apply Spec To Target | 对瞄准的其他角色造成伤害 |

下面逐个分析这些案例。

---

# 20. GE_Damage：Instant 伤害

## 资产配置

```text
Duration Policy = Instant

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude = -20
```

## 使用方式

按 `1`：

```cpp
ApplyEffectToSelf(DamageEffect);
```

## 执行结果

```text
Health 100 → 80
```

GE 执行结束，不会作为持续效果留在 ASC 中。

按 `0` 不能撤销已经造成的伤害。

## 案例重点

- `Duration Policy = Instant`
- Modifier 的基本组成
- 负数 Add 表示减少属性
- Instant 修改执行后不会自动恢复

## 常见应用场合

- 普通攻击伤害
- 固定环境伤害
- 掉落伤害
- 简单技能伤害
- 测试伤害链路

---

# 21. GE_Heal：Instant 治疗

## 资产配置

```text
Duration Policy = Instant

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude = 20
```

## 使用方式

按 `2`：

```cpp
ApplyEffectToSelf(HealEffect);
```

## 案例重点

`GE_Damage` 和 `GE_Heal` 结构完全相同，主要区别只是 Magnitude 的正负号。

GAS 不会自动理解“伤害”和“治疗”这两个业务概念。

它看到的是：

```text
Health Add -20
Health Add +20
```

Health 最终由 AttributeSet 限制在：

```text
0 到 MaxHealth
```

## 常见应用场合

- 药品立即回血
- 治疗技能
- 拾取生命包
- 复活后的生命恢复
- Instant 资源补充

---

# 22. GE_AttackBuff：有持续时间的加法 Buff

## 资产配置

```text
Duration Policy = Has Duration
Duration Magnitude = 5

Modifiers[0]
    Attribute = Attack
    Modifier Op = Add Base
    Magnitude = 50

Granted Tags
    Demo.Effect
    Demo.Effect.AttackBuff
```

## 使用方式

按 `3`：

```cpp
ApplyEffectToSelf(AttackBuffEffect);
```

## 执行结果

初始：

```text
Attack BaseValue = 20
Attack CurrentValue = 20
```

Buff 激活：

```text
Attack BaseValue = 20
Attack CurrentValue = 70
```

五秒后：

```text
Attack BaseValue = 20
Attack CurrentValue = 20
```

## 案例重点

- `Has Duration`
- `Duration Magnitude`
- 持续 Modifier 参与 CurrentValue 聚合
- GE 到期后 Modifier 和 Granted Tags 自动移除

## 常见应用场合

- 临时攻击 Buff
- 临时防御 Buff
- 短时间移动速度提升
- 技能增伤窗口
- 增益药剂

---

# 23. GE_Weapon：Infinite 和 Handle

## 资产配置

```text
Duration Policy = Infinite

Modifiers[0]
    Attribute = Attack
    Modifier Op = Add Base
    Magnitude = 20

Granted Tags
    Demo.Effect
    Demo.Effect.Weapon
```

## 使用方式

按 `4` 装备：

```cpp
WeaponEffectHandle = ApplyEffectToSelf(WeaponEffect);
```

按 `5` 卸下：

```cpp
AbilitySystemComponent->RemoveActiveGameplayEffect(
    WeaponEffectHandle);
```

## 案例重点

- Infinite 没有自动结束时间
- 应用后保存 ActiveGameplayEffectHandle
- 使用 Handle 精确移除某次效果
- GE Class 和 Active GE 不是同一个概念

## 常见应用场合

- 武器属性
- 防具属性
- 被动技能
- 天赋常驻加成
- 持续姿态
- 常驻光环

---

# 24. GE_DynamicDamage：SetByCaller

## 资产配置

```text
Duration Policy = Instant

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude Calculation Type = SetByCaller
    Data Tag = Data.Damage
```

GE 资产里没有固定伤害值。

## 使用方式

按 `6` 后，代码先创建 Spec：

```cpp
FGameplayEffectSpecHandle SpecHandle =
    AbilitySystemComponent->MakeOutgoingSpec(
        DynamicDamageEffect,
        1.f,
        Context);
```

再写入：

```cpp
SpecHandle.Data->SetSetByCallerMagnitude(
    DamageTag,
    -30.f);
```

最后应用 Spec。

## 案例重点

- GE Class 只声明需要 `Data.Damage`
- 具体数值保存在本次 Spec 中
- 同一个 GE 可以处理很多不同数值
- SetByCaller 标签必须提前注册

## 常见应用场合

- 武器根据面板计算最终伤害
- 暴击后传入不同伤害
- 技能根据蓄力时间计算数值
- 消耗当前一定数量的资源
- 服务器结算后把结果交给 GE

---

# 25. GE_Poison：周期伤害

## 资产配置

```text
Duration Policy = Has Duration
Duration Magnitude = 10
Period = 1
Execute Periodic Effect On Application = true

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude = -5

Granted Tags
    Demo.Effect
    Demo.Effect.Poison
```

## 使用方式

按 `7`：

```cpp
ApplyEffectToSelf(PoisonEffect);
```

## 执行特点

应用成功时立即结算一次，之后在有效期内按一秒周期继续结算。

每次周期执行后，AttributeSet 的 `PostGameplayEffectExecute` 都会限制 Health。

清除 Poison 只会停止未来的周期，不会恢复之前已经扣除的生命。

## 常见应用场合

- 中毒
- 燃烧
- 流血
- 持续治疗
- 周期回蓝
- 区域持续伤害

---

# 26. GE_StackAttack：Buff 叠层

## 资产配置

```text
Duration Policy = Has Duration
Duration Magnitude = 10

Modifiers[0]
    Attribute = Attack
    Modifier Op = Add Base
    Magnitude = 10

Stacking Type = Aggregate By Target
Stack Limit Count = 5
Stack Duration Refresh Policy = Refresh On Successful Application

Granted Tags
    Demo.Effect
    Demo.Effect.Stack
```

## 使用方式

连续按 `8`。

每次应用后查询：

```cpp
const int32 StackCount =
    AbilitySystemComponent->GetCurrentStackCount(Handle);
```

## 执行结果

初始 Attack 为 20：

```text
1 层：30
2 层：40
3 层：50
4 层：60
5 层：70
```

达到 5 层后不再提高层数。

成功增加层数时，持续时间重新从 10 秒计算。

## 常见应用场合

- 连击增伤
- 中毒层数
- 流血层数
- 破甲层数
- 叠层护盾
- 达到最大层触发后续效果

---

# 27. GE_AttackMultiply：乘法 Modifier

## 资产配置

```text
Duration Policy = Has Duration
Duration Magnitude = 5

Modifiers[0]
    Attribute = Attack
    Modifier Op = Multiply Additive
    Magnitude = 1.5

Granted Tags
    Demo.Effect
    Demo.Effect.AttackMultiply
```

## 使用方式

清除其他攻击 Buff 后按 `F1`：

```cpp
ApplyEffectToSelf(AttackMultiplyEffect);
```

初始 Attack 为 20：

```text
Attack CurrentValue = 20 × 1.5 = 30
```

五秒后恢复。

## 为什么要和 GE_AttackBuff 对比

`GE_AttackBuff`：

```text
Attack +50
```

`GE_AttackMultiply`：

```text
Attack ×1.5
```

两者都提高 Attack，但它们处于不同的聚合阶段。

## 常见应用场合

- 百分比增伤
- 百分比减速
- 难度倍率
- 属性倍率 Buff
- 装备词条中的百分比加成

---

# 28. GE_LevelDamage：根据 Spec Level 读取曲线

## 资产配置

```text
Duration Policy = Instant

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude = Scalable Float
    Value = -1
    Curve Table = CT_LevelDamage
    Row = Damage
```

Curve Table：

```text
Level 1 → 10
Level 2 → 20
Level 3 → 40
```

## 使用方式

每按一次 `F2`，服务器依次使用：

```text
Level 1
Level 2
Level 3
然后回到 Level 1
```

代码把 Level 传给 Spec：

```cpp
ApplyEffectToSelf(
    LevelDamageEffect,
    static_cast<float>(AppliedLevel));
```

## 执行结果

```text
Level 1：Health -10
Level 2：Health -20
Level 3：Health -40
```

## 案例重点

- GE Level 属于 Spec
- Scalable Float 可以根据 Level 查询曲线
- 数据成长不需要在 Character 中写大量分支
- 曲线和 GE 资产负责数据，C++ 只负责传入 Level

## 常见应用场合

- 技能等级伤害
- 装备等级属性
- 怪物等级成长
- Buff 随等级增强
- 策划可调的数值成长表

---

# 29. GE_Oiled：没有 Modifier 的状态 GE

## 资产配置

```text
Duration Policy = Infinite
Modifiers = 空

Granted Tags
    Demo.Effect
    Demo.Effect.Oiled
    State.Oiled
```

## 使用方式

按 `F3`：

```cpp
ApplyEffectToSelf(OiledEffect);
```

代码会先检查是否已经拥有 `State.Oiled`，避免重复应用演示状态。

## 案例重点

GameplayEffect 不一定要修改 Attribute。

它也可以只负责管理状态：

```text
角色现在处于 Oiled 状态
```

这个状态可以被其他 GE、GameplayAbility 或业务代码查询。

## 常见应用场合

- 沉默
- 潮湿
- 沾油
- 隐身
- 无敌
- 战斗中
- 可被处决
- 技能连招标记

---

# 30. GE_BurningRequiresOil：满足标签才能应用

## 资产配置

```text
Duration Policy = Has Duration
Duration Magnitude = 5
Period = 1
Execute Periodic Effect On Application = true

Modifiers[0]
    Attribute = Health
    Modifier Op = Add Base
    Magnitude = -3

Application Required Tags
    State.Oiled

Granted Tags
    Demo.Effect
    Demo.Effect.Burning
    State.Burning
```

## 使用方式

先直接按 `F4`：

```text
目标没有 State.Oiled
Burning Spec 被拒绝
```

然后：

```text
按 F3 → 获得 State.Oiled
按 F4 → Burning 应用成功
```

应用成功后，立即执行一次 `Health -3`，然后在有效期内继续周期执行。

## 案例重点

- GE 可以没有 Modifier，只提供前置状态
- 另一个 GE 可以把状态作为应用条件
- 条件判断由 ASC 和 GE Component 处理
- Character 不需要手动写“如果 Oiled 就扣血”的完整业务分支

## 常见应用场合

- 沾油后燃烧
- 潮湿后导电
- 冰冻后碎裂
- 标记目标后引爆
- 低生命目标才能处决
- 特定状态免疫某类效果

---

# 31. F5：把 GE 应用到其他目标

前面的案例大部分调用：

```cpp
ApplyGameplayEffectSpecToSelf(...);
```

`F5` 演示：

```cpp
ApplyGameplayEffectSpecToTarget(...);
```

它没有新增一个特殊伤害 GE，而是复用现有的 `GE_Damage`。

## 执行流程

```text
客户端按 F5
        ↓
Server RPC 请求测试
        ↓
服务器从角色视点执行射线检测
        ↓
检查目标不是自己
        ↓
检查目标距离不超过 1500
        ↓
检查目标实现 IAbilitySystemInterface
        ↓
取得目标 ASC
        ↓
来源 ASC 创建 GE_Damage Spec
        ↓
ApplyGameplayEffectSpecToTarget
        ↓
目标 Health -20
```

核心代码：

```cpp
return AbilitySystemComponent
    ->ApplyGameplayEffectSpecToTarget(
        *SpecHandle.Data.Get(),
        TargetAbilitySystem);
```

这里：

```text
Source ASC = 当前角色的 ASC
Target ASC = 射线命中角色的 ASC
```

## 为什么由服务器重新射线检测

不能让客户端直接告诉服务器：

```text
“请给这个任意目标扣血。”
```

否则被修改的客户端可以伪造目标。

本课让服务器重新确定命中对象，并检查：

- 目标是否有效
- 目标是否为自己
- 目标是否超出距离
- 目标是否拥有 ASC

这是服务器权威的基本思路。

## 常见应用场合

- 普通攻击命中
- 技能命中
- 治疗其他角色
- 给队友添加 Buff
- 给敌人添加 Debuff
- 范围查询后逐个应用 GE

---

# 32. 按 0：通过标签批量移除 GE

持续中的演示 GE 都授予父标签：

```text
Demo.Effect
```

按 `0` 时创建查询：

```cpp
FGameplayEffectQuery Query =
    FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
        DemoEffectTags);
```

然后批量移除：

```cpp
AbilitySystemComponent->RemoveActiveEffects(Query);
```

可以移除：

- AttackBuff
- Weapon
- Poison
- StackAttack
- AttackMultiply
- Oiled
- Burning

不能撤销：

- Damage 已经造成的伤害
- Heal 已经完成的治疗
- DynamicDamage 已经造成的伤害
- LevelDamage 已经造成的伤害
- Poison 和 Burning 已经结算的周期伤害

因为这些已经不是等待移除的临时 Modifier。

## Handle 和 Tag Query 的区别

```text
Handle
    精确移除某个 Active GE

Tag Query
    批量移除符合条件的一类 Active GE
```

常见应用：

- 卸下一件具体装备：Handle
- 净化全部 Debuff：Tag Query
- 死亡时清除临时状态：Tag Query
- 取消某个技能产生的具体 Buff：Handle

---

# 33. 属性变化监听

Character 通过 ASC 监听 Health 和 Attack：

```cpp
AbilitySystemComponent
    ->GetGameplayAttributeValueChangeDelegate(
        UGASLesson2AttributeSet::GetHealthAttribute())
    .AddUObject(
        this,
        &ThisClass::OnHealthChanged);
```

回调得到：

```cpp
Data.OldValue
Data.NewValue
```

因此无论属性来自：

- Instant GE
- Duration GE
- Infinite GE
- Periodic GE
- SetByCaller GE
- Curve Level GE
- 来自其他角色的 GE

监听方式都是统一的。

UI 不需要知道具体是哪一个 GE 修改了 Health，普通血条只关心属性的新值。

---

# 34. 网络同步流程

本课 ASC 启用了复制：

```cpp
AbilitySystemComponent->SetIsReplicated(true);
AbilitySystemComponent->SetReplicationMode(
    EGameplayEffectReplicationMode::Full);
```

AttributeSet 属性也配置了 RepNotify：

```cpp
DOREPLIFETIME_CONDITION_NOTIFY(
    UGASLesson2AttributeSet,
    Health,
    COND_None,
    REPNOTIFY_Always);
```

客户端按键后的流程：

```text
客户端输入
    ↓
RequestTest
    ↓
ServerApplyTestEffect RPC
    ↓
服务器 ExecuteTest
    ↓
服务器 ASC 应用 GE
    ↓
服务器 AttributeSet 属性变化
    ↓
ASC / Attribute Replication
    ↓
客户端收到属性和 Active GE 状态
    ↓
OnRep 和属性变化委托
```

客户端不会直接执行权威属性修改。

`Full` 模式适合本课观察完整 Active GameplayEffect。正式项目会根据玩家角色、AI 和 ASC 的位置选择 Full、Mixed 或 Minimal。

---

# 35. 推荐实验顺序

## 实验一：Instant

```text
按 9：观察 Health = 100
按 1：Health -20
按 2：Health +20
按 9：观察 BaseValue 和 CurrentValue
```

重点：Instant 会留下已经执行的属性结果。

## 实验二：Duration

```text
按 0：清场
按 3：Attack +50
立即按 9
五秒后再按 9
```

重点：观察 Attack 的 BaseValue 和 CurrentValue。

## 实验三：Infinite 和 Handle

```text
按 4：装备 Weapon
按 9：观察 Attack
按 5：按 Handle 移除
按 9：确认恢复
```

## 实验四：SetByCaller

```text
按 6
观察日志中的 Data.Damage = -30
观察 Health 变化
```

## 实验五：Period

```text
按 7
观察应用时立即扣血
继续观察每秒扣血
中途按 0
确认后续周期停止
```

## 实验六：Stack

```text
按 0
连续按 8 五次
观察 StackCount 和 Attack
继续按第六次
确认不超过五层
```

## 实验七：Add 和 Multiply

```text
按 0
按 F1
按 9
确认 Attack 20 → 30

再按 0
按 3
按 F1
按 9
观察 Add 和 Multiply 的聚合结果
```

## 实验八：GE Level

```text
保证 Health 足够
连续按 F2 三次
观察伤害依次为 10、20、40
```

## 实验九：Application Requirement

```text
按 0
直接按 F4：应用失败
按 F3：获得 State.Oiled
按 F4：应用成功
观察 State.Burning 和周期伤害
```

## 实验十：Source 和 Target

使用双客户端 PIE：

```text
客户端 A 瞄准客户端 B
客户端 A 按 F5
客户端 B 的 Health -20
客户端 A 的 Health 不变
```

---

# 36. 常见问题

## 为什么伤害和治疗都用 Add

因为 GAS 只执行数值运算：

```text
负数 Add → 属性减少
正数 Add → 属性增加
```

“伤害”和“治疗”是项目业务含义。

## 为什么 GE 资产里有规则，代码还要创建 Spec

GE Class 是共享模板，Spec 是本次执行的数据。

Level、SetByCaller、来源和 Context 都属于本次 Spec。

## 为什么 Weapon 要保存 Handle

因为 Infinite GE 不会自动结束，需要找到那一次 Active GE 并移除。

## 为什么 Poison 清除后不回血

清除只会停止未来周期。已经执行的每次 `Health -5` 已经修改了属性。

## 为什么 Oiled 没有 Modifier 也有意义

GE 也可以只管理 GameplayTag 状态，不一定修改数值属性。

## 为什么 Burning 不在 Character 中手动判断标签后扣血

应用条件是 GE 本身的规则，放在 Application Tag Requirements 中可以被 ASC 统一处理，也方便复用和配置。

## 为什么 F5 复用 GE_Damage

GE 描述“如何修改目标”，对自己还是对别人由应用 API 决定。

同一个伤害规则既可以：

```text
ApplySpecToSelf
ApplySpecToTarget
```

---

# 37. 本课暂时没有展开的内容

当前课程已经覆盖 GE 最常见的基础配置：

```text
Instant
Duration
Infinite
Modifier
Add
Multiply
SetByCaller
Scalable Float Curve
Period
Stack
Granted Tags
Application Requirements
Handle
Tag Query
Apply To Target
```

还没有展开：

```text
Attribute Based Magnitude
MMC
GameplayEffectExecutionCalculation
Source / Target Attribute Capture
Snapshot
GameplayCue
Immunity
Chance To Apply
Additional Effects
Remove Other Effects Component
Ongoing Tag Requirements
Granted Abilities
客户端预测和 PredictionKey
```

其中伤害系统下一步最重要的是：

```text
攻击者 Attack
    + 技能基础伤害
    - 目标 Defense
    = 最终伤害
```

这种多属性结算通常会进入 MMC、Attribute Capture 和 ExecutionCalculation。

---

# 38. 这一节真正需要记住的内容

## 第一

```text
AttributeSet 定义属性
GameplayEffect 定义属性和状态怎样变化
ASC 执行和管理这些变化
```

## 第二

```text
GE Class 是模板
Spec 是本次执行数据
ActiveGameplayEffect 是正在运行的实例
Handle 用于找到具体实例
```

## 第三

```text
Instant 立即执行
Duration 到期移除
Infinite 主动移除
```

## 第四

```text
Modifier = Attribute + Op + Magnitude
```

## 第五

```text
SetByCaller 和 GE Level 都属于 Spec 的动态数据
```

## 第六

```text
GE 不一定修改属性
它也可以只授予 GameplayTag 或定义应用条件
```

## 第七

```text
对自己和对目标可以复用同一个 GE
区别在于 ASC 使用的应用接口
```

## 第八

```text
多人游戏中，由服务器权威应用 GE
客户端接收 ASC 和 AttributeSet 的复制结果
```

最后用一句话总结：

> GameplayEffect 是 GAS 中统一描述属性变化、持续状态、周期结算、叠层规则和应用条件的数据对象；ASC 把这份规则应用到目标的 AttributeSet，并负责其生命周期与网络同步。
