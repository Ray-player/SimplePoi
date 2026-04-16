#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PoiEventSubsystem.generated.h"

class APoiBasePawn;
class UButton;
//POI消息结构体
USTRUCT(BlueprintType)
struct FPoiMessageStruct
{
	GENERATED_BODY()
public:
	FPoiMessageStruct()
	{
		KeyName = TEXT("");
		DataMap = {{TEXT("text"),TEXT("")}};
	}

	FPoiMessageStruct(FString InKeyName, FString InText)
	{
		KeyName = InKeyName;
		DataMap = {{TEXT("text"), InText}};
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> DataMap;
};
// 实现等于运算符
FORCEINLINE bool operator==(const FPoiMessageStruct& A, const FPoiMessageStruct& B)
{
	if (A.KeyName != B.KeyName)
	{
		return false;
	}

	if (A.DataMap.Num() != B.DataMap.Num())
	{
		return false;
	}

	for (const auto& Pair : A.DataMap)
	{
		if (!B.DataMap.Contains(Pair.Key) || B.DataMap[Pair.Key] != Pair.Value)
		{
			return false;
		}
	}

	return true;
}
// 实现GetTypeHash函数
FORCEINLINE uint32 GetTypeHash(const FPoiMessageStruct& Key)
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(Key.KeyName));

	// 对DataMap中的所有键值对生成哈希
	for (const auto& Pair : Key.DataMap)
	{
		Hash = HashCombine(Hash, GetTypeHash(Pair.Key));
		Hash = HashCombine(Hash, GetTypeHash(Pair.Value));
	}

	return Hash;
}

//文本数组结构体
USTRUCT(BlueprintType)
struct FTextArrayStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(MultiLine="true"))
	FText TitleText;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta=(MultiLine="true"))
	TArray<FText> ChildrenText;
};

//聚焦句柄结构体
USTRUCT(Blueprintable)
struct FFocusHandleSet
{
	GENERATED_BODY()

public:
	//每秒钟更新次数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UpdateTimesEverySec = 60;
	//聚焦已用时
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UsedTime = 0.0f;
	//聚焦曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRuntimeFloatCurve FocusCurve;
};

/**
 * 聚焦信息结构体，Pawn根据该结构体信息使摄像机聚焦到对应点
 */
USTRUCT(Blueprintable)
struct FFocusMessageStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="聚焦名称"))
	FString Name = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="是否瞬移"))
	bool bIsTeleport = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="目标位置"))
	FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="目标旋转"))
	FRotator TargetRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="目标摄像机臂长"))
	float TargetSprintLens = -1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="聚焦动画用时"))
	float FocusTimes = 1.0f;
};

/**
 * Pawn相机控制相关的设置
 * 包含了Pawn相机控制相关的设置，如鼠标运动模式、是否可以通过鼠标移动、是否通过键盘移动、是否可以通过鼠标旋转、是否开启滚轮缩放功能等。
 */
USTRUCT(BlueprintType, meta=(ToolTip="Pawn相机控制相关的设置"))
struct FControlSettingStruct
{
	GENERATED_BODY()

public:
	//以鼠标点击为中心运动还是以Pawn自身为中心运动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="鼠标运动模式"))
	bool bMoveByMouseClick = true;
	//是否可以通过鼠标移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="鼠标移动"))
	bool bCanMouseMove = true;
	//是否通过键盘移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="键盘移动"))
	bool MoveByKeyboard = true;
	//是否可以通过鼠标旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="鼠标旋转"))
	bool bCanRotate = true;
	//是否开启滚轮缩放功能
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="滚轮缩放"))
	bool bCanScale = true;
	//任意点聚焦功能,默认为鼠标双击聚焦
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="任意点聚焦"))
	bool bAnyWhereCanFocus = true;

	//移动受鼠标点击位置的灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="鼠标移动灵敏度"))
	float MoveSensitivity = 10.f;
	//按键移动的灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="按键移动灵敏度"))
	float KeyMoveSensitivity = 20.f;
	//旋转灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="旋转灵敏度"))
	float RotateSensitivity = 1.f;
	//缩放灵敏度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="缩放灵敏度"))
	float ScaleSensitivity = 2.f;

	//旋转俯仰角(Pitch)限制,x为最小值,y为最大值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="旋转俯仰角限制"))
	FVector2D PitchLimit = FVector2D(-89.9f, 5.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="缩放距离限制"))
	FVector2D ScaleLimit = FVector2D(300.f, 40000.f);

	//初始聚焦信息(若Name不为空,会在BeginPlay中将位置旋转设置为该值)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName="初始聚焦信息"))
	FFocusMessageStruct InitialFocusMessage;
};

/**
 * 界面信息结构体，Widget类根据该结构体信息显示界面内容
 */
USTRUCT(Blueprintable)
struct FWidgetDataStruct
{
	GENERATED_BODY()

public:
	//控件UI整体缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D WidgetSize = FVector2D(1.f, 1.f);
	//控件文本内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WidgetText = FText::FromString(TEXT(""));
	//控件文本字体
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateFontInfo WidgetFont;
	//控件可调整的外轮廓padding
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin WidgetPadding;
	//控件笔刷
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateBrush Brush;
	//是否存在选中时的笔刷样式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSelectedBrush = false;
	//选中时的笔刷样式
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bHasSelectedBrush", EditConditionHides))
	FSlateBrush SelectedBrush;
	//光标悬停时是否使用选中样式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HoveredUseSelectedBrush = false;
	//控件动画速度映射
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> AniSpeedMap = {
		{TEXT("HoveredAnimation"), 3.f},
		{TEXT("VisitAnimation"), 1.5f}
	};
	//其他信息映射值
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> OtherMessageMap;
};

//用于子系统存储PoiActor数据
USTRUCT(Blueprintable)
struct FPoiMapData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FString, TObjectPtr<AActor>> PoiActorKeyMap;
};

/*POI接口声明*/
UINTERFACE(Blueprintable)
class SIMPLEPOI_API UPoiActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * POI接口声明
 */
class IPoiActorInterface
{
	GENERATED_BODY()

public:
	// 绑定按钮对应的事件 - 可在蓝图中重写
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "0"))
	void BindButtonEvent(UButton* InButton);
	//使用widget可视性动画变更该Poi的可视性
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "1"))
	void SetPoiVisitByWidgetAni(bool bIsVisitForward = true, float InDelayTime = 0.f);
	//通过属性设置该Poi的可视性,无动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "1"))
	void SetPoiVisitByProperties(bool bIsVisit = true, float InDelayTime = 0.f);
	//切换Widget的类型
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "2"))
	void ChangeWidgetType(int InIndex);
	//聚焦到自己的方法
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "2"))
	void FocusToSelf();
	//获取该PoiActor所属的分组和Key,如果不存在则返回false
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "2"))
	bool GetGroupAndKey(FString& OutGroup, FString& OutKey) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface", meta = (DisplayPriority = "2"))
	FFocusMessageStruct GetFocusMessage() const;
	//点击widget时触发的事件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "12"))
	void EWidgetOnClicked();
	//选中时触发的事件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "13"))
	void ESelected();
	//移除选中时触发的事件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "14"))
	void EUnSelected();
	//鼠标移入widget时触发的事件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "15"))
	void EOnHovered();
	//鼠标移出widget时触发的事件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poi Interface|ButtonEvent",
		meta = (DisplayPriority = "16"))
	void EUnHovered();
};

UCLASS(Blueprintable,meta = (ShowWorldContextPin))
class SIMPLEPOI_API UPoiActionSet : public UObject
{
	GENERATED_BODY()

public:
	//单个操作消息还是多个操作消息
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSingleMessage = true;
	//响应的操作消息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsSingleMessage", EditConditionHides))
	FPoiMessageStruct ActMessage = FPoiMessageStruct();
	//可响应的多个操作消息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bIsSingleMessage", EditConditionHides))
	TArray<FPoiMessageStruct> ActMessages = {FPoiMessageStruct()};
	//获取外部对象作为Actor
	UFUNCTION(BlueprintPure,Category = "Poi Action")
	AActor* GetOuterAsActor();
	UFUNCTION(BlueprintPure,Category = "Poi Action")
	UObject* GetOuterObject();
	//执行操作
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Poi Action")
	void ExecuteAction(const FGameplayTag& CommandTag);
	//执行操作完成后的事件
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Poi Action")
	void OnActionExecuted(const FGameplayTag& CommandTag);
	//绑定Poi子系统委托
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Poi Action", meta = (DisplayName="BindFunction"))
	void ActionBindPoiSubsystemDelegate();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReceivedActorStrByTag, AActor*, CurActor, FGameplayTag, CommandTag,
                                               const FPoiMessageStruct&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReceivedObjectStrByTag, UObject*, CurObject, FGameplayTag, CommandTag,
                                               const FPoiMessageStruct&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReceivedActorSimpleStrByTag, AActor*, CurActor, FGameplayTag,
                                               CommandTag, const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReceivedObjectSimpleStrByTag, UObject*, CurObject, FGameplayTag,
                                               CommandTag, const FString&, Message);

/**
 *  POI事件子系统,通过游戏实例子系统处理相关事件消息
 */
UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="Poi事件管理子系统"))
class SIMPLEPOI_API UPoiEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//当前选中的Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PoiSubSystem|Data")
	TObjectPtr<AActor> CurrentSelectedActor;
	//场景中的PoiBasePawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PoiSubSystem|Data")
	TObjectPtr<APoiBasePawn> PoiBasePawn;
	//焦点消息映射数据,用于存储无UI的Poi数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PoiSubSystem|Data")
	TMap<FString, FFocusMessageStruct> FocusMessageMap;
	//PoiActor分组映射数据,用于存储多个PoiActor
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PoiSubSystem|Data")
	TMap<FString, FPoiMapData> PoiActorGroupMap;
	//POI操作映射数据,用于存储多个PoiActor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PoiSubSystem|Data")
	TMap<FPoiMessageStruct, TObjectPtr<UPoiActionSet>> ActionSetMap;
	UFUNCTION(BlueprintPure, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="获取PoiPawn"))
	APoiBasePawn* GetPoiPawn();
	//设置当前选中的Actor,并触发选中事件
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="设置当前选中的Actor"))
	void SetCurrentSelectedActor(AActor* InActor);
	//直接将聚焦数据添加到FocusMessageMap中
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="添加聚焦数据到Map"))
	void AddFocusDataToMap(const FFocusMessageStruct& FocusData);
	/**
	* 添加PoiActor到分组管理系统中
	* 
	* @param InActor 要添加到分组的Actor对象，必须包含有效的Poi相关数据
	* @param bIsAddToMessageMap 是否同时将该Actor的聚焦数据添加到焦点消息映射中
	*                           - true:  将Actor的聚焦数据存储到FocusMessageMap中
	*                           - false: 用于无UI的Poi数据管理,仅添加到PoiActorGroupMap分组中（默认值）
	*/
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="添加PoiActor到Map"))
	void AddPoiActorToGroup(AActor* InActor, const bool bIsAddToMessageMap = false);
	//查找整组Actor
	UFUNCTION(BlueprintPure, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="查找整组PoiActor"))
	void FindAllActorInGroup(const FString& InGroup, TArray<AActor*>& OutActorArray);
	//通过分组和Key查找PoiActor
	UFUNCTION(BlueprintPure, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="查找指定PoiActor"))
	AActor* FindPoiActorByGroupAndKey(const FString& InGroup, const FString& InKey);
	//通过分组和Key查找聚焦数据(优先从无实例数据中查找)
	UFUNCTION(BlueprintPure, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="查找指定聚焦数据"))
	bool FindFocusMessageByGroupAndKey(const FString& InGroup, const FString& InKey,
	                                   FFocusMessageStruct& OutFocusMessage);

	//通过数据聚焦到目标(通过委托传递聚焦数据)
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="通过数据聚焦"))
	void BroadcastFocusMessageByDelegate(const FFocusMessageStruct& FocusData);
	//通过Group(分组)和Key(通常为聚焦名称)聚焦到目标
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="通过关键字聚焦"))
	bool FocusToActorByKey(const FString& InGroup, const FString& InKey);

	//使用Actor的聚焦数据聚焦到目标(通过Actor的接口调用)
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|PoiFunction", meta=(DisplayName="通过Actor聚焦"))
	void FocusToActorByFocusMessage(AActor* InActor);

	UPROPERTY(BlueprintAssignable, Category="PoiSubSystem|Delegate", meta=(DisplayName="接收Actor字符串消息"))
	FOnReceivedActorStrByTag ReceivedActorStrByTag;
	UPROPERTY(BlueprintAssignable, Category="PoiSubSystem|Delegate", meta=(DisplayName="接收Object字符串消息"))
	FOnReceivedObjectStrByTag ReceivedObjectStrByTag;
	UPROPERTY(BlueprintAssignable, Category="PoiSubSystem|Delegate", meta=(DisplayName="接收Actor简单字符串消息"))
	FOnReceivedActorSimpleStrByTag ReceivedActorSimpleStrByTag;
	UPROPERTY(BlueprintAssignable, Category="PoiSubSystem|Delegate", meta=(DisplayName="接收Object简单字符串消息"))
	FOnReceivedObjectSimpleStrByTag ReceivedObjectSimpleStrByTag;


	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|MesFunction", meta=(DisplayName="发送Actor字符串消息"))
	void SendActorStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage);
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|MesFunction", meta=(DisplayName="发送Object字符串消息"))
	void SendObjectStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FPoiMessageStruct& InMessage);
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|MesFunction", meta=(DisplayName="发送Actor简单字符串消息"))
	void SendActorSimpleStrByTag(AActor* CurActor, FGameplayTag CommandTag, const FString& InMessage);
	UFUNCTION(BlueprintCallable, Category="PoiSubSystem|MesFunction", meta=(DisplayName="发送Object简单字符串消息"))
	void SendObjectSimpleStrByTag(UObject* CurObject, FGameplayTag CommandTag, const FString& InMessage);

	//Json字符串转为结构体数据
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Json")
	static bool JsonStringToPoiStruct(const FString JsonString,
	                                  UPARAM(DisplayName="FocusData")
	                                  FFocusMessageStruct& OutStruct);
	//结构体数据转为Json字符串
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Json")
	static bool PoiStructToJsonString(const FFocusMessageStruct& InStruct,
	                                  UPARAM(DisplayName="JsonString")
	                                  FString& OutJsonString);

	//将FocusMessageMap序列化为JSON字符串
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Json")
	FString FocusMessageMapToJsonString();
	//将PoiActorGroupMap序列化为JSON字符串
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Json")
	FString PoiActorGroupMapToJsonString();


	//根据AniKeyCut查找WidgetData中对应的动画速度
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Data")
	static bool FindAniSpeed(const FString& AniKeyCut, const FWidgetDataStruct& WidgetData, float& OutSpeed);
	//创建POI消息结构体
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Data")
	static FPoiMessageStruct CreatePoiMessageStruct(const FString& InKeyName, FText InText)
	{
		return FPoiMessageStruct(InKeyName, *InText.ToString());
	}
	//POI消息结构体是否相同
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Data")
	static bool PoiMessageStructIsEqual(const FPoiMessageStruct& A, const FPoiMessageStruct& B)
	{
		return A == B;
	}
	//字符串转Base64
	UFUNCTION(BlueprintPure, Category="PoiCommonFunction|Data")
	static FString ReadFileAsBase64(const FString& Str)
	{
		TArray<uint8> FileData;
		if (FFileHelper::LoadFileToArray(FileData, *Str))
		{
			// 转换为Base64
			return FBase64::Encode(FileData);
		}
		return TEXT("");
	}
};
