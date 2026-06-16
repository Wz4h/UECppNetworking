# UE C++联机教程06：Relevancy（网络相关性）

---

## 本节课目标

上一节课我们学习了：

```text
NetDriver
↓
Connection
↓
ActorChannel
↓
Actor
```

我们知道：

- Replication
- RPC
- Spawn
- Destroy

最终都会通过 ActorChannel 完成网络通信。

---

那么新的问题来了：

> 是不是每个 Actor 都会为每个 Connection 创建 ActorChannel？

答案是：

**不是。**

本节课我们学习：

# Relevancy

即：

> Actor 是否值得同步给某个 Connection

---

# 为什么需要 Relevancy

假设服务器存在：

| 类型 | 数量 |
|--------|--------|
| 玩家 | 500 |
| 怪物 | 1000 |
| 掉落物 | 2000 |
| 资源点 | 5000 |

总计：

```text
8500个Actor
```

---

如果：

```text
8500个Actor
同步给
500个玩家
```

服务器需要维护：

```text
8500 × 500
```

份同步关系。

显然无法接受。

---

因此服务器必须回答：

> 这个 Actor 是否值得同步给这个客户端？

这就是：

```text
Relevancy
```

---

# Relevancy 的本质

```text
Actor
↓
Relevancy 判断
↓
Relevant ?
↓
是
↓
创建 ActorChannel
↓
开始同步
```

---

如果：

```text
Not Relevant
```

则：

```text
关闭 ActorChannel
↓
停止同步
```

---

## 重要

Actor 在客户端消失：

不一定是 Destroy。

很可能是：

```text
Not Relevant
```

导致 ActorChannel 被关闭。

---

# 第一种：距离相关

最常见的 Relevancy。

---

## 示例

玩家：

```text
PlayerA
```

怪物：

```text
MonsterA
```

---

距离：

```text
20m
```

结果：

```text
Relevant
```

客户端看到怪物。

---

距离：

```text
2000m
```

结果：

```text
Not Relevant
```

客户端怪物消失。

---

## 实际案例

### MMO

```text
怪物
NPC
掉落物
采集点
```

全部使用距离相关。

---

### 开放世界

```text
动物
资源
普通NPC
```

全部使用距离相关。

---

### 生存游戏

```text
树木
矿石
资源箱
野兽
```

玩家附近同步即可。

---

# 第二种：AlwaysRelevant

有些 Actor 无论距离多远都必须同步。

---

## 示例

```cpp
GameState
```

---

里面保存：

```text
比赛时间
剩余人数
当前阶段
胜利条件
```

---

这些信息所有玩家都需要。

因此：

```text
AlwaysRelevant
```

---

## 实际案例

| 游戏类型 | 对象 |
|----------|----------|
| 吃鸡 | 毒圈状态 |
| MOBA | 比赛时间 |
| RTS | 胜利条件 |
| 副本 | Boss阶段 |

---

# 第三种：OnlyRelevantToOwner

只同步给主人。

---

## 示例

Player1：

```text
护送任务
```

Player2：

```text
击杀任务
```

---

双方任务信息互不相关。

因此：

```text
OnlyRelevantToOwner
```

---

## 实际案例

### 背包

```text
Inventory
```

---

### 任务系统

```text
Quest
```

---

### 个人奖励

```text
DailyReward
```

---

### 新手引导

```text
TutorialActor
```

---

# 自定义 Relevancy

默认规则并不能覆盖所有项目。

因此 UE 提供：

```cpp
IsNetRelevantFor()
```

---

# 案例1：队伍可见

Team1：

```text
PlayerA
PlayerB
```

---

Team2：

```text
PlayerC
PlayerD
```

---

地图存在：

```text
Team1 Trap
```

---

结果：

| 玩家 | 是否可见 |
|--------|--------|
| A | ✅ |
| B | ✅ |
| C | ❌ |
| D | ❌ |

---

## 实际应用

- 战术射击
- RTS
- MOBA

---

# 案例2：战争迷雾

MOBA最经典案例。

---

服务器：

```text
敌方英雄存在
```

---

但：

```text
没有视野
```

---

结果：

```text
Not Relevant
```

---

注意：

不是隐藏。

不是透明。

而是：

```text
客户端根本不存在该Actor
```

---

# 案例3：副本隔离

服务器：

```text
副本1
副本2
副本3
```

---

Boss始终存在。

---

但：

```text
副本1玩家
```

不应该看到：

```text
副本2 Boss
```

---

因此：

```text
InstanceId
↓
Relevancy判断
```

---

# 案例4：观察者模式

死亡玩家：

```text
Observer
```

---

希望：

```text
全图可见
```

---

特殊处理：

```cpp
IsNetRelevantFor()
```

返回：

```text
true
```

---

# Ownership 与 Relevancy

很多新人容易混。

---

## Ownership

解决：

```text
谁能发RPC
```

---

例如：

```text
Door
Chest
Monster
```

没有 Ownership。

---

客户端不能直接调用：

```cpp
Server_OpenDoor()
```

---

## Relevancy

解决：

```text
谁能看到Actor
```

---

例如：

```text
Monster
```

存在于服务器。

但：

```text
不Relevant
```

客户端仍然看不到。

---

# 本节总结

## Relevancy 决定什么？

```text
Actor
↓
Relevancy判断
↓
Relevant ?
↓
创建ActorChannel
↓
开始同步
```

---

## UE内置规则

| 规则 | 用途 |
|--------|--------|
| 距离相关 | 怪物、掉落物 |
| AlwaysRelevant | GameState |
| OnlyRelevantToOwner | 背包、任务 |

---

## 常见自定义规则

| 规则 | 场景 |
|--------|--------|
| 队伍可见 | RTS / MOBA |
| 战争迷雾 | MOBA |
| 副本隔离 | MMO |
| 观察者模式 | Spectator |

---

## 一句话记忆

```text
Ownership
决定谁能发消息

Relevancy
决定谁能看到Actor
```

```text
Actor
↓
Relevancy
↓
ActorChannel
↓
Connection
↓
Client
```