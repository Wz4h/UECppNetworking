# UE GAS 教程 01：AttributeSet —— GAS 的属性系统

## 1. 这一节要解决什么问题

这一节我们先不讲 GameplayAbility，也不讲 GameplayEffect。

只讲 GAS 里最基础，也最容易理解的一部分：

> AttributeSet。

在普通 UE 项目里，我们可能会这样定义角色属性：

```cpp
float Health = 100.0f;
float MaxHealth = 100.0f;
float Attack = 20.0f;
float Defense = 10.0f;
```

看起来完全没有问题。

那么 GAS 为什么还要专门设计一个 `AttributeSet`？

为什么不能直接用普通变量？

这一节我们主要解决几个问题：

1. AttributeSet 是什么？
2. GameplayAttributeData 是什么？
3. 它和普通 float 有什么区别？
4. 属性为什么需要 BaseValue 和 CurrentValue？
5. 属性是怎么被修改的？
6. 属性变化后怎么监听？
7. 属性怎么做最大值、最小值限制？
8. AttributeSet 在网络游戏中怎么同步？
9. 那些看起来很复杂的宏到底是干什么的？

学完这一节以后，你应该能够独立写出一个最基础的：

* Health
* MaxHealth
* Attack
* Defense

属性系统。

---

# 2. 从普通变量开始

假设我们现在正在做一个 RPG。

角色有：

```cpp
float Health;
float MaxHealth;
float Attack;
float Defense;
```

例如：

```cpp
UPROPERTY(BlueprintReadOnly)
float Health = 100.0f;

UPROPERTY(BlueprintReadOnly)
float MaxHealth = 100.0f;
```

受到伤害：

```cpp
Health -= Damage;
```

回血：

```cpp
Health += Heal;
```

限制血量：

```cpp
Health = FMath::Clamp(Health, 0.0f, MaxHealth);
```

从功能上来说，这当然能用。

问题是：

随着游戏越来越复杂，我们会不断增加新的需求。

比如：

* 装备增加 20 点攻击力
* Buff 增加 30% 攻击力
* Debuff 降低 10 点攻击力
* Buff 结束后恢复
* 临时增加最大生命值
* UI 监听血量变化
* 服务端修改血量
* 客户端同步血量
* 属性修改前限制数值
* 属性修改后触发死亡
* 不同系统都可能修改同一个属性

这个时候，一个简单的：

```cpp
float Health;
```

就开始不够用了。

所以 GAS 做了一件事情：

> 把“属性”本身变成 GAS 系统中的一种标准对象。

---

# 3. AttributeSet 是什么

AttributeSet 可以理解为：

> 一组 GAS 属性的容器。

例如我们创建：

```cpp
UCLASS()
class UGASAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
};
```

然后把角色属性放进去：

```cpp
UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData Health;

UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData MaxHealth;

UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData Attack;

UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData Defense;
```

完整一点：

```cpp
UCLASS()
class UGASAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly)
    FGameplayAttributeData Health;

    UPROPERTY(BlueprintReadOnly)
    FGameplayAttributeData MaxHealth;

    UPROPERTY(BlueprintReadOnly)
    FGameplayAttributeData Attack;

    UPROPERTY(BlueprintReadOnly)
    FGameplayAttributeData Defense;
};
```

从表面看：

以前是：

```cpp
float Health;
```

现在只是变成：

```cpp
FGameplayAttributeData Health;
```

但这里实际上发生了非常重要的变化。

因为现在 Health 不再只是一个数字。

它正式进入了 GAS 的属性系统。

---

# 4. FGameplayAttributeData 是什么

这是理解 AttributeSet 最重要的地方。

我们来看：

```cpp
FGameplayAttributeData Health;
```

很多人第一次看 GAS 都会有一个疑问：

> 为什么 Health 不直接是 float？

原因是：

`FGameplayAttributeData` 内部并不是只有一个值。

它主要包含：

```cpp
BaseValue
CurrentValue
```

也就是说，一个 GAS 属性其实有两个值：

```text
Health
│
├── BaseValue
│
└── CurrentValue
```

---

# 5. BaseValue 和 CurrentValue

这是 GAS 属性系统非常重要的概念。

假设角色攻击力：

```text
Attack = 100
```

那么：

```text
BaseValue = 100
CurrentValue = 100
```

现在角色获得一个 Buff：

```text
攻击力 +50
```

那么可能变成：

```text
BaseValue = 100
CurrentValue = 150
```

因为这个 Buff 只是一个临时修改。

Buff 消失以后：

```text
BaseValue = 100
CurrentValue = 100
```

所以：

## BaseValue

可以理解成：

> 属性的基础值。

通常保存比较长期的属性结果。

例如：

```text
角色基础攻击力
升级后的攻击力
永久属性成长
```

---

## CurrentValue

可以理解成：

> 当前真正生效的最终值。

它可能受到：

* Buff
* Debuff
* 装备
* GameplayEffect
* Modifier

等影响。

例如：

```text
基础攻击力：100
Buff：+50
Debuff：-20
```

最终：

```text
CurrentValue = 130
```

---

# 6. 为什么需要两个值

如果只有：

```cpp
float Attack;
```

那么一个 Buff：

```text
攻击力 +50
```

可能直接写：

```cpp
Attack += 50;
```

Buff 结束：

```cpp
Attack -= 50;
```

看起来好像也没问题。

但是如果同时存在：

```text
Buff A：+50
Buff B：+30%
Debuff C：-20
装备 D：+40
```

而且：

```text
Buff A 先结束
装备 D 被卸下
Buff B 继续存在
Debuff C 被驱散
```

你就需要自己管理：

```text
谁加了多少？
什么时候删除？
删除后该恢复多少？
不同修改之间是什么顺序？
百分比怎么计算？
```

这会变得非常麻烦。

GAS 的思路不是：

```text
每次修改变量，然后再想办法改回去
```

而是：

```text
记录属性基础值
+
记录所有 Modifier
+
系统自动计算最终值
```

这就是 BaseValue 和 CurrentValue 存在的重要意义。

---

# 7. AttributeSet 本身负责什么

这里需要注意一个非常容易误解的地方：

> AttributeSet 本身不是“属性管理器”。

它更像：

> GAS 属性的定义和规则集合。

通常负责：

```text
定义有哪些属性
属性修改前做什么
属性修改后做什么
属性网络同步
提供属性访问接口
```

而真正管理这些属性的核心对象其实是：

```text
AbilitySystemComponent
```

也就是 ASC。

可以先简单记成：

```text
AttributeSet
负责定义属性

ASC
负责管理属性

GameplayEffect
负责修改属性
```

这一节我们暂时不展开 ASC 和 GameplayEffect。

只需要先建立这个关系。

---

# 8. 创建一个 AttributeSet

首先创建：

```cpp
MyAttributeSet.h
MyAttributeSet.cpp
```

头文件：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.generated.h"

UCLASS()
class UMyAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UMyAttributeSet();

};
```

现在我们增加：

```text
Health
MaxHealth
Attack
Defense
```

---

# 9. 定义属性

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData Health;

UPROPERTY(BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData MaxHealth;

UPROPERTY(BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData Attack;

UPROPERTY(BlueprintReadOnly, Category = "Attributes")
FGameplayAttributeData Defense;
```

这样我们就定义了四个 GAS 属性。

---

# 10. 为什么网上的 AttributeSet 有一堆宏

很多 GAS 教程第一次就会写：

```cpp
ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)
```

新手很容易懵：

> 这又是什么？

我们先不要死记。

这些宏本质上只是：

> 自动生成一组访问属性的函数。

因为 GAS 不只是想知道：

```cpp
Health 当前是多少
```

还需要知道：

```text
Health 这个属性本身是谁
怎么获取它
怎么设置它
怎么初始化它
```

所以通常需要四个函数。

---

# 11. ATTRIBUTE_ACCESSORS 到底生成了什么

常见写法：

```cpp
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
```

然后：

```cpp
ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)
```

实际上相当于帮我们生成四类东西。

---

## 11.1 Property Getter

```cpp
GAMEPLAYATTRIBUTE_PROPERTY_GETTER
```

它返回的是：

```cpp
FGameplayAttribute
```

注意：

不是 float。

而是：

```cpp
FGameplayAttribute
```

可以理解为：

> 对这个属性的描述。

例如：

```cpp
GetHealthAttribute()
```

不是问：

```text
Health 等于多少？
```

而是在问：

```text
你说的 Health 是哪个属性？
```

GameplayEffect、ASC 等系统需要通过这个对象识别属性。

---

# 12. FGameplayAttribute 是什么

这里容易和：

```cpp
FGameplayAttributeData
```

混淆。

简单区分：

```text
FGameplayAttributeData
=
属性的数据

FGameplayAttribute
=
属性的描述
```

例如：

```text
Health = 100
```

其中：

```cpp
FGameplayAttributeData
```

保存：

```text
100
```

而：

```cpp
FGameplayAttribute
```

描述的是：

```text
UMyAttributeSet::Health
```

也就是：

> 这是哪个 AttributeSet 里的哪个属性。

---

# 13. Value Getter

宏还会生成：

```cpp
GetHealth()
```

它返回：

```cpp
float
```

相当于：

```cpp
Health.GetCurrentValue()
```

所以：

```cpp
AttributeSet->GetHealth();
```

可以直接获得当前血量。

---

# 14. Value Setter

还会生成：

```cpp
SetHealth(float NewValue)
```

用于修改 CurrentValue。

不过实际 GAS 项目中：

> 不推荐到处直接 SetHealth。

正式游戏逻辑通常更希望通过：

```text
GameplayEffect
```

修改属性。

因为 GameplayEffect 可以统一参与：

```text
Buff
Debuff
Modifier
预测
网络同步
Tag
持续时间
堆叠
```

但初始化、调试以及某些特殊逻辑里仍然可能使用 Setter。

---

# 15. Value Initializer

还会生成：

```cpp
InitHealth(float NewValue)
```

它通常用于：

> 初始化 BaseValue 和 CurrentValue。

例如：

```cpp
InitHealth(100.0f);
```

初始化以后：

```text
BaseValue = 100
CurrentValue = 100
```

---

# 16. 最终属性定义

所以我们通常会这样写：

```cpp
UCLASS()
class UMyAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UMyAttributeSet();

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxHealth)

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Attack;
    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Attack)

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Defense)
};
```

于是每个属性自动拥有：

```text
GetHealthAttribute()
GetHealth()
SetHealth()
InitHealth()
```

类似的一套接口。

---

# 17. 属性初始化

最简单的初始化方式：

```cpp
UMyAttributeSet::UMyAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitAttack(20.0f);
    InitDefense(10.0f);
}
```

现在角色初始状态：

```text
Health = 100
MaxHealth = 100
Attack = 20
Defense = 10
```

不过正式项目中，通常不会把所有角色属性硬编码在构造函数。

因为不同角色可能有不同属性。

例如：

```text
战士
Health = 200

法师
Health = 100

Boss
Health = 5000
```

后面我们会通过 GameplayEffect 或数据资产进行初始化。

第一节先使用构造函数即可。

---

# 18. 属性修改前：PreAttributeChange

AttributeSet 提供了一个非常重要的函数：

```cpp
virtual void PreAttributeChange(
    const FGameplayAttribute& Attribute,
    float& NewValue
) override;
```

它的含义是：

> 属性的 CurrentValue 即将发生变化。

例如：

```text
Health 原来 = 80
现在准备改成 = 130
```

在真正变成 130 之前：

```cpp
PreAttributeChange(...)
```

会被调用。

---

# 19. 为什么需要 PreAttributeChange

最常见用途就是：

> Clamp。

例如：

```text
Health 不能超过 MaxHealth。
```

我们可以写：

```cpp
void UMyAttributeSet::PreAttributeChange(
    const FGameplayAttribute& Attribute,
    float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(
            NewValue,
            0.0f,
            GetMaxHealth()
        );
    }
}
```

现在：

```text
Health = 80

回血 +100
```

本来应该：

```text
180
```

但：

```cpp
Clamp(180, 0, 100)
```

最终变成：

```text
100
```

---

# 20. PreAttributeChange 适合做什么

它比较适合：

```text
限制 CurrentValue
```

例如：

```text
移动速度不能低于 100
攻击速度不能超过 3
血量不能超过最大血量
暴击率限制在 0~1
```

例如：

```cpp
if (Attribute == GetMoveSpeedAttribute())
{
    NewValue = FMath::Clamp(
        NewValue,
        100.0f,
        1000.0f
    );
}
```

---

# 21. 属性修改后：PostGameplayEffectExecute

AttributeSet 另外一个非常重要的函数：

```cpp
virtual void PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data
) override;
```

这个函数表示：

> GameplayEffect 已经修改完属性。

例如一个伤害 GameplayEffect：

```text
Health - 20
```

执行以后：

```cpp
PostGameplayEffectExecute
```

会被调用。

---

# 22. PostGameplayEffectExecute 能干什么

例如：

```cpp
void UMyAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(
            FMath::Clamp(
                GetHealth(),
                0.0f,
                GetMaxHealth()
            )
        );
    }
}
```

我们可以在这里：

```text
限制最终生命值
判断死亡
触发特殊逻辑
处理 Damage
处理 Heal
```

例如：

```cpp
if (GetHealth() <= 0.0f)
{
    // Death
}
```

---

# 23. PreAttributeChange 和 PostGameplayEffectExecute 区别

这是非常容易混淆的地方。

简单理解：

## PreAttributeChange

发生在：

```text
CurrentValue 即将变化
```

更适合：

```text
限制即将写入的数值
```

例如：

```text
MoveSpeed 最大 1000
```

---

## PostGameplayEffectExecute

发生在：

```text
GameplayEffect 已经执行完成
```

更适合：

```text
根据修改结果执行游戏逻辑
```

例如：

```text
血量 <= 0
角色死亡
```

可以暂时理解成：

```text
PreAttributeChange
=
改之前

PostGameplayEffectExecute
=
GameplayEffect 改完之后
```

---

# 24. 为什么很多项目不直接修改 Health

例如我们受到：

```text
20 点伤害
```

最直觉写法：

```cpp
SetHealth(GetHealth() - 20.0f);
```

虽然能用。

但是大型 GAS 项目通常不会这么写。

而是：

```text
Damage
↓
GameplayEffect
↓
ASC
↓
修改 Attribute
↓
AttributeSet
```

原因是如果所有东西都直接：

```cpp
SetHealth()
```

那 GAS 提供的大量能力就绕过去了。

例如：

```text
Buff
Debuff
Tag
Modifier
预测
堆叠
持续时间
来源
网络
```

所以正式玩法属性尽量通过 GameplayEffect 修改。

---

# 25. 为什么 Damage 经常也是一个 Attribute

很多 GAS 项目还会定义：

```cpp
FGameplayAttributeData Damage;
```

这时候很多人会问：

> Damage 又不是角色长期属性，为什么也放 AttributeSet？

因为有一种属性叫：

> Meta Attribute。

也就是临时属性。

例如：

```text
攻击产生 100 Damage
```

GameplayEffect 先修改：

```text
Damage = 100
```

然后 AttributeSet 读取 Damage：

```text
Damage
↓
Defense
↓
最终伤害
↓
Health
```

例如：

```cpp
const float LocalDamage = GetDamage();

SetDamage(0.0f);

const float FinalDamage =
    FMath::Max(LocalDamage - GetDefense(), 0.0f);

SetHealth(GetHealth() - FinalDamage);
```

这里 Damage 只是一个：

> 中间计算数据。

使用完成后：

```cpp
Damage = 0
```

所以它并不是永久角色属性。

---

# 26. 为什么不直接 GameplayEffect 修改 Health

假设：

```text
攻击力 = 100
敌人防御 = 30
```

最终伤害应该：

```text
70
```

如果 GameplayEffect 直接：

```text
Health -100
```

那么防御系统就很难统一处理。

更常见的架构是：

```text
攻击
↓
Damage = 100
↓
AttributeSet
↓
读取 Defense = 30
↓
FinalDamage = 70
↓
Health -= 70
```

这样所有伤害都统一经过：

```text
Damage → Health
```

之后就很容易加入：

```text
护盾
免伤
暴击
抗性
死亡
吸血
伤害日志
```

---

# 27. AttributeSet 不是业务逻辑垃圾桶

这里非常重要。

AttributeSet 可以处理：

```text
属性约束
伤害结算
属性相关规则
```

但不要把所有业务逻辑都塞进 AttributeSet。

比如：

```text
播放死亡动画
播放音效
生成金币
弹死亡 UI
切换关卡
刷新任务
```

这些都不应该直接塞在 AttributeSet。

AttributeSet 更应该负责：

```text
Health 变成 0
```

然后：

```text
其他系统监听 Health
```

再处理死亡。

也就是说：

```text
AttributeSet
负责属性

角色系统
负责角色行为
```

---

# 28. 怎么监听属性变化

假设 UI 需要监听：

```text
Health
```

以前如果使用普通 float：

```cpp
Health -= Damage;
```

那么 UI 怎么知道 Health 变了？

我们可能需要自己写：

```cpp
OnHealthChanged.Broadcast();
```

但是 GAS 已经统一提供了属性变化事件。

ASC 可以：

```cpp
GetGameplayAttributeValueChangeDelegate(
    UMyAttributeSet::GetHealthAttribute()
)
```

然后绑定：

```cpp
ASC->GetGameplayAttributeValueChangeDelegate(
    UMyAttributeSet::GetHealthAttribute()
).AddUObject(
    this,
    &ThisClass::OnHealthChanged
);
```

函数：

```cpp
void AMyCharacter::OnHealthChanged(
    const FOnAttributeChangeData& Data)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Health: %f -> %f"),
        Data.OldValue,
        Data.NewValue
    );
}
```

这样无论 Health 被谁修改：

```text
技能
Buff
Debuff
伤害
回血
服务器同步
```

监听者都可以获得变化。

这就是 Attribute 相比普通 float 非常大的优势。

---

# 29. Attribute 的网络同步

如果这是一个联机游戏。

Health 不能只存在服务端。

客户端 UI 也需要知道：

```text
Health = 75
```

所以属性需要进行 Replication。

例如：

```cpp
UPROPERTY(
    BlueprintReadOnly,
    ReplicatedUsing = OnRep_Health,
    Category = "Attributes"
)
FGameplayAttributeData Health;
```

然后：

```cpp
UFUNCTION()
void OnRep_Health(
    const FGameplayAttributeData& OldHealth);
```

实现：

```cpp
void UMyAttributeSet::OnRep_Health(
    const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UMyAttributeSet,
        Health,
        OldHealth
    );
}
```

---

# 30. GAMEPLAYATTRIBUTE_REPNOTIFY 是什么

这个宏做的不只是：

```text
变量同步完成
```

它还会通知 GAS：

> 这个 Attribute 的值发生了网络变化。

这样 GAS 的属性变化 Delegate 等机制才能正确工作。

所以不能简单把它理解成：

```cpp
OnRep_Health()
```

它是：

```text
UE Replication
+
GAS Attribute 通知
```

之间的桥梁。

---

# 31. GetLifetimeReplicatedProps

还需要正常注册 Replication：

```cpp
void UMyAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UMyAttributeSet,
        Health,
        COND_None,
        REPNOTIFY_Always
    );
}
```

其他属性同样处理：

```cpp
DOREPLIFETIME_CONDITION_NOTIFY(
    UMyAttributeSet,
    MaxHealth,
    COND_None,
    REPNOTIFY_Always
);

DOREPLIFETIME_CONDITION_NOTIFY(
    UMyAttributeSet,
    Attack,
    COND_None,
    REPNOTIFY_Always
);
```

---

# 32. 为什么使用 REPNOTIFY_Always

普通 UE 属性：

```text
数值改变
↓
同步
```

就够了。

但 GAS 中：

Attribute 变化可能还需要：

```text
更新预测结果
触发 Attribute Change Delegate
同步 GameplayEffect 计算结果
```

所以 GAS 通常使用：

```cpp
REPNOTIFY_Always
```

确保 RepNotify 正确执行。

---

# 33. 一个完整的 AttributeSet

头文件：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MyAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UMyAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UMyAttributeSet();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    virtual void PreAttributeChange(
        const FGameplayAttribute& Attribute,
        float& NewValue
    ) override;

    virtual void PostGameplayEffectExecute(
        const FGameplayEffectModCallbackData& Data
    ) override;

public:

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Health,
        Category = "Attributes"
    )
    FGameplayAttributeData Health;

    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health)

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MaxHealth,
        Category = "Attributes"
    )
    FGameplayAttributeData MaxHealth;

    ATTRIBUTE_ACCESSORS(UMyAttributeSet, MaxHealth)

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Attack,
        Category = "Attributes"
    )
    FGameplayAttributeData Attack;

    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Attack)

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Defense,
        Category = "Attributes"
    )
    FGameplayAttributeData Defense;

    ATTRIBUTE_ACCESSORS(UMyAttributeSet, Defense)

protected:

    UFUNCTION()
    void OnRep_Health(
        const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxHealth(
        const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_Attack(
        const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_Defense(
        const FGameplayAttributeData& OldValue);
};
```

---

# 34. CPP 实现

```cpp
#include "MyAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMyAttributeSet::UMyAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitAttack(20.0f);
    InitDefense(10.0f);
}
```

---

# 35. PreAttributeChange

```cpp
void UMyAttributeSet::PreAttributeChange(
    const FGameplayAttribute& Attribute,
    float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(
            NewValue,
            0.0f,
            GetMaxHealth()
        );
    }
}
```

---

# 36. PostGameplayEffectExecute

```cpp
void UMyAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(
            FMath::Clamp(
                GetHealth(),
                0.0f,
                GetMaxHealth()
            )
        );
    }
}
```

---

# 37. Replication

```cpp
void UMyAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UMyAttributeSet,
        Health,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UMyAttributeSet,
        MaxHealth,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UMyAttributeSet,
        Attack,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UMyAttributeSet,
        Defense,
        COND_None,
        REPNOTIFY_Always
    );
}
```

---

# 38. RepNotify

```cpp
void UMyAttributeSet::OnRep_Health(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UMyAttributeSet,
        Health,
        OldValue
    );
}

void UMyAttributeSet::OnRep_MaxHealth(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UMyAttributeSet,
        MaxHealth,
        OldValue
    );
}

void UMyAttributeSet::OnRep_Attack(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UMyAttributeSet,
        Attack,
        OldValue
    );
}

void UMyAttributeSet::OnRep_Defense(
    const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UMyAttributeSet,
        Defense,
        OldValue
    );
}
```

---

# 39. 到这里 AttributeSet 到底帮我们做了什么

现在重新对比一下。

普通变量：

```cpp
float Health;
```

它只是：

```text
一个数字
```

但是 GAS Attribute：

```cpp
FGameplayAttributeData Health;
```

进入 GAS 以后，它拥有：

```text
BaseValue / CurrentValue

GameplayEffect 修改

Modifier 计算

属性变化监听

属性变化前处理

GameplayEffect 执行后处理

网络同步

预测系统支持

统一属性描述

统一 Getter / Setter

Buff / Debuff 自动参与计算
```

所以：

> GAS Attribute 的真正价值不是“存储数值”，而是让这个数值进入 GAS 的规则系统。

---

# 40. 普通 float 和 Attribute 的本质区别

可以这样理解：

```text
普通 float
=
一个数值
```

而：

```text
Gameplay Attribute
=
一个被 GAS 管理的游戏属性
```

普通变量：

```text
Health = 100
```

GAS 属性：

```text
Health
│
├── BaseValue
├── CurrentValue
├── Modifier
├── GameplayEffect
├── Replication
├── Delegate
├── Prediction
└── Attribute Rules
```

所以如果只是：

```text
一个永远不会被 Buff 修改的简单变量
```

不一定需要 Attribute。

但如果是：

```text
Health
Mana
Attack
Defense
MoveSpeed
CritRate
Armor
Shield
```

这种需要参与游戏玩法计算的属性，就非常适合 GAS Attribute。

---

# 41. 什么应该做 Attribute

比较适合：

```text
Health
MaxHealth

Mana
MaxMana

Attack
Defense

Armor
MagicResistance

MoveSpeed

AttackSpeed

CritRate

CritDamage

Shield
```

因为这些属性通常：

```text
会被 GameplayEffect 修改
会被 Buff / Debuff 修改
需要网络同步
需要 UI 监听
```

---

# 42. 什么不一定需要做 Attribute

例如：

```text
角色名字
角色 ID
背包容量
任务编号
当前动画状态
摄像机距离
UI 设置
```

这些东西虽然也是“数据”，但不是：

```text
GAS 游戏属性
```

就没必要强行放 AttributeSet。

不要形成：

> 只要是 float 就放 AttributeSet。

这是错误理解。

---

# 43. AttributeSet 最重要的设计原则

可以记住一句话：

> AttributeSet 存的是“会参与 GAS GameplayEffect 计算的角色属性”。

而不是：

> AttributeSet 存角色所有数据。

---

# 44. AttributeSet 与 ASC 的关系

这时候我们已经可以理解一个基本结构：

```text
Character
│
├── AbilitySystemComponent
│
└── AttributeSet
```

其中：

```text
AttributeSet
定义：
Health
Attack
Defense
```

而：

```text
AbilitySystemComponent
管理：
GameplayEffect
GameplayAbility
GameplayTag
Attribute
```

所以 AttributeSet 并不是独立工作的。

真正把它运行起来的是：

```text
ASC
```

---

# 45. 为什么 GAS 要把 AttributeSet 单独做成一个类

可能会有人问：

> 为什么不直接把 Health 写到 ASC 里面？

这是 GAS 非常重要的模块化设计。

假设游戏有：

```text
角色属性
战斗属性
经济属性
载具属性
```

可以拆成：

```text
UCharacterAttributeSet

UCombatAttributeSet

UEconomyAttributeSet

UVehicleAttributeSet
```

ASC 可以同时管理多个 AttributeSet。

也就是说：

```text
ASC
=
属性系统管理器

AttributeSet
=
一组属性定义
```

这让属性可以按照模块拆分。

---

# 46. 一个 ASC 可以有多个 AttributeSet

例如：

```text
AbilitySystemComponent

├── CharacterAttributeSet
│   ├── Health
│   ├── Mana
│   └── MoveSpeed
│
├── CombatAttributeSet
│   ├── Attack
│   ├── Defense
│   └── CritRate
│
└── EconomyAttributeSet
    ├── Gold
    └── Experience
```

所以：

> AttributeSet 不是 GAS 的全部属性系统。

它只是：

> 属性的组织方式。

---

# 47. 第一节真正需要记住的东西

这一节不需要记住所有 API。

只需要理解五件事。

---

## 第一

AttributeSet 是：

> GAS 属性的集合。

例如：

```text
Health
Attack
Defense
```

---

## 第二

一个 Attribute 不是普通 float。

它使用：

```cpp
FGameplayAttributeData
```

包含：

```text
BaseValue
CurrentValue
```

---

## 第三

AttributeSet 主要负责：

```text
定义属性
限制属性
处理属性变化
网络同步
```

---

## 第四

Attribute 本身通常由：

```text
GameplayEffect
```

进行修改。

而不是业务代码到处直接：

```cpp
SetHealth()
```

---

## 第五

真正管理 AttributeSet 的是：

```text
AbilitySystemComponent
```

整体关系：

```text
GameplayEffect
        ↓
AbilitySystemComponent
        ↓
AttributeSet
        ↓
Health / Attack / Defense
```

---

# 48. 第一节 Demo 建议

这一节 Demo 不需要做技能。

也不要做复杂战斗。

只做一个角色：

```text
Health = 100
MaxHealth = 100
Attack = 20
Defense = 10
```

屏幕显示：

```text
Health: 100
Attack: 20
Defense: 10
```

然后准备几个测试按钮：

```text
按 1
Health -20

按 2
Health +20

按 3
Attack +10

按 4
Attack -10
```

同时输出：

```text
OldValue
NewValue
```

例如：

```text
Health: 100 -> 80
```

这样可以直观看到：

```text
Attribute
属性变化监听
Clamp
Replication
```

整个链路。

---

# 49. 这一节暂时不要讲什么

第一节建议不要展开：

```text
GameplayAbility
AbilityTask
GameplayCue
复杂 GameplayEffect
ExecutionCalculation
MMC
PredictionKey
GameplayTag 深入原理
ASC 网络预测
```

这些东西现在加入，只会让 AttributeSet 变得难懂。

这一节只留下一个问题：

> 我们已经定义好了 Health，那么接下来应该由谁来修改它？

答案就是：

```text
GameplayEffect
```

这正好可以作为下一节。

---

# 50. 下一节预告

这一节我们解决了：

```text
游戏有哪些属性？
属性是怎么保存和管理的？
```

但是还有一个关键问题：

```text
攻击力 +20
受到 30 伤害
移动速度降低 50%
持续 5 秒的 Buff
```

这些东西到底怎么修改 Attribute？

这就是 GAS 的另外一个核心概念：

> GameplayEffect。

下一节：

# GameplayEffect —— GAS 如何修改属性
