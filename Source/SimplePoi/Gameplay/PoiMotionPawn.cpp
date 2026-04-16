#include "PoiMotionPawn.h"
#include "InputTriggers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// 绑定输入函数到C++方法宏
#define Bind_InputFunction_OnCplus(InputActionName,FunctionName,TriggerType) \
FOnInputActionEvent FunctionName##Delegate;\
FunctionName##Delegate.BindDynamic(this, &APoiMotionPawn::FunctionName);\
AddInputActionBindEvent(TEXT(#InputActionName), ETriggerEvent::TriggerType,FunctionName##Delegate);

APoiMotionPawn::APoiMotionPawn():
	Super(),
	TargetHandleLoc(0.f, 0.f, 0.f),
	CurrentZoomValue(1.f),
	bLeftMouseDown(false), bRightMouseDown(false),
	bTransformationCompleted(false),
	MoveAxisXForward(FVector(0.f, 0.f, 1.f)),
	bKeyMoveForwardAtCameraForward(false),
	CameraZlineLength(500.f),
	LineTraceDistance(INT_MAX),
	LineTraceDebug(false)
{
}

void APoiMotionPawn::SetupInputComplete_Implementation(UInputComponent* InPlayerInputComponent)
{
	//Bind_InputFunction_OnCplus(IA_KeyMove,OnKeyMove,Triggered);
	Super::SetupInputComplete_Implementation(InPlayerInputComponent);
}

void APoiMotionPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void APoiMotionPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool APoiMotionPawn::UpdateKeyMoveData_Implementation(FVector InputAxis)
{
	const float KeySensitivity = 0.001f * PawnData.KeyMoveSensitivity;
	if (FMath::Abs(InputAxis.Y) > 0)
	{
		const FVector Forward = bKeyMoveForwardAtCameraForward
			                  ? GetActorForwardVector()
			                  : FVector::VectorPlaneProject(GetActorForwardVector(), FVector::UpVector);
		AddMovementInput(Forward, InputAxis.Y * KeySensitivity);
	}
	if (FMath::Abs(InputAxis.X) > 0)
	{
		AddMovementInput(GetActorRightVector(), InputAxis.X * KeySensitivity);
	}
	if (FMath::Abs(InputAxis.Z) > 0)
	{
		AddMovementInput(FVector::UpVector, InputAxis.Z * KeySensitivity);
	}
	return true;
}
bool APoiMotionPawn::GetMouseLocationInWorldSpace(FVector& OutLocation, FVector& WorldDir)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController != nullptr)
	{
		return  PlayerController->DeprojectMousePositionToWorld(OutLocation, WorldDir);
	}
	return false;
}

bool APoiMotionPawn::GetMouseHandleByGeometry(FVector& Intersection)
{
	FVector Dir;
	if (FVector StartLoc; GetMouseLocationInWorldSpace(StartLoc, Dir))
	{
		if (Camera == nullptr) { return false; }
		const FVector EndLoc = StartLoc + (Camera->GetComponentLocation().Z * CameraZlineLength) * Dir;
		const FPlane InPlane = UKismetMathLibrary::MakePlaneFromPointAndNormal(TargetHandleLoc, FVector(0.f, 0.f, 1.f));
		float inT = 0.f;
		return UKismetMathLibrary::LinePlaneIntersection(StartLoc, EndLoc, InPlane, inT, Intersection);
	}
	return false;
}

bool APoiMotionPawn::GetMouseHandleByLineTrace(FHitResult& OutHit, FVector& Intersection,
	float& Distance)
{
	FVector Dir;
	if (FVector StartLoc; GetMouseLocationInWorldSpace(StartLoc, Dir))
	{
		const FVector EndLoc = StartLoc + LineTraceDistance * Dir;
		const bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLoc, EndLoc, ETraceTypeQuery::TraceTypeQuery1,
			true, {}, LineTraceDebug? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, OutHit, true);
		Intersection = OutHit.Location;
		Distance=OutHit.Distance;
		return bHit;
	}
	
	return false;
}

FVector APoiMotionPawn::GetSpringArmOffsetLoc() const
{
	if (SpringArm && Camera)
	{
		const FVector ForwardSlotOffset = SpringArm->GetForwardVector() * (SpringArm->TargetArmLength - SpringArm->
			SocketOffset.X);
		const FVector LocationVec = SpringArm->GetUpVector() * SpringArm->SocketOffset.Z + SpringArm->
			GetComponentLocation();
		return LocationVec - ForwardSlotOffset - Camera->GetComponentLocation();
	}
	return FVector::ZeroVector;
}

bool APoiMotionPawn::InitPawnInputTriggerEvent(const FString& MouseMoveActionName,const FString& KeyMoveActionName)
{
	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(InputComponent); IsValid(EnhancedInputComponent))
	{
		if (const UInputAction* MouseAction = InputActionsMap.Find(MouseMoveActionName) ?
			(*InputActionsMap.Find(MouseMoveActionName)).Get() : nullptr; IsValid(MouseAction))
		{
			EnhancedInputComponent->BindAction(MouseAction,
				ETriggerEvent::Triggered,this,&APoiMotionPawn::OnMouseMove);
		}
		if (const UInputAction* KeyAction = InputActionsMap.Find(KeyMoveActionName) ?
			(*InputActionsMap.Find(KeyMoveActionName)).Get() : nullptr; IsValid(KeyAction))
		{
			EnhancedInputComponent->BindAction(KeyAction,
				ETriggerEvent::Triggered,this,&APoiMotionPawn::OnKeyMove);
		}
		return true;
	}
	return false;
}

void APoiMotionPawn::OnKeyMove_Implementation(const FInputActionValue& Value)
{
	FVector InputAxis = Value.Get<FVector>();
	UpdateKeyMoveData(InputAxis);
	
}

void APoiMotionPawn::OnMouseMove_Implementation(const FInputActionValue& Value)
{
	FVector2D InputAxis = Value.Get<FVector2D>();
	if (bLeftMouseDown)
		UpdateRotationData(InputAxis);
	if (bRightMouseDown)
		UpdateMoveData(InputAxis);
}

bool APoiMotionPawn::UpdateZoomData_Implementation(float InputAxis)
{
	bTransformationCompleted = false;
	if (PawnData.bCanScale)
	{
		if (PawnData.bMoveByMouseClick)
		{
			float HitDistance;
			if (FHitResult Hit; GetMouseHandleByLineTrace(Hit, TargetHandleLoc, HitDistance))
			{
				if (HitDistance >= CurrentZoomValue)
				{
					const float Alpha = InputAxis * CurrentZoomValue / 100.f;
					const FVector ZoomTarget = UKismetMathLibrary::VLerp(GetActorLocation(), TargetHandleLoc, Alpha);
					const FVector SetLocation = FMath::VInterpTo(GetActorLocation(), ZoomTarget,
					                                             GetWorld()->GetDeltaSeconds(), CurrentZoomValue);
					SetActorLocation(SetLocation,true);
					return true;
				}
			}
		}
		else
		{
			float NewArmLength =SpringArm->TargetArmLength - SpringArm->TargetArmLength * CurrentZoomValue * InputAxis;
			NewArmLength = FMath::Clamp(NewArmLength, PawnData.ScaleLimit.X, PawnData.ScaleLimit.Y);
			SpringArm->TargetArmLength = NewArmLength;
			return true;
		}
	}
	return false;
}

bool APoiMotionPawn::UpdateMoveData_Implementation(FVector2D InputAxis)
{
	FVector Intersection;
	if (const bool bGetMouse = GetMouseHandleByGeometry(Intersection); PawnData.bCanMouseMove && bGetMouse)
	{
		const FVector ActorLoc = GetActorLocation();
		const FVector TargetLoc = TargetHandleLoc - Intersection - GetSpringArmOffsetLoc() + ActorLoc;
		const FVector VInterpActorLoc = FVector(TargetLoc.X, TargetLoc.Y, ActorLoc.Z);
		const FVector SetLocation = FMath::VInterpTo(ActorLoc, VInterpActorLoc, GetWorld()->GetDeltaSeconds(),
													 PawnData.MoveSensitivity);
		SetActorLocation(SetLocation);
		return true;
	}
	return false;
}

bool APoiMotionPawn::StopUpdateMoveData_Implementation()
{
	return true;
}

bool APoiMotionPawn::UpdateRotationData_Implementation(FVector2D InputAxis)
{
	return true;
}



