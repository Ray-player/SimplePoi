
#include "PoiBasePawn.h"


#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
APoiBasePawn::APoiBasePawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	SetRootComponent(ForwardArrow);
	
	// 创建弹簧臂
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritRoll = true;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraLagSpeed = 8.f;
	SpringArm->CameraRotationLagSpeed = 5.f;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bConstrainAspectRatio = false;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(RootComponent);
	MovementComponent->MaxSpeed = 800000.f;
	MovementComponent->Acceleration = 5000000.f;
	MovementComponent->Deceleration = 10000000.f;
	MovementComponent->TurningBoost = 1.f;
}

// Called to bind functionality to input
void APoiBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>((Cast<APlayerController>(Controller)->GetLocalPlayer()));
	for (int32 Index = 0; Index < InputMappingContexts.Num(); ++Index)
	{
		if (UInputMappingContext* MappingContext = InputMappingContexts[Index])
		{
			// 使用索引作为优先级，索引越大优先级越小
			InputSubsystem->AddMappingContext(MappingContext, InputMappingContexts.Num() - Index);
			for (auto Action : MappingContext->GetMappings())
			{
				InputActionsMap.Add(Action.Action->GetName(), Action.Action);
			}
		}
	}
	
	SetupInputComplete(PlayerInputComponent);
}

bool APoiBasePawn::FocusDataToTargetByHandle_Implementation(const FFocusMessageStruct& FocusData)
{
	return false;
}

void APoiBasePawn::BeginPlay()
{
	Super::BeginPlay();
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		// 从游戏实例中获取 PoiEventSubsystem
		if (UPoiEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UPoiEventSubsystem>())
		{
			EventSubsystem->ReceivedActorSimpleStrByTag.AddDynamic(this,&APoiBasePawn::OnSubsystemCallTagEvent);
		}
	}
	
	//从聚焦数据中初始化位置
	// if (!PawnData.InitialFocusMessage.Name.IsEmpty())
	// {
	// 	SetActorLocationAndRotation(PawnData.InitialFocusMessage.TargetLocation,
	// 		PawnData.InitialFocusMessage.TargetRotation);
	// }
}

void APoiBasePawn::AddInputActionBindEvent(const FString ActionName, ETriggerEvent TriggerEvent,
                                           FOnInputActionEvent Function)
{
	if (TriggerEvent == ETriggerEvent::None || !Function.IsBound())
	{
		UE_LOG(LogController, Error, TEXT("事件选择/未绑定"));
		return;
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent); IsValid(EnhancedInputComponent))
	{
		
		if (const UInputAction* Action = GetInputAction(ActionName); IsValid(Action))
		{
			// 检查是否已存在绑定，若存在则先解绑
			if (InputBindings.Contains(ActionName) && InputBindings[ActionName].Contains(TriggerEvent))
			{
				FEnhancedInputActionEventBinding* ExistingBinding = InputBindings[ActionName][TriggerEvent];
				EnhancedInputComponent->RemoveBinding(*ExistingBinding);
			}
			// 使用lambda包装动态委托
             FEnhancedInputActionEventBinding& Binding = EnhancedInputComponent->BindActionValueLambda(Action, TriggerEvent,
				[this,Function](const FInputActionValue& Value) mutable
				{
					// 安全调用动态委托
					Function.ExecuteIfBound(Value);
				});
			// 存储新的绑定句柄
			InputBindings.FindOrAdd(ActionName).Add(TriggerEvent, &Binding);
		}
		else
		{
			UE_LOG(LogController, Error, TEXT("AddInputActionBindEvent: ActionName %s is not found"), *ActionName);
		}
	}
}

const UInputAction* APoiBasePawn::GetInputAction(const FString ActionName)
{
	for (auto InputActionName : InputActionsMap)
	{
		if (InputActionName.Key.Contains(ActionName))
		{
			return InputActionName.Value;
		}
	}
	UE_LOG(LogController, Error, TEXT("GetInputAction: ActionName %s is not found"), *ActionName);
	return nullptr;
}

void APoiBasePawn::OnSubsystemCallTagEvent_Implementation(AActor* CurActor, FGameplayTag CommandTag,
	const FString& InMessage)
{
}

void APoiBasePawn::SetupInputComplete_Implementation(UInputComponent* InPlayerInputComponent)
{
}

