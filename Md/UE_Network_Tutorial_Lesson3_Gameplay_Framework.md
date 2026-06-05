# UE C++ 联机教程03：Gameplay Framework 网络模型

## 课程目标
- 理解多人联机中核心对象的设计逻辑
- 明确Gameplay功能与网络约束的关系
- 为后续 Ownership、RPC、Replication、移动同步打基础

---

## 核心认知
1. 游戏设计首先考虑功能：谁触发、谁修改、谁表现
2. 网络约束是实现多人一致性的条件，不是设计本身
3. Gameplay类选择与职责应服务于功能，而不是单纯为了网络分布
4. 同一个功能可能涉及多个类：GameMode / GameState / PlayerState / PlayerController / Pawn / Character

---

## 第一部分：GameMode
- 代表游戏规则
- 示例：玩家如何出生、什么时候胜利、比赛是否开始
- 设计逻辑：规则放GameMode，客户端不可修改
- 类比：裁判只在服务器，客户端无法直接操作

## 第二部分：GameState
- 代表公共状态
- 示例：比赛倒计时、波次、Boss阶段
- 设计逻辑：服务器修改，Replication同步给所有客户端
- 类比：公告板，所有玩家都能看到

## 第三部分：PlayerController
- 代表玩家的意志和输入
- 每个客户端只拥有自己的PlayerController
- 用于捕获输入、发送RPC请求、操作本地UI
- 类比：玩家的控制器，负责下达指令

## 第四部分：PlayerState
- 代表玩家身份和长期信息
- 示例：名字、等级、击杀数
- 服务器修改，Replication同步给所有客户端
- 类比：玩家名片，所有人都能看到

## 第五部分：Pawn
- 可被控制的实体
- 示例：角色、汽车、无人机
- PlayerController通过Possess控制Pawn
- 用于分离控制者和被控制对象

## 第六部分：Character
- 特殊Pawn，内置CharacterMovementComponent
- 适合人形角色
- 示例：玩家角色、NPC、怪物

---

## 最终总结

| 类 | 本质 |
|------|------|
| GameMode | 游戏规则 |
| GameState | 游戏状态 |
| PlayerController | 玩家意志/输入 |
| PlayerState | 玩家身份/长期信息 |
| Pawn | 可控制实体 |
| Character | 人形Pawn，带移动组件 |

---

## 设计原则总结
1. Gameplay先于网络设计：先明确功能逻辑，再考虑网络同步约束
2. 类职责明确：GameMode → 规则；GameState →全局状态；PlayerState →玩家信息；PlayerController →输入；Pawn/Character →角色状态
3. 网络约束只影响数据同步：RPC和Replication保证客户端看到一致结果
4. 可直观演示：开门、掉血、拾取、建造等功能，看到效果比打印日志更直观
