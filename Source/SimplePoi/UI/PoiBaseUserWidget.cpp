#include "PoiBaseUserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"

void UPoiBaseUserWidget::InitWidgetByStruct_Implementation(APoiActor* InPoiActor, const FWidgetDataStruct& WidgetMes)
{
	//保存数据
	OwnerPoi = InPoiActor;
	SelfData = WidgetMes;
	if (UButton* Button = GetNeedBindButton())
	{
		IPoiActorInterface::Execute_BindButtonEvent(static_cast<UObject*>(OwnerPoi),Button);
	}
	//设置文本
	if (UTextBlock* TextBlock = GetNeedTextBlock())
		TextBlock->SetText(WidgetMes.WidgetText);
	//设置整体缩放
	if (WidgetMes.WidgetSize.Length()>0)
		this->SetRenderScale(WidgetMes.WidgetSize);
	if (UOverlay* Overlay = GetTolOverlayWidget())
	{
		Overlay->SetRenderScale(FVector2D(0.f,0.f));
	}
	
}

void UPoiBaseUserWidget::SetVisitByProperties_Implementation(bool bIsVisit, float InDelayTime)
{
	if (UOverlay* Overlay = GetTolOverlayWidget())
	{
		const FVector2D Render2D = bIsVisit ? FVector2D::One() : FVector2D::Zero();
		if (InDelayTime > 0)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			                                       [this,Render2D, Overlay]()
			                                       {
				                                       Overlay->SetRenderScale(Render2D);
			                                       }, InDelayTime, false);
		}
		else
		{
			Overlay->SetRenderScale(Render2D);
		}
	}
}

UWidgetAnimation* UPoiBaseUserWidget::GetAnimationComponentByKey(const FString& Key)
{
	if (AnimationDic.IsEmpty())
	{
		UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
		if (WidgetClass && WidgetClass->Animations.Num() > 0)
		{
			for (UWidgetAnimation* Animat : WidgetClass->Animations)
			{
				AnimationDic.Add(Animat->GetName(), Animat);
			}
		}
	}
	//初始化后从映射中返回动画
	if (const TObjectPtr<UWidgetAnimation>* Anim = AnimationDic.Find(Key))
	{
		return *Anim;
	}
	//未查找到时遍历匹配的动画
	else
	{
		TArray<FString> Keys;
		AnimationDic.GetKeys(Keys);
		for (const FString& AnimKey : Keys)
		{
			if (AnimKey.Contains(Key))
			{
				return AnimationDic[AnimKey];
			}
		}
	}
	return nullptr;
}

void UPoiBaseUserWidget::PlayAnimationByDirKey_Implementation(const FString& Key, const bool bIsForwardAni,
	const float PlayRate, const float DelayTime)
{
	if (DelayTime>0)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			[this,Key,bIsForwardAni,PlayRate](){PlayAniFunction(Key,PlayRate,bIsForwardAni);},DelayTime,false);
	}
	else
	{
		PlayAniFunction(Key,PlayRate,bIsForwardAni);
	}
}

UOverlay* UPoiBaseUserWidget::GetTolOverlayWidget_Implementation() const
{
	// 使用WidgetTree遍历查找根UOverlay组件
	UOverlay* Result = nullptr;
	this->WidgetTree->ForEachWidget([&Result](UWidget* Widget)-> bool
	{
		if (UOverlay* Overlay = Cast<UOverlay>(Widget))
		{
			Result = Overlay;
			return false; // 找到后停止遍历
		}
		return true; // 继续遍历
	});
	return Result;
}

void UPoiBaseUserWidget::SetMyWidgetBrush(const FSlateBrush& NewBrush)
{
	if (UButton* Button = GetNeedBindButton())
	{
		FButtonStyle ButtonStyle = Button->GetStyle();
		ButtonStyle.SetNormal(NewBrush);
		ButtonStyle.SetHovered(NewBrush);
		ButtonStyle.SetPressed(NewBrush);
		Button->SetStyle(ButtonStyle);
	}
}

void UPoiBaseUserWidget::PlayAniFunction(const FString& Key, const float PlayRate, bool bIsForwardAni)
{
	if (TObjectPtr<UWidgetAnimation> PlayAni = GetAnimationComponentByKey(Key); IsValid(PlayAni))
	{
		// 记录当前播放的动画信息
		CurAniKey = Key;
		bCurIsForwardAni = bIsForwardAni;
		const EUMGSequencePlayMode::Type PlayMode = bIsForwardAni
													  ? EUMGSequencePlayMode::Forward
													  : EUMGSequencePlayMode::Reverse;
		// 播放动画
		PlayAnimation(PlayAni, 0.f, 1, PlayMode, PlayRate);
		
		// 绑定动画完成事件
		if(bIsBoundAniEvent && !AniEventBoundedMap.Contains(PlayAni))
		{
			AniEventBoundedMap.Add(PlayAni,true);
			FWidgetAnimationDynamicEvent AniEvent;
			AniEvent.BindDynamic(this,&UPoiBaseUserWidget::OnAniFinished);
			BindToAnimationFinished(PlayAni, AniEvent);
		}
	}
}
void UPoiBaseUserWidget::PlayVisitAni_Implementation(const bool bIsForwardAni,const float DelayTime)
{
	const FString AniKey = TEXT("Visit");
	float PlayRate = 1.f;
	UPoiEventSubsystem::FindAniSpeed(AniKey,SelfData,PlayRate);
	if (UOverlay* lay = GetTolOverlayWidget(); IsValid(lay) && bIsForwardAni)
	{
		lay->SetRenderScale(FVector2D(0.0f, 0.0f));
	}
	PlayAnimationByDirKey(AniKey,bIsForwardAni,PlayRate,DelayTime);
}

void UPoiBaseUserWidget::PlayHoveredAni_Implementation(const bool bIsForwardAni,const float DelayTime)
{
	// 播放Hovered动画前先检查是否正在播放Visit动画
	TObjectPtr<UWidgetAnimation> VisitAni = GetAnimationComponentByKey(TEXT("Visit"));
	if (!this->IsAnimationPlaying(VisitAni))
	{
		const FString AniKey = TEXT("Hovered");
		float PlayRate = 1.f;
		UPoiEventSubsystem::FindAniSpeed(AniKey,SelfData,PlayRate);
		PlayAnimationByDirKey(AniKey,bIsForwardAni,PlayRate,DelayTime);
	}
}

UButton* UPoiBaseUserWidget::GetNeedBindButton_Implementation() const
{
	// 使用WidgetTree遍历查找第一个按钮组件
	UButton* Result = nullptr;
	this->WidgetTree->ForEachWidget([&Result](UWidget* Widget)-> bool
	{
		if (UButton* Button = Cast<UButton>(Widget))
		{
			Result = Button;
			return false; // 找到后停止遍历
		}
		return true; // 继续遍历
	});
	return Result;
}

UTextBlock* UPoiBaseUserWidget::GetNeedTextBlock_Implementation() const
{
	// 使用WidgetTree遍历查找第一个文本组件
	UTextBlock* Result = nullptr;
	this->WidgetTree->ForEachWidget([&Result](UWidget* Widget)-> bool
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			Result = TextBlock;
			return false; // 找到后停止遍历
		}
		return true; // 继续遍历
	});
	return Result;
}