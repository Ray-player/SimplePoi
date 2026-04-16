#pragma once

#include "CoreMinimal.h"
#include "PoiBasePawn.h"
#include "PoiMotionPawn.generated.h"

struct FInputActionValue;

UCLASS()
class SIMPLEPOI_API APoiMotionPawn : public APoiBasePawn
{
	GENERATED_BODY()

public:
	APoiMotionPawn();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComplete_Implementation(UInputComponent* InPlayerInputComponent) override;

	//可 BlueprintNativeEvent
	//获取鼠标点击场景中的位置
	UFUNCTION(BlueprintPure, Category="PureGet")
	bool GetMouseLocationInWorldSpace(FVector& OutLocation,FVector& WorldDir);
	//根据鼠标点击位置计算目标位置句柄
	UFUNCTION(BlueprintPure, Category="PureGet")
	bool GetMouseHandleByGeometry(FVector& Intersection);
	//根据鼠标点击位置计算目标位置句柄(射线检测)
	UFUNCTION(BlueprintPure, Category="PureGet")
	bool GetMouseHandleByLineTrace(FHitResult& OutHit,FVector& Intersection,float& Distance);

	//获取弹簧臂偏移位置
	UFUNCTION(BlueprintPure, Category="PureGet")
	FVector GetSpringArmOffsetLoc() const;
	

	//初始化Trigger相关输入事件绑定,绑定到相关函数(KeyMove/MouseMove)
	UFUNCTION(BlueprintCallable, Category="Init")
	bool InitPawnInputTriggerEvent(const FString& MouseMoveActionName,const FString& KeyMoveActionName);
	
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	void OnKeyMove(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	void OnMouseMove(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaTime) override;
	//目标位置句柄
	UPROPERTY(Category = "PoiPawn|Data", EditDefaultsOnly, BlueprintReadWrite)
	FVector TargetHandleLoc;
	//当前缩放长度倍率
	UPROPERTY(Category = "PoiPawn|Data", EditAnywhere, BlueprintReadWrite)
	float CurrentZoomValue;
	UPROPERTY(Category = "PoiPawn|Data", EditDefaultsOnly, BlueprintReadWrite)
	bool bLeftMouseDown;
	UPROPERTY(Category = "PoiPawn|Data", EditDefaultsOnly, BlueprintReadWrite)
	bool bRightMouseDown;
	UPROPERTY(Category = "PoiPawn|Data", EditDefaultsOnly, BlueprintReadWrite)
	bool bTransformationCompleted;
	UPROPERTY(Category = "PoiPawn|Data", EditDefaultsOnly, BlueprintReadWrite)
	FTimerHandle UpdateTimeHandle;

	//位移时x轴方向向量
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite)
	FVector MoveAxisXForward;
	//按键移动时是否根据相机前方向移动
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite)
	bool bKeyMoveForwardAtCameraForward;
	//相机z轴方向长度
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite)
	float CameraZlineLength;
	//射线检测距离
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite)
	int32 LineTraceDistance;
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite)
	bool LineTraceDebug;
	
	//根据输入轴更新移动数据
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	bool UpdateMoveData(FVector2D InputAxis);
	//停止更新移动数据
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category="Update")
	bool StopUpdateMoveData();

	//根据输入轴更新旋转数据
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	bool UpdateRotationData(FVector2D InputAxis);
	//根据输入轴更新缩放数据
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	bool UpdateZoomData(float InputAxis);
	//根据输入轴更新按键移动数据
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Update")
	bool UpdateKeyMoveData(FVector InputAxis);

	

	
};
