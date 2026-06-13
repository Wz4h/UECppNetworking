# UE C++联机教程05：UE网络框架

## 本节课目标

前面几节课我们学习了：

* Replication
* RPC
* Gameplay Framework
* Ownership

但这些知识点都是分开的。

这一节课我们要回答一个更底层的问题：

```text
UE网络系统到底是怎么组织的？
```

---

# 一个问题

我们已经知道：

```cpp
UPROPERTY(Replicated)
int32 Health;
```

可以同步。

---

我们也知道：

```cpp
Server_Attack()
```

可以发送RPC。

---

还知道：

```cpp
SpawnActor()
```

可以把Actor同步到客户端。

---

以及：

```cpp
Destroy()
```

可以让客户端删除Actor。

---

那么问题来了：

```text
属性同步

RPC同步

Actor生成

Actor销毁
```

这些完全不同的事情。

为什么都能通过网络传输？

---

# 先看最简单的理解

很多人刚学联机时会认为：

```text
变量自己同步

函数自己同步

Actor自己同步
```

---

但实际上不是这样。

UE网络系统并不是这样组织的。

---

# UE网络框架

先记住下面这张图：

```text
NetDriver
│
└── Connection
     │
     ├── ActorChannel(Character)
     ├── ActorChannel(Monster)
     └── ActorChannel(Door)
```

这就是UE传统网络框架最重要的骨架。

---

# NetDriver

NetDriver可以理解成：

```text
网络总管理器
```

负责：

* 管理网络连接
* 处理网络Tick
* 管理网络同步

---

你可以把它理解成：

```text
服务器网络系统入口
```

---

# Connection

Connection表示：

```text
一个客户端连接
```

例如：

```text
Server

Client1
Client2
```

服务器会维护：

```text
Connection1
Connection2
```

---

所有网络消息最终都会通过Connection发送。

例如：

```text
RPC

Replication

Actor生成

Actor销毁
```

---

# ActorChannel

接下来才是本节课最重要的内容。

---

很多人以为：

```text
UE以变量同步
```

实际上不是。

---

UE网络系统真正的组织单位是：

```text
Actor
```

---

每个需要同步的Actor。

都会在对应Connection上建立：

```text
ActorChannel
```

---

例如：

```text
MonsterA
```

对于：

```text
Connection1
```

可能会创建：

```text
ActorChannel#15
```

---

对于：

```text
Connection2
```

可能会创建：

```text
ActorChannel#32
```

---

因此：

```text
同一个Actor

在不同Connection上

会拥有不同ActorChannel
```

---

# 为什么需要ActorChannel

假设服务器存在：

```text
CharacterA

MonsterA

DoorA
```

---

客户端也存在：

```text
CharacterA

MonsterA

DoorA
```

---

现在服务器发送：

```text
Health = 50
```

客户端会问：

```text
谁的Health？
```

---

于是UE不会直接发送：

```text
Health = 50
```

而是：

```text
ActorChannel#15

Health = 50
```

---

客户端立刻知道：

```text
这是CharacterA的数据
```

---

因此：

```text
ActorChannel
```

本质上是：

```text
服务器Actor

↓

客户端Actor

之间的同步通道
```

---

# Replication在哪里

现在再看属性同步。

例如：

```cpp
Health
```

发生变化。

---

实际上流程是：

```text
Health变化

↓

ActorChannel

↓

Connection

↓

Client
```

---

所以：

```text
Replication
```

并不是直接同步变量。

而是：

```text
通过ActorChannel同步Actor状态
```

---

# RPC在哪里

RPC也是一样。

例如：

```cpp
Server_Attack()
```

---

实际上：

```text
RPC

↓

ActorChannel

↓

Connection

↓

Server
```

---

因此：

```text
RPC和Replication

最终都会经过ActorChannel
```

---

# Spawn在哪里

服务器：

```cpp
SpawnActor()
```

---

如果Actor允许复制：

```cpp
bReplicates = true;
```

---

UE会：

```text
创建ActorChannel

↓

通知客户端创建副本
```

---

因此：

```text
Spawn
```

也属于ActorChannel体系。

---

# Destroy在哪里

服务器：

```cpp
Destroy()
```

---

UE会：

```text
关闭ActorChannel

↓

通知客户端删除副本
```

---

因此：

```text
Destroy
```

同样属于ActorChannel体系。

---

# 一个重要结论

前面我们学过：

```text
Replication

RPC

Ownership
```

---

现在我们终于能把它们串起来。

---

UE网络系统并不是：

```text
变量同步系统

RPC系统

Actor同步系统
```

三个独立系统。

---

而是：

```text
Actor
↓

ActorChannel
↓

Connection
↓

网络传输
```

---

所有网络行为都围绕Actor组织。

---

# Relevancy

最后补充一个问题。

---

是不是每个Actor都会拥有ActorChannel？

答案：

```text
不是
```

---

只有：

```text
Relevant
```

的Actor。

才会为对应Connection创建ActorChannel。

---

例如：

```text
1000米外怪物
```

可能根本不会同步。

---

因此：

```text
Relevancy
```

决定：

```text
Actor是否需要为某个Connection建立ActorChannel
```

---

关于Relevancy。

下一节课我们单独展开。

---

# 本节总结

记住下面这张图即可：

```text
NetDriver
↓

Connection
↓

ActorChannel
↓

Actor
```

以及：

```text
Replication
RPC
Spawn
Destroy
```

最终都会通过：

```text
ActorChannel
```

完成网络通信。

UE网络系统真正的组织单位不是变量。

也不是函数。

而是：

```text
Actor
```
