#include "PoiActor.h"

#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SimplePoi/UI/PoiBaseUserWidget.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "EditorViewportClient.h"
#endif

APoiActor::APoiActor():
	Group(TEXT("Default")),
	SetArmLength(500.f),
	LineTraceDistance(INT_MAX),
	WidgetTypeIndex(0),
	bUseSameText(true)
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	/* 设置根组件在编辑器中的可视化*/
	RootComponent->Mobility = EComponentMobility::Movable;
#if WITH_EDITORONLY_DATA
	RootComponent->bVisualizeComponent = true;
#endif
	// 创建弹簧臂
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritRoll = true;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bConstrainAspectRatio = false;

	//创建widget
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	Widget->SetupAttachment(RootComponent);
	Widget->SetWidgetSpace(EWidgetSpace::Screen);
	Widget->SetDrawAtDesiredSize(true);
	// Widget->SetTickWhenOffscreen(true);
	
}

void APoiActor::BeginPlay()
{
	Super::BeginPlay();
	
}

UPoiBaseUserWidget* APoiActor::GetMyWidget()
{
	if (MyWidgetInstance) 
		return MyWidgetInstance;
	
	// 检查World和PlayerController的有效性
	UWorld* World = GetWorld();
	if (!World) 
		return nullptr;
		
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) 
		return nullptr;
	
	// 获取Widget类
	if (TSubclassOf<UPoiBaseUserWidget> WidgetClass = GetWidgetClassByIndex(WidgetTypeIndex))
	{
		// 创建Widget实例
		MyWidgetInstance = Cast<UPoiBaseUserWidget>(CreateWidget(PlayerController, WidgetClass));
		// 检查Widget创建是否成功
		if (MyWidgetInstance)
		{
			// 检查Widget组件是否存在
			if (Widget)
			{
				// 设置Widget
				Widget->SetWidget(MyWidgetInstance);
				MyWidgetInstance->OnAnimationPlayed.AddDynamic(this, &APoiActor::OnWidgetAniPlayedCallback);
				// 初始化Widget数据
				MyWidgetInstance->InitWidgetByStruct(this,WidgetSettingMap[WidgetClass]);
			}
		}
	}
	return MyWidgetInstance.Get();
}

TSubclassOf<UPoiBaseUserWidget> APoiActor::GetWidgetClassByIndex(int Index) const
{
	if (WidgetSettingMap.Num() > 0)
	{
		TArray<TSubclassOf<UPoiBaseUserWidget>> keys;
		WidgetSettingMap.GetKeys(keys);
		if (keys[Index])
		{
			return keys[Index];
		}
	}
	return nullptr;
}

void APoiActor::ConstructInitialData_Implementation()
{
	SpringArm->TargetArmLength = SetArmLength;
	FocusMessage.TargetLocation = Camera->GetComponentLocation();
	FocusMessage.TargetRotation = Camera->GetComponentRotation();
	FocusMessage.TargetSprintLens = SpringArm->TargetArmLength;
	UE_LOG(LogActor, Warning, TEXT("ViewLocation: %s, ViewRotation: %s"),
		*FocusMessage.TargetLocation.ToString(), *FocusMessage.TargetRotation.ToString());
	if (bUseSameText && WidgetSettingMap.Num() > 0)
	{
		TArray<TSubclassOf<UPoiBaseUserWidget>> WidgetKeys;
		WidgetSettingMap.GenerateKeyArray(WidgetKeys);
		for (auto Item : WidgetKeys)
		{
			WidgetSettingMap.Find(Item)->WidgetText = FText::FromString(FocusMessage.Name);
		}
		//FocusMessage.Name = WidgetSameText.ToString();
	}
	else if (Group.Len() > 0)
	{
		FocusMessage.Name = GetActorNameOrLabel();
	}
}

void APoiActor::ChangeWidgetBrushOnSelect(bool bIsPoiSelected)
{
	if (MyWidgetInstance && MyWidgetInstance->SelfData.bHasSelectedBrush)
	{
		const FWidgetDataStruct& WidgetMes = MyWidgetInstance->SelfData;
		// 在这里使用WidgetMes进行其他操作
		const FSlateBrush NewBrush = bIsPoiSelected ? WidgetMes.SelectedBrush : WidgetMes.Brush;
		MyWidgetInstance->SetMyWidgetBrush(NewBrush);
	}
}

void APoiActor::AlignDirectlyScreenLens()
{
	// 获取视口相机的当前位置
	// 只在编辑器环境下获取视口信息
	if (const EWorldType::Type WorldType = GetWorld()->WorldType; WorldType == EWorldType::Editor)
	{
#if WITH_EDITOR
		// 获取编辑器视口变换
		if (GEditor)
		{
			// 获取当前活动的视口客户端
			FLevelEditorViewportClient* ViewportClient =
				static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
			if (ViewportClient)
			{
				SetActorLocation(ViewportClient->GetViewLocation());
				SetActorRotation(ViewportClient->GetViewRotation());
				SetArmLength = 0.f;
				//重置相机相对旋转
				Camera->SetRelativeRotation(FRotator::ZeroRotator);
				SpringArm->TargetArmLength = SetArmLength;
				FocusMessage.TargetSprintLens = SetArmLength;
				FocusMessage.TargetLocation = Camera->GetComponentLocation();  // 更新目标位置
                FocusMessage.TargetRotation = Camera->GetComponentRotation();  // 更新目标旋转
				if (GIsEditor)
				{
					PostEditChange();
					MarkPackageDirty();
				}
			}
		}
#endif
	}
}

void APoiActor::AlignTraceNowScreenLens()
{
	// 只在编辑器环境下获取视口信息
	if (const EWorldType::Type WorldType = GetWorld()->WorldType; WorldType == EWorldType::Editor)
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
#if WITH_EDITOR
		// 获取编辑器视口变换
		if (GEditor)
		{
			// 获取当前活动的视口客户端
			FLevelEditorViewportClient* ViewportClient =
				static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
			if (ViewportClient)
			{
				// 获取视口位置和旋转
				ViewLocation = ViewportClient->GetViewLocation();
				ViewRotation = ViewportClient->GetViewRotation();

				const FVector ViewTargetLocation = ViewLocation + ViewRotation.Vector() * LineTraceDistance;
				if (FHitResult OutHit; UKismetSystemLibrary::LineTraceSingle(GetWorld(), ViewLocation, ViewTargetLocation,
				                                                             ETraceTypeQuery::TraceTypeQuery1, true, {},
				                                                             EDrawDebugTrace::None, OutHit, true))
				{
					// 处理命中结果
					if (OutHit.bBlockingHit)
					{
						//重置相机相对旋转
						Camera->SetRelativeRotation(FRotator::ZeroRotator);
						SetActorLocation(OutHit.Location, false, nullptr, ETeleportType::TeleportPhysics);
						SetActorRotation(ViewRotation, ETeleportType::TeleportPhysics);
						// 更新相机臂长
						SetArmLength = OutHit.Distance;
						SpringArm->TargetArmLength = SetArmLength;
						FocusMessage.TargetSprintLens = SetArmLength;
						FocusMessage.TargetLocation = Camera->GetComponentLocation();  // 更新目标位置
                        FocusMessage.TargetRotation = Camera->GetComponentRotation();  // 更新目标旋转
						
						if (GIsEditor)
						{
							PostEditChange();
							MarkPackageDirty();
						}
					}
				}
			}
		}
#endif
	}
}

void APoiActor::AlignFixLocNowScreenLens()
{
	// 获取视口相机的当前位置
	// 只在编辑器环境下获取视口信息
	if (const EWorldType::Type WorldType = GetWorld()->WorldType; WorldType == EWorldType::Editor)
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
#if WITH_EDITOR
		// 获取编辑器视口变换
		if (GEditor)
		{
			// 获取当前活动的视口客户端
			FLevelEditorViewportClient* ViewportClient =
				static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
			if (ViewportClient)
			{
				// 获取视口位置和旋转
				ViewLocation = ViewportClient->GetViewLocation();
				ViewRotation = ViewportClient->GetViewRotation();
				//计算旋转角度
				SetActorRotation(FRotationMatrix::MakeFromX(GetActorLocation() - ViewLocation).Rotator());
				Camera->SetWorldRotation(ViewRotation);
				// 更新相机臂长
				SetArmLength = (ViewLocation - GetActorLocation()).Size();
				SpringArm->TargetArmLength = SetArmLength;
				
				FocusMessage.TargetSprintLens = SetArmLength;
				FocusMessage.TargetLocation = Camera->GetComponentLocation();  // 更新目标位置
				FocusMessage.TargetRotation = Camera->GetComponentRotation();  // 更新目标旋转
				
				if (GIsEditor)
				{
					PostEditChange();
					MarkPackageDirty();
				}
			}
		}
#endif
	}
}

void APoiActor::UpdateFocusMessageOnNextTick()
{
	FocusMessage.TargetLocation = Camera->GetComponentLocation();
	FocusMessage.TargetRotation = Camera->GetComponentRotation();
}


void APoiActor::BindButtonEvent_Implementation(UButton* InButton)
{
	if (!InButton)
	{
		UE_LOG(LogActor,Error,TEXT("未获取到按钮类，请查看对应UI控件：%s 是否实现GetNeedBindButton函数"),*MyWidgetInstance.GetName());
	}
	//IPoiBaseInterface::BindButtonEvent_Implementation(InButton);
	// 先清除已绑定的事件，避免重复绑定
	InButton->OnClicked.Clear();
	InButton->OnHovered.Clear();
	InButton->OnUnhovered.Clear();
	// 直接绑定接口实现函数到按钮事件
	InButton->OnClicked.AddDynamic(this, &APoiActor::EWidgetOnClicked);
	InButton->OnHovered.AddDynamic(this, &APoiActor::EOnHovered);
	InButton->OnUnhovered.AddDynamic(this, &APoiActor::EUnHovered);
}

void APoiActor::ChangeWidgetType_Implementation(int Index)
{
	if (MyWidgetInstance)
	{
		if (TSubclassOf<UPoiBaseUserWidget> WidgetClass = GetWidgetClassByIndex(Index))
		{
			if (MyWidgetInstance->GetClass() != WidgetClass)
			{
				WidgetTypeIndex = Index;
				bInChangeWidgetState = true;
				MyWidgetInstance->PlayVisitAni(true,0.f);
			}
		}
	}
}

bool APoiActor::GetGroupAndKey_Implementation(FString& OutGroup, FString& OutKey) const
{
	OutGroup = Group;
	OutKey = FocusMessage.Name;
	return true;
}

void APoiActor::OnWidgetAniPlayedCallback_Implementation(const FString& AniName, const bool bIsForwardAni,
                                                         UPoiBaseUserWidget* InWidget)
{
	//如果为切换状态动画播放完成
	if (bInChangeWidgetState)
	{
		if (bIsForwardAni)
		{
			bInChangeWidgetState = false;
			if (MyWidgetInstance)
			{
				MyWidgetInstance->OnAnimationPlayed.RemoveDynamic(this,&APoiActor::OnWidgetAniPlayedCallback);
				MyWidgetInstance->RemoveFromParent();
				MyWidgetInstance = nullptr;
			}
			GetMyWidget()->PlayVisitAni(true,0.f);
		}
	}
	// else if (AniName.Contains(TEXT("Visit")))
	// {
	// 	bIsVisited = bIsForwardAni;
	// }
}

void APoiActor::SetPoiVisitByWidgetAni_Implementation(bool bIsVisit, float DelayTime)
{
	if (MyWidgetInstance)
	{
		bIsVisited = bIsVisit;
		MyWidgetInstance->PlayVisitAni(bIsVisit,DelayTime);
	}
}

void APoiActor::SetPoiVisitByProperties_Implementation(bool bIsVisit, float InDelayTime)
{
	if (MyWidgetInstance)
	{
		bIsVisited = bIsVisit;
		MyWidgetInstance->SetVisitByProperties(bIsVisit,InDelayTime);
	}
}

void APoiActor::EOnHovered_Implementation()
{
	if (!bIsSelected)
	{
		if (MyWidgetInstance->SelfData.bHasSelectedBrush) ChangeWidgetBrushOnSelect(true);
		MyWidgetInstance->PlayHoveredAni(true);
	}
}

void APoiActor::EUnHovered_Implementation()
{
	if (!bIsSelected)
	{
		if (MyWidgetInstance->SelfData.bHasSelectedBrush) ChangeWidgetBrushOnSelect(true);
		MyWidgetInstance->PlayHoveredAni(false);
	}
}

void APoiActor::EWidgetOnClicked_Implementation()
{
	// 获取游戏实例
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		// 从游戏实例中获取 PoiEventSubsystem
		if (UPoiEventSubsystem* PoiSubsystem = GameInstance->GetSubsystem<UPoiEventSubsystem>())
		{
			// 调用 SetCurrentSelectedActor 方法，传入当前 Actor 作为参数
			PoiSubsystem->SetCurrentSelectedActor(this);
		}
	}
}

