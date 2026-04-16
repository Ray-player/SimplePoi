#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimplePoi/Core/PoiEventSubsystem.h"
#include "PoiBaseUserWidget.generated.h"

class APoiActor;
class UButton;
class UTextBlock;
class UOverlay;
class UPoiBaseUserWidget;


/*POI控件接口声明*/
UINTERFACE(Blueprintable)
class SIMPLEPOI_API UPoiWidgetInterface : public UInterface
{
	GENERATED_BODY()
};
class IPoiWidgetInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category = "Poi Interface|Widget")
	void SetWidgetMessageMap(const TMap<FString,FText>& Mes);
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category = "Poi Interface|Widget")
	void SetWidgetPropertyMap(const TMap<FString,FString>& Mes);
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category = "Poi Interface|Widget")
	UObject* GetPannalObject();
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category = "Poi Interface|Widget")
	UPrimaryDataAsset* GetPrimaryDataAsset();
};

// 事件分发器声明,动画完成时委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAnimationPlayed, const FString&, AnimKey,
                                               const bool, bIsForwardAni, UPoiBaseUserWidget*, PoiWidget);

/**
 * 基础POI用户控件类,提供UI动画播放功能
 */
UCLASS(Blueprintable)
class SIMPLEPOI_API UPoiBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
	friend class APoiActor;
	TMap<TObjectPtr<UWidgetAnimation>, bool> AniEventBoundedMap;
	bool bCurIsForwardAni;
	FString CurAniKey;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data",meta=(AllowPrivateAccess = "true",ExposeOnSpawn))
	TObjectPtr<APoiActor> OwnerPoi;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	bool bIsBoundAniEvent = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	TMap<FString, TObjectPtr<UWidgetAnimation>> AnimationDic;
	UPROPERTY(BlueprintAssignable, Category = "Event", meta = (DisplayName = "动画播放完成后"))
	FOnAnimationPlayed OnAnimationPlayed;
	//根据结构体初始化控件数据
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Data")
	void InitWidgetByStruct(APoiActor* InPoiActor,const FWidgetDataStruct& WidgetMes);
	
	/* 手动触发动画播放完成事件*/
	UFUNCTION(BlueprintCallable, Category = "AniFunction")
	void BroadcastOnAniFinished(const FString& AnimKey, const bool bIsForwardAni, UPoiBaseUserWidget* PoiWidget)
	{
		OnAnimationPlayed.Broadcast(AnimKey, bIsForwardAni, PoiWidget);
	}
	//根据属性设置可视性
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "AniFunction")
	void SetVisitByProperties(bool bIsVisit,float InDelayTime);
protected:
	//当前控件数据
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Data")
	FWidgetDataStruct SelfData;
	
	//根据关键字获取动画组件,支持模糊匹配
	UFUNCTION(BlueprintCallable, Category = "AniFunction")
	UWidgetAnimation* GetAnimationComponentByKey(const FString& Key);

	//获取需要绑定的按钮 - C++默认获取第一个按钮,可在蓝图中重写
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction|Pure")
	UButton* GetNeedBindButton() const;
	//获取对应文本组件 - C++默认获取第一个文本组件,可在蓝图重写
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction|Pure")
	UTextBlock* GetNeedTextBlock() const;
	// 获取总的UI控制覆层 - 须在蓝图中重写,否则查找第一个UOverlay组件
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction|Pure")
	UOverlay* GetTolOverlayWidget() const;
	
	UFUNCTION(BlueprintCallable, Category = "AniFunction")
	void SetMyWidgetBrush(const FSlateBrush& NewBrush);
	
public:
	/**
	* 根据动画关键字播放UI动画,需在蓝图中实现
	* @param Key 动画关键字，用于指定要播放的具体动画
	* @param bIsForwardAni 是否正向播放动画，true为正向，false为反向，默认为true
	* @param PlayRate 动画播放速率，默认为1.0表示正常速率，大于1.0加速播放，小于1.0减速播放
	* @param DelayTime 动画播放前的延迟时间（秒），默认为0表示立即播放
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction")
	void PlayAnimationByDirKey(const FString& Key, const bool bIsForwardAni = true, const float PlayRate = 1.f,
	                           const float DelayTime = 0.f);
	void PlayAniFunction(const FString& Key, const float PlayRate, bool bIsForwardAni);
	
	UFUNCTION()
	void OnAniFinished()
	{
		BroadcastOnAniFinished(CurAniKey, bCurIsForwardAni, this);
	}
	//播放访问动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction")
	void PlayVisitAni(const bool bIsForwardAni = true,const float DelayTime = 0.f);
	//播放悬停动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AniFunction")
	void PlayHoveredAni(const bool bIsForwardAni = true,const float DelayTime = 0.f);
};
