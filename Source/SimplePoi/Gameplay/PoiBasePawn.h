#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "SimplePoi/Core/PoiEventSubsystem.h"
#include "PoiBasePawn.generated.h"

class UPoiSettingsPDA;
class UInputMappingContext;
class UArrowComponent;
class USpringArmComponent;
enum class ETriggerEvent : uint8;
class UFloatingPawnMovement;
class UCameraComponent;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInputActionEvent, const FInputActionValue&, Value);

UCLASS()
class SIMPLEPOI_API APoiBasePawn : public APawn
{
	GENERATED_BODY()
private:
	TMap<FString, TMap<ETriggerEvent, FEnhancedInputActionEventBinding*>> InputBindings;
public:
	APoiBasePawn();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//使用聚焦数据通过委托聚焦到对应位置
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent, Category="Pawn",meta=(DisplayName="Pawn使用聚焦数据聚焦"))
	bool FocusDataToTargetByHandle(const FFocusMessageStruct& FocusData);
	
protected:
	UPROPERTY(Category = "Pawn", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ForwardArrow;
	UPROPERTY(Category = "Pawn", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(Category = "Pawn", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(Category = "PoiPawn|Input",EditDefaultsOnly,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="PoiPawn|Input")
	TMap<FString,TObjectPtr<const UInputAction>> InputActionsMap;

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintNativeEvent, Category="PoiPawn|Input")
	void SetupInputComplete(UInputComponent* InPlayerInputComponent);
	
public:
	UPROPERTY(Category = "Pawn", VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> MovementComponent;
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> MainWidget;
	//POI初始化数据资产
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPoiSettingsPDA> PoiDataAsset;
	//Pawn设置数据
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FControlSettingStruct PawnData;
	//聚焦相关设置
	UPROPERTY(Category = "PoiPawn|Base", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FFocusHandleSet FocusSet;
	//添加输入事件绑定
	UFUNCTION(BlueprintCallable,Category="PoiPawn|Input",meta=(DisplayName="添加输入事件绑定"))
	void AddInputActionBindEvent(const FString ActionName, ETriggerEvent TriggerEvent,
		FOnInputActionEvent Function);
	UFUNCTION(BlueprintCallable,Category="PoiPawn|Input")
	const UInputAction* GetInputAction(const FString ActionName);

	//当子系统触发 接收Actor简单字符串消息 事件时调用,用蓝图重写
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category="PoiPawn|Event")
	void OnSubsystemCallTagEvent(AActor* CurActor,FGameplayTag CommandTag,const FString& InMessage);
};
