# UE C++ 联机教程04：Ownership

---

# 本节目标

理解：

* 为什么有些 Server RPC 调不动
* 为什么很多公共对象不直接接收客户端 RPC
* 为什么 Weapon、Inventory、Ability 可以拥有自己的 RPC
* Ownership 到底解决了什么问题

---

# 课程开场

前几节课我们已经学会了：

* 每台机器都有自己的世界
* Replication 用于同步状态
* RPC 用于发送请求
* Gameplay Framework 的职责划分

现在我们遇到一个新的问题：

同样都是 Actor。

为什么有些 Actor 能发 Server RPC。

有些 Actor 却不行？

---

# Demo1

## Door 直接发 Server RPC

场景：

```text
Character
DoorActor
```

DoorActor 上实现：

```cpp
Server_TestDoorRPC()
```

客户端按下按键：

```cpp
DoorActor->Server_TestDoorRPC();
```

结果：

```text
服务器没有执行
```

---

# 对比实验

Character 上实现：

```cpp
Server_TestCharacterRPC()
```

客户端按下按键：

```cpp
Character->Server_TestCharacterRPC();
```

结果：

```text
服务器成功执行
```

---

# 问题

同样都是 Actor。

为什么：

```text
Door 失败

Character 成功
```

---

# 第一结论

UE 并不是允许所有 Actor 都接收客户端 Server RPC。

只有某些特殊 Actor 才有资格作为客户端请求入口。

---

# Demo2

## Character 作为统一请求入口

不让 Door 接收 RPC。

改为：

```cpp
Character->Server_Interact(Door)
```

服务器：

```cpp
Door->Open()
```

结果：

```text
门成功打开
```

---

# 为什么这样设计

很多公共对象：

```text
Door
Chest
Tree
NPC
Boss
BuildActor
```

都属于：

```text
世界对象
```

而不是：

```text
某个玩家
```

---

因此实际项目中更常见的是：

```text
Client
↓
Character
↓
Server
↓
Door
```

而不是：

```text
Client
↓
Door
↓
Server
```

---

# 这会不会导致所有 RPC 都写到 Character

不会。

---

# Demo3

## 私有组件分担 RPC

Character：

```text
InteractionComponent
WeaponComponent
InventoryComponent
```

---

WeaponComponent：

```cpp
Server_Fire()
```

客户端调用：

```cpp
WeaponComponent->Server_Fire()
```

成功。

---

InventoryComponent：

```cpp
Server_UseItem()
```

客户端调用：

```cpp
InventoryComponent->Server_UseItem()
```

成功。

---

# 说明

Ownership 并不是：

```text
所有 RPC 都写 Character
```

---

而是：

```text
所有客户端请求
必须来自玩家拥有的对象
```

---

这些对象可以是：

```text
Character

Weapon

InventoryComponent

BuildComponent

AbilitySystemComponent
```

---

# Ownership

终于来到今天的主题。

---

# Ownership 是什么

简单理解：

```text
Actor 是否归属于某个玩家连接
```

---

例如：

```text
Weapon
↓
Character
↓
PlayerController
↓
Connection
```

---

UE 能找到：

```text
Connection
```

---

于是：

```cpp
Weapon->Server_Fire()
```

允许执行。

---

而：

```text
Door
↓
nullptr
```

---

UE 找不到：

```text
Connection
```

---

于是：

```cpp
Door->Server_OpenDoor()
```

拒绝执行。

---

# Connection 是什么

Connection：

```text
客户端和服务器之间的网络连接
```

---

服务器：

```text
Connection1
Connection2
Connection3
```

---

每个玩家都会拥有：

```text
PlayerController
```

---

关系：

```text
Connection
↓
PlayerController
```

---

因此：

```text
PlayerController
```

天然知道：

```text
对应哪个客户端
```

---

# Ownership 的本质

Ownership 真正解决的问题：

不是：

```text
Actor 属于谁
```

---

而是：

```text
UE 能否从当前对象找到对应的 Connection
```

---

找得到：

```text
允许 Server RPC
```

---

找不到：

```text
拒绝 Server RPC
```

---

# 重要结论

Ownership 并不是安全性的根源。

真正保证安全的是：

```text
Server Authority
```

和：

```text
服务器 Gameplay 验证
```

---

即使 RPC 成功进入服务器。

仍然必须验证：

```text
距离

权限

资源

状态

冷却
```

---

Ownership 只是：

```text
客户端请求入口约束
```

---

# 本课总结

公共对象：

```text
Door
Chest
Tree
NPC
Boss
```

通常不直接接收客户端 RPC。

---

玩家私有对象：

```text
Character

Weapon

Inventory

Ability

BuildComponent
```

可以拥有自己的 RPC。

---

Ownership 的本质：

```text
帮助 UE 建立

Actor
↓
Connection

之间的归属关系
```

---

一句话总结：

Ownership 不是让所有 RPC 集中到 Character。

Ownership 是让客户端请求必须来自玩家拥有的网络入口体系。
