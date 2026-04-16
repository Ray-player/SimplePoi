#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimplePoi/Core/PoiEventSubsystem.h"
#include "PoiActor.generated.h"

class UWidgetComponent;
class UCameraComponent;
class USpringArmComponent;
class UPoiBaseUserWidget;

UCLASS()
class SIMPLEPOI_API APoiActor : public AActor, public IPoiActorInterface
{
	GENERATED_BODY()
private:
	UPROPERTY(Category = "Actor", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(Category = "Actor", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(Category = "Actor", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> Widget;
public:
	APoiActor();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi",meta=(DisplayName="相机臂长"))
	float SetArmLength;
	// 所属组,用于批量操作(为空时，可自由设置key值)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi",meta=(AllowPrivateAccess = "true",ExposeOnSpawn))
	FString Group;
	// 聚焦信息结构体
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi",meta=(AllowPrivateAccess = "true",ExposeOnSpawn))
	FFocusMessageStruct FocusMessage;
	// 射线检测距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi")
	int32 LineTraceDistance;
	/** 显示的Widget索引 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi|Widget")
	int32 WidgetTypeIndex;
	// 是否使所有Widget使用相同文本,若使用则传入聚焦Name作为文本
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poi|Widget")
	bool bUseSameText;
	//需要的UI类映射
	UPROPERTY(Category = "Poi|Widget", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<UPoiBaseUserWidget>, FWidgetDataStruct> WidgetSettingMap;
	//当前widget实例
	UPROPERTY(Category = "Poi|Widget", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPoiBaseUserWidget> MyWidgetInstance;
	//是否正在切换Widget状态
	UPROPERTY(Category = "Poi|Boolean",VisibleAnywhere,BlueprintReadWrite)
	bool bInChangeWidgetState = false;
	UPROPERTY(Category = "Poi|Boolean",VisibleAnywhere,BlueprintReadWrite)
	bool bIsSelected = false;
	UPROPERTY(Category = "Poi|Boolean",VisibleAnywhere,BlueprintReadWrite)
	bool bIsVisited = false;
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	//获取widget实例,不存在则根据TypeIndex创建
	UFUNCTION(Category = "Function", BlueprintCallable)
	UPoiBaseUserWidget* GetMyWidget();
	//根据索引获取Widget类
	UFUNCTION(Category = "Function", BlueprintPure)
	TSubclassOf<UPoiBaseUserWidget> GetWidgetClassByIndex(int Index) const;
	//构造初始数据
	UFUNCTION(Category = "Function", BlueprintNativeEvent,BlueprintCallable)
	void ConstructInitialData();
	//当Widget动画播放完成时调用
	UFUNCTION(Category = "Function",BlueprintNativeEvent, BlueprintCallable)
	void OnWidgetAniPlayedCallback(const FString& AniName,const bool bIsForwardAni, UPoiBaseUserWidget* InWidget);
	//选中时改变Widget笔刷
	UFUNCTION(Category = "Function", BlueprintCallable, meta =(DisplayName="选中时改变Widget笔刷", Keywords="PoiFunc"))
	void ChangeWidgetBrushOnSelect(bool bIsPoiSelected);
	//摄像机臂长设为0,直接将PoiActor位置设置为视口位置,旋转设置为视口旋转
	UFUNCTION(Category = "Poi", BlueprintCallable,meta=(CallInEditor = "true", DisplayName="直接对齐到当前视口"))
	void AlignDirectlyScreenLens();
	//从视口发出射线(可视性碰撞通道)将PoiActor位置设置为射线命中位置,旋转设置为视口旋转,相机臂长设置为射线长度
	UFUNCTION(Category = "Poi", BlueprintCallable,meta=(CallInEditor = "true", DisplayName="射线对齐到当前视口"))
	void AlignTraceNowScreenLens();
	//当前PoiActor位置不变,调整Actor旋转,Camera组件旋转和相机臂长使其对齐当前视口
	UFUNCTION(Category = "Poi", BlueprintCallable,meta=(CallInEditor = "true", DisplayName="固定对齐到当前视口"))
	void AlignFixLocNowScreenLens();
	UFUNCTION()
	void UpdateFocusMessageOnNextTick();
	
	//需要在蓝图中重写EWidgetOnClicked,ESelected,EUnSelected
	virtual void BindButtonEvent_Implementation(UButton* InButton) override;
	virtual void ChangeWidgetType_Implementation(int InIndex) override;
	virtual bool GetGroupAndKey_Implementation(FString& OutGroup,FString& OutKey) const override;
	virtual FFocusMessageStruct GetFocusMessage_Implementation() const override { return FocusMessage; }
	
	virtual void SetPoiVisitByWidgetAni_Implementation(bool bIsVisitForward, float InDelayTime) override;
	virtual void SetPoiVisitByProperties_Implementation(bool bIsVisit, float InDelayTime) override;
	
	virtual void EOnHovered_Implementation() override;
	virtual void EUnHovered_Implementation() override;

	virtual void EWidgetOnClicked_Implementation() override;
	//virtual void ESelected_Implementation() override;
	//virtual void EUnSelected_Implementation() override;
	
};
