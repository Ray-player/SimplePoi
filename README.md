# SimplePoi 插件使用说明

## 一、插件概述

**SimplePoi** 是一个用于虚幻引擎5的视点控制与POI（Point of Interest）标记系统插件，提供摄像机控制、数据驱动UI组件和事件管理功能。

### 基本信息

- **版本**：1.0
- **引擎版本要求**：UE 5.5+（蓝图由5.5版本编辑器创建，暂不支持低版本）
- **依赖插件**：EnhancedInput

### 核心功能

- 摄像机视点控制与聚焦
- POI点标记系统
- 数据驱动的UI组件
- 事件消息通信系统
- UI动画播放系统

### 示例场景

![bluer5](/Resources/bluer5.png)

---

## 二、核心类与组件

### 1. APoiActor（POI标记Actor）

POI点标记的核心类，用于在场景中创建可交互的标记点。

**主要属性：**

- `Group`：所属组名，用于批量操作
- `FocusMessage`：聚焦信息结构体（位置、旋转、臂长等）
- `WidgetTypeIndex`：显示的Widget索引
- `WidgetSettingMap`：UI类映射配置

**关键方法：**

- `GetMyWidget()`：获取或创建Widget实例
- `ChangeWidgetType()`：切换Widget类型
- `FocusToSelf()`：聚焦到自身位置
- `SetPoiVisitByWidgetAni()`：通过动画设置可视性
- `EWidgetOnClicked()`：点击事件
- `ESelected/EUnSelected()`：选中/取消选中事件

### 2. UPoiEventSubsystem（事件管理子系统）

游戏实例子系统，负责管理POI数据、事件和消息传递的中心。

**主要数据存储：**

- `FocusMessageMap`：聚焦数据映射（无UI的POI数据）
- `PoiActorGroupMap`：POI Actor分组管理
- `ActionSet`：操作数据,通过子类继承并在 `PDA_PrimaryMainUIdata`数据资产中设置应用对应内容

**关键功能：**

- `AddPoiActorToGroup()`：添加POI到分组
- `FindPoiActorByGroupAndKey()`：通过分组和Key查找POI
- `FocusToActorByKey()`：通过关键字聚焦
- `BroadcastFocusMessageByDelegate()`：广播聚焦消息
- `SendActorStrByTag()`：发送Actor消息

### 3. APoiBasePawn（摄像机控制Pawn）

提供摄像机运动控制和聚焦功能的基础Pawn类。

**控制设置（FControlSettingStruct）：**

- `bMoveByMouseClick`：鼠标点击为中心运动
- `bCanMouseMove`：鼠标移动
- `MoveByKeyboard`：键盘移动
- `bCanRotate`：鼠标旋转
- `bCanScale`：滚轮缩放
- `bAnyWhereCanFocus`：任意点聚焦（双击）
- 灵敏度参数：MoveSensitivity、RotateSensitivity、ScaleSensitivity
- 角度限制：PitchLimit、ScaleLimit

**关键方法：**

- `FocusDataToTargetByHandle()`：使用聚焦数据移动摄像机
- `AddInputActionBindEvent()`：添加输入事件绑定

### 4. UPoiBaseUserWidget（基础UI控件）

提供UI动画播放和数据绑定的基础Widget类。

**关键功能：**

- `PlayAnimationByDirKey()`：按关键字播放动画
- `PlayVisitAni()`：播放访问动画
- `PlayHoveredAni()`：播放悬停动画
- `InitWidgetByStruct()`：通过结构体初始化控件
- `OnAnimationPlayed`：动画播放完成委托

---

## 三、数据结构

### FFocusMessageStruct（聚焦信息）

```cpp
- Name：聚焦名称
- bIsTeleport：是否瞬移
- TargetLocation：目标位置
- TargetRotation：目标旋转
- TargetSprintLens：目标摄像机臂长
- FocusTimes：聚焦动画用时
```

### FWidgetDataStruct（UI数据）

```cpp
- WidgetSize：控件缩放
- WidgetText：控件文本
- WidgetFont：控件字体
- WidgetPadding：外轮廓padding
- Brush：控件笔刷
- SelectedBrush：选中笔刷
- AniSpeedMap：动画速度映射
- OtherMessageMap：其他信息映射
```

### FPoiMessageStruct（消息结构）

```cpp
- KeyName：键名
- DataMap：数据映射（包含text等字段）
```

---

## 四、使用流程

### 1. 基础设置

1. **启用插件**：在项目设置中启用SimplePoi和EnhancedInput插件
2. **配置GameMode**：使用 `BP_DemoGameMode`或继承 `APoiBasePawn`的自定义Pawn
3. **配置输入**：使用提供的输入映射（`IMC_PoiBaseInput`）

### 2. 创建POI点

1. 在场景中放置 `BP_BasePoi`或继承 `APoiActor`的蓝图
2. 配置 `FocusMessage`设置聚焦参数
3. 设置 `Group`和 `WidgetTypeIndex`
4. 配置 `WidgetSettingMap`设置UI样式

### 3. 摄像机控制

1. 使用 `APoiBasePawn`或 `BP_MainPawn`作为玩家Pawn
2. 在 `PawnData`中配置控制参数
3. 通过 `PoiEventSubsystem`调用聚焦功能：
   - `FocusToActorByKey(Group, Key)`
   - `FocusToActorByFocusMessage(Actor)`

### 4. UI交互

1. 创建继承 `UPoiBaseUserWidget`的自定义Widget
2. 实现接口方法（`GetNeedBindButton`、`GetNeedTextBlock`等）
3. 在 `PoiActor`中配置Widget映射
4. 通过事件系统处理交互（点击、悬停、选中）

### 5. 事件通信

使用 `PoiEventSubsystem`的消息系统：

```cpp
// 发送消息
void SendActorStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage);
void SendObjectStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage);
void SendActorSimpleStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FString& InMessage);
void SendObjectSimpleStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FString& InMessage);

// 接收消息（绑定委托）
ReceivedActorStrByTag.AddDynamic(this, &MyClass::OnReceiveMessage);
```

---

## 五、GameplayTags配置

插件预定义的GameplayTags（在 `Config/Tags/PoiPluginTags.ini`中）：

- `SimplePoiPlugin.WidgetAction.Click`：UMG按钮点击
- `SimplePoiPlugin.WidgetAction.Enable`：UMG启用
- `SimplePoiPlugin.WidgetAction.Disable`：UMG禁用
- `SimplePoiPlugin.WidgetAction.Change`：UMG切换
- `SimplePoiPlugin.PoiAction.Selected`：POI组件选中
- `SimplePoiPlugin.PoiAction.Focus`：POI组件聚焦

蓝图调用示例：  
- 事件子系统的一些常用方法  

![bluer0](/Resources/bluer0.png)  

- 其他方法在右键菜单中的分类  

![bluer1](/Resources/bluer1.png)  

---

## 六、资源说明

### 蓝图类

- `BP_BasePoi`：基础POI Actor
- `BP_MainPawn`：主控Pawn
- `BP_DemoGameMode`：演示GameMode
- `WBP_MainUI`：主UI
- `WBP_PoiBase`：基础POI Widget

### 输入配置

- `IMC_PoiBaseInput`：基础输入映射
- `IA_KeyMove`：键盘移动
- `IA_MouseClickL/R`：鼠标左右键
- `IA_MouseMove`：鼠标移动
- `IA_WheelRoll`：滚轮滚动

### 数据资产

- `DA_MainInit`：主初始化数据
- `DA_MainDataAsset`：主UI数据
- `C_FocusCurve`：聚焦运动曲线

### 演示场景

- `DemoMap.umap`：演示地图

---

## 八、注意事项

1. **引擎版本**：必须使用UE 5.5或更高版本
2. **依赖插件**：确保EnhancedInput插件已启用
3. **蓝图兼容性**：蓝图由5.5创建，低版本无法打开
4. **性能优化**：大量POI时注意分组管理和按需加载
5. **动画系统**：确保Widget动画命名规范，便于关键字匹配
6. **输入冲突**：注意输入映射与其他系统的冲突

---

## 九、扩展开发

### 自定义POI Actor

继承 `APoiActor`并重写以下方法：

- `ConstructInitialData()`：初始化数据
- `BindButtonEvent()`：绑定按钮事件
- `EWidgetOnClicked()`：点击事件处理
- `EOnHovered()`悬停事件和 `ESelected`选中事件和对应取消事件处理

### 自定义Widget

场景中的Widget通过继承 `UPoiBaseUserWidget`并实现：

- `GetNeedBindButton()`：返回需要绑定的按钮
- `GetNeedTextBlock()`：返回文本组件
- `PlayAnimationByDirKey()`：自定义动画播放逻辑

面板上的全局Widget控件：

- 参考 `WBP_MainUI`实现,或继承 `IPoiWidgetInterface`接口自行实现
- 关于主控件 `WBP_MainUI` 的使用与继承请结合蓝图Pawn类 `BP_MainPawn` 参考

### 自定义数据资产

继承 `PoiSettingsPDA` 参考子蓝图自行创建：

- 分离UI的数据与功能,数据在PDA中配置,功能在 `ActionSet` 中实现
- `DA_MainDataAsset` 为纯数据资产,不包含任何逻辑,也可以参考开发额外的PDA
- 公有事件类 `SpecialActionSet` 通过 `BP_MainPawn`加载并注册,并在自身函数 `BindFunction` 中实现Poi事件系统绑定
- 特有事件类 `SpecialActionSet` 通过 `WBP_MainUI` 调用公共宏  `CallPoiAction` 绑定Poi事件系统
- `DA_MainDataAsset`配置示例：
![bluer2](/Resources/bluer2.png)
- 公共Action如何加载(在pawn初始化中调用)：
![bluer4](/Resources/bluer4.png)
- Acton简单重写示例(以下为摄像机聚焦公共Action)：
![bluer3](/Resources/bluer3.png)

---

## 十、技术支持

如需更多技术细节，请参考：

- 源代码：`Source/SimplePoi/`
- 示例蓝图：`Content/Blueprints/`
- 演示场景：`Content/DemoMap.umap`
