# UE C++ 网络教程 第七课：Prediction、Authority 与 Correction

## 本课目标

理解：

- 什么是 Prediction
- 什么是 Authority
- 什么是 Correction
- 为什么 CharacterMovement 不需要自己写移动 RPC
- Demo 中的 Prediction 与真实项目中的 Prediction 有什么区别

最终建立认知：

```text
CharacterMovement
=
Prediction
+
Authority
+
Correction
```

并理解：

```text
GAS
=
Prediction
+
Authority
+
Correction
```

---

# 为什么需要 Prediction

前面课程中。

我们已经学习了：

```text
Client Input

↓

Server RPC

↓

Server Logic

↓

Replication

↓

Client Update
```

这种方式适用于：

- 开门
- 拾取物品
- 修改状态
- 修改血量

因为这些功能对于延迟并不敏感。

但是角色移动不一样。

---

# 严格服务器权威的问题

假设：

客户端按下 W。

流程：

```text
Client Input

↓

Server RPC

↓

Server Move

↓

Replication

↓

Client Update
```

服务器决定最终位置。

逻辑完全正确。

因为：

```text
Server Is Authority
```

服务器拥有最终权威。

---

# 高延迟下会发生什么

假设网络延迟：

```text
100ms
```

流程：

```text
客户端按下W

↓

100ms

↓

服务器收到输入

↓

服务器移动

↓

100ms

↓

客户端收到结果
```

总等待时间：

```text
200ms
```

表现：

```text
按键

↓

角色过一会才开始移动
```

玩家会明显感觉到迟钝。

---

# Prediction

解决方案：

客户端不要等待服务器。

先自己移动。

同时发送请求。

流程：

```text
Client Input

↓

Client Local Move

+

Server RPC
```

客户端认为：

```text
服务器大概率会同意
```

所以直接显示结果。

这种行为称为：

```text
Prediction
```

预测。

---

# Prediction 的本质

Prediction 并不是同步技术。

Prediction 的本质：

```text
客户端先执行

假设服务器会同意
```

核心目标：

```text
改善操作手感
```

而不是：

```text
改变服务器权威
```

---

# Prediction 的好处

没有 Prediction：

```text
按键

↓

等待网络

↓

角色移动
```

有 Prediction：

```text
按键

↓

角色立即移动
```

玩家感觉：

```text
输入立即生效
```

---

# Prediction 的问题

Prediction 本质：

```text
先执行

后验证
```

因此：

客户端结果

可能不等于

服务器结果。

例如：

客户端：

```text
MoveSpeed = 1200
```

服务器：

```text
MoveSpeed = 600
```

客户端认为：

```text
应该移动10米
```

服务器认为：

```text
只能移动5米
```

最终：

```text
位置不一致
```

---

# Authority

当客户端和服务器产生冲突时。

谁说了算？

答案：

```text
服务器
```

这就是：

```text
Authority
```

服务器权威。

无论客户端预测什么。

最终结果永远由服务器决定。

---

# Correction

服务器发现：

```text
客户端预测错误
```

于是：

```text
发送正确结果
```

客户端收到后：

```text
修正位置
```

例如：

```text
Client Position = 1000

Server Position = 500
```

客户端最终回到：

```text
500
```

这就是：

```text
Correction
```

修正。

---

# 三者关系

Prediction：

```text
客户端先执行
```

Authority：

```text
服务器裁决
```

Correction：

```text
修正错误结果
```

完整流程：

```text
Input

↓

Prediction

↓

Authority

↓

Correction
```

---

# Demo 中的 Prediction

本课 Demo：

```text
Client Input

↓

Client Local Move

↓

Server RPC

↓

Server Move

↓

Compare Result

↓

Correction
```

这是一个最小化实现。

目的：

```text
理解 Prediction 原理
```

而不是：

```text
实现完整移动系统
```

---

# Demo 缺少什么

真实项目中的 Prediction。

远比 Demo 复杂。

因为需要解决：

```text
高延迟

丢包

连续输入

网络抖动

作弊
```

等问题。

---

# 1. 输入缓存

Demo：

```text
只处理当前输入
```

真实项目：

```text
保存大量历史输入
```

例如：

```text
Frame1

Frame2

Frame3

Frame4

Frame5
```

全部缓存。

---

# 为什么需要输入缓存

服务器可能告诉客户端：

```text
第三帧预测错误
```

此时客户端需要：

```text
回到第三帧

↓

重新执行

第四帧

第五帧

第六帧
```

否则无法恢复正确状态。

---

# 2. 输入重放

客户端收到服务器修正后。

需要：

```text
回滚

↓

重新模拟
```

流程：

```text
服务器修正

↓

回到正确位置

↓

重新执行缓存输入
```

这就是：

```text
Input Replay
```

输入重放。

---

# 3. 时间戳同步

Demo：

```text
直接发送数据
```

真实项目：

```text
输入

+

时间戳
```

服务器必须知道：

```text
哪个输入

对应哪个时间
```

否则无法验证。

---

# 4. 平滑修正

Demo：

```text
直接拉回
```

表现：

```text
瞬移
```

CharacterMovement：

```text
平滑修正
```

表现：

```text
慢慢回到正确位置
```

玩家不容易察觉。

---

# 5. 丢包处理

Demo：

```text
默认消息一定送达
```

真实网络：

```text
可能丢包
```

Prediction 系统需要处理：

```text
重发

确认

补偿
```

机制。

---

# 6. 防作弊验证

客户端可能修改代码：

```text
MoveSpeed = 999999
```

服务器必须：

```text
重新模拟

验证结果
```

否则任何人都能作弊。

---

# CharacterMovement 做了什么

CharacterMovement 并不是：

```text
移动同步组件
```

而是：

```text
完整移动预测系统
```

内部已经实现：

- Prediction
- Authority
- Correction
- 输入缓存
- 输入重放
- 时间戳同步
- 平滑修正
- 防作弊验证

因此：

```text
CharacterMovement
=
工业级 Prediction 系统
```

---

# GAS 中的 Prediction

GAS 也使用 Prediction。

例如：

```text
释放技能
```

客户端：

```text
立即播放动画

立即播放特效

立即播放音效
```

同时：

```text
请求服务器
```

服务器确认：

```text
继续执行
```

服务器拒绝：

```text
撤销预测结果
```

流程本质与 CharacterMovement 一样：

```text
Prediction

↓

Authority

↓

Correction
```

区别只是：

CharacterMovement 预测：

```text
移动
```

GAS 预测：

```text
技能
```

---

# 实际开发如何使用

大多数项目：

角色移动：

```text
CharacterMovement
```

技能系统：

```text
GAS
```

普通状态同步：

```text
Replication
```

普通交互：

```text
RPC + Replication
```

一般不会自己实现完整 Prediction。

课程中的 Demo 只是帮助理解：

```text
Prediction

↓

Authority

↓

Correction
```

这套核心思想。

---

# 本课总结

Prediction：

```text
客户端先执行
```

Authority：

```text
服务器决定最终结果
```

Correction：

```text
服务器修正预测错误
```

三者关系：

```text
Prediction

↓

Authority

↓

Correction
```

Demo：

```text
教学版 Prediction
```

CharacterMovement：

```text
工业级移动 Prediction
```

GAS：

```text
工业级技能 Prediction
```

最终结论：

```text
CharacterMovement
=
Prediction
+
Authority
+
Correction

GAS
=
Prediction
+
Authority
+
Correction
```