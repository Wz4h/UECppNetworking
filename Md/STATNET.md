# 第九章：UE 网络优化

> 本章目标：理解 UE 网络优化的核心思想，掌握实际项目中最常用的网络优化方法，并能够利用 `stat net` 分析网络性能。

---

# 一、网络优化的本质

很多人认为网络优化就是：

* 压缩数据
* 提高带宽
* 更换网络协议

实际上，在 UE 游戏开发中，大多数网络优化都遵循一个原则：

> **网络优化不是让数据传得更快，而是让不该传的数据根本不传。**

整个优化思路可以概括为：

```text
减少同步对象
        ↓
减少同步频率
        ↓
减少同步数据
        ↓
减少服务器检查成本
```

---

# 二、网络分析工具 —— stat net

进入 PIE 联机模式（Server + Client），打开控制台：

```text
stat net
```

即可看到网络实时统计信息。

课程重点关注以下几个指标。

| 指标                 | 作用                    | 是否重点  |
| ------------------ | --------------------- | ----- |
| Out Rate           | 每秒发送带宽                | ⭐⭐⭐⭐⭐ |
| In Rate            | 每秒接收带宽                | ⭐⭐⭐⭐  |
| Num Network Actors | 当前参与网络同步的 Actor 数量    | ⭐⭐⭐⭐⭐ |
| Num Actor Channels | 当前建立的 ActorChannel 数量 | ⭐⭐⭐⭐  |
| Ping               | 网络延迟                  | ⭐⭐⭐⭐  |

> 所有网络优化都不要凭感觉，而是观察 `stat net` 数据是否发生变化。

---

# 三、网络优化 Demo

本章使用四个 Demo 演示 UE 中最常见的网络优化方式。

---

# Demo 1：Relevancy（相关性优化）

## 问题

服务器：

```text
Spawn 1000 个 Replicated Cube
```

默认情况下：

```text
所有 Cube
↓

全部参与同步
```

观察：

```text
stat net
```

可以看到：

* Network Actors 上升
* Actor Channels 上升
* Out Rate 上升

---

## 优化

设置：

```cpp
bAlwaysRelevant = false;
NetCullDistanceSquared = ...
```

远离 Cube 后：

```text
Not Relevant
↓

不建立 ActorChannel

↓

不参与同步
```

再次观察：

```text
stat net
```

可以发现：

* Network Actors 基本不变
* Actor Channels 减少
* Out Rate 明显下降

---

## 实际项目案例

适用于：

* 大地图 NPC
* 怪物
* 掉落物
* 车辆
* 建筑
* 玩家

几乎所有多人游戏都会使用 Relevancy。

---

# Demo 2：NetUpdateFrequency（同步频率）

## 问题

Replicated Actor 默认会按照一定频率同步。

如果：

```cpp
NetUpdateFrequency = 100;
```

则：

```text
每秒尝试同步约100次
```

即使变化很小，也需要不断检查。

---

## 优化

根据对象的重要程度降低同步频率。

例如：

| 对象  | 推荐频率      |
| --- | --------- |
| 玩家  | 30~100 Hz |
| NPC | 5~20 Hz   |
| 门   | 1~5 Hz    |
| 宝箱  | 状态变化时     |

再次观察：

```text
stat net
```

可以发现：

```text
Out Rate

下降
```

---

## 实际项目案例

* NPC
* 巡逻怪
* 电梯
* 移动物体
* AI

---

# Demo 3：FastArray（增量同步）

## 问题

假设背包：

```text
100 个物品
```

修改：

```text
第 37 个物品
```

普通同步：

```text
整个数组参与同步
```

网络开销较大。

---

## 优化

使用：

```cpp
FFastArraySerializer
```

修改：

```text
第 37 个

↓

只同步第 37 个
```

而不是整个数组。

---

## 实际项目案例

FastArray 广泛用于：

* 背包
* Buff 列表
* 技能列表
* 队伍成员
* 任务列表
* GAS Attribute 容器（部分场景）

---

# Demo 4：Dormancy（休眠）

## 问题

默认情况下：

Replicated Actor 即使属性没有变化，

服务器仍然需要每帧检查：

```text
有没有变量变化？

需不需要同步？
```

如果：

```text
5000 扇门
```

虽然没有发送任何数据，

服务器依然需要不断检查。

---

## 优化

当确定 Actor 一段时间不会变化：

```cpp
SetNetDormancy(DORM_DormantAll);
```

服务器将停止检查该 Actor。

当需要修改时：

```cpp
FlushNetDormancy();
```

同步完成后：

```cpp
SetNetDormancy(DORM_DormantAll);
```

再次进入休眠。

---

## 实际项目案例

适用于：

* 门
* 宝箱
* 开关
* 电梯
* 静态机关
* 可破坏物

> Dormancy 主要优化服务器 CPU，而不是带宽。

---

# 四、四种优化方式总结

| 方法                 | 优化目标    | 常见对象      |
| ------------------ | ------- | --------- |
| Relevancy          | 减少同步对象  | 玩家、NPC、怪物 |
| NetUpdateFrequency | 降低同步频率  | AI、移动物体   |
| FastArray          | 减少同步数据  | 背包、Buff   |
| Dormancy           | 减少服务器检查 | 门、宝箱、机关   |

---

# 五、实际项目中的优化顺序

实际开发一般不会直接研究 ReplicationGraph 或 Iris。

通常按照以下顺序优化：

```text
① 能不同步就不同步
        ↓
② 能降低同步频率就降低
        ↓
③ 能只同步变化的数据就不要同步全部
        ↓
④ 长时间不变化的 Actor 进入 Dormancy
        ↓
⑤ 大规模项目再考虑 ReplicationGraph
        ↓
⑥ 最后再考虑 Iris
```

---

# 六、拓展知识

## Relevancy

作用：

```text
决定：

同步谁
```

适合：

* 大地图
* MMO
* 开放世界

---

## ReplicationGraph

作用：

```text
优化 Relevancy 的计算过程
```

适用于：

* 上百玩家
* 大型开放世界
* MMO

普通 4 人联机项目一般不需要。

---

## Iris

UE5 新一代 Replication 系统。

作用：

```text
优化 Replication 内部实现
```

开发者通常无需修改 Gameplay 代码。

---

## Prediction

作用：

```text
减少操作延迟
```

典型：

* CharacterMovement
* GAS
* 技能释放

---

## Lag Compensation

作用：

```text
服务器查看历史状态
```

典型：

* FPS 命中判定

---

## Rollback

作用：

```text
恢复历史状态

↓

重新模拟游戏
```

典型：

* 格斗游戏
* RTS
* 确定性同步

---

# 七、本章总结

网络优化不是某一种技术。

而是一套逐层减少同步成本的方法。

真正的优化思路可以总结为：

```text
Relevancy
↓

减少同步对象

↓

NetUpdateFrequency
↓

降低同步频率

↓

FastArray
↓

减少同步数据

↓

Dormancy
↓

减少服务器检查
```

最后记住一句话：

> **网络优化不是让网络更快，而是让网络需要发送的数据越来越少。**
