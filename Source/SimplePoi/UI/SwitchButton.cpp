
#include "SwitchButton.h"
#include "Components/ButtonSlot.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Blueprint/WidgetTree.h"
#include "Binding/States/WidgetStateRegistration.h"
#include "Styling/DefaultStyleCache.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SwitchButton)

#define LOCTEXT_NAMESPACE "SimplePoiUMG"

/////////////////////////////////////////////////////
// USwitchButton

USwitchButton::USwitchButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsSwitchingStyleOnClick(true)
{
	// 初始化默认样式
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	DefaultButtonStyle = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetButtonStyle();
	AlternativeButtonStyle = UE::Slate::Private::FDefaultStyleCache::GetRuntime().GetButtonStyle();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

#if WITH_EDITOR
	if (IsEditorWidget())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		DefaultButtonStyle = UE::Slate::Private::FDefaultStyleCache::GetEditor().GetButtonStyle();
		AlternativeButtonStyle = UE::Slate::Private::FDefaultStyleCache::GetEditor().GetButtonStyle();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
#endif // WITH_EDITOR
	
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	ColorAndOpacity = FLinearColor::White;
	BackgroundColor = FLinearColor::White;

	ClickMethod = EButtonClickMethod::DownAndUp;
	TouchMethod = EButtonTouchMethod::DownAndUp;
	PressMethod = EButtonPressMethod::DownAndUp;

	IsFocusable = true;
	bIsUsingAlternateStyle = false;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

#if WITH_EDITORONLY_DATA
	AccessibleBehavior = ESlateAccessibleBehavior::Summary;
	bCanChildrenBeAccessible = false;
#endif
}

void USwitchButton::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyButton.Reset();
}

TSharedRef<SWidget> USwitchButton::RebuildWidget()
{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const FButtonStyle& CurrentStyle = bIsUsingAlternateStyle ? AlternativeButtonStyle : DefaultButtonStyle;
	
	MyButton = SNew(SButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered)
		.ButtonStyle(&CurrentStyle)
		.ClickMethod(ClickMethod)
		.TouchMethod(TouchMethod)
		.PressMethod(PressMethod)
		.IsFocusable(IsFocusable)
		;
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	if (GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}
	
	return MyButton.ToSharedRef();
}

void USwitchButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!MyButton.IsValid())
	{
		return;
	}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// 根据状态应用当前样式
	const FButtonStyle& CurrentStyle = bIsUsingAlternateStyle ? AlternativeButtonStyle : DefaultButtonStyle;
	MyButton->SetButtonStyle(&CurrentStyle);
	MyButton->SetColorAndOpacity(ColorAndOpacity);
	MyButton->SetBorderBackgroundColor(BackgroundColor);
	MyButton->SetClickMethod(ClickMethod);
	MyButton->SetTouchMethod(TouchMethod);
	MyButton->SetPressMethod(PressMethod);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

UClass* USwitchButton::GetSlotClass() const
{
	return UButtonSlot::StaticClass();
}

void USwitchButton::OnSlotAdded(UPanelSlot* InSlot)
{
	// 如果子项已经存在，则将其添加到活动槽中
	if (MyButton.IsValid())
	{
		CastChecked<UButtonSlot>(InSlot)->BuildSlot(MyButton.ToSharedRef());
	}
}

void USwitchButton::OnSlotRemoved(UPanelSlot* InSlot)
{
	// 如果存在，则从活动槽中移除小部件。
	if (MyButton.IsValid())
	{
		MyButton->SetContent(SNullWidget::NullWidget);
	}
}

void USwitchButton::SetUseAlternativeStyle(bool bUseAlternative)
{
	// 只有当样式实际发生变化时才更新和触发事件
	if (bIsUsingAlternateStyle != bUseAlternative)
	{
		bIsUsingAlternateStyle = bUseAlternative;
		SynchronizeProperties();
		
		// 触发样式变更事件，传递当前是否使用替代样式的状态
		OnStyleChanged.Broadcast(bIsUsingAlternateStyle);
	}
}

void USwitchButton::ToggleStyle()
{
	bIsUsingAlternateStyle = !bIsUsingAlternateStyle;
	SynchronizeProperties();
	
	// 触发样式变更事件，传递当前是否使用替代样式的状态
	OnStyleChanged.Broadcast(bIsUsingAlternateStyle);
}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
void USwitchButton::SetColorAndOpacity(FLinearColor InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	if (MyButton.IsValid())
	{
		MyButton->SetColorAndOpacity(InColorAndOpacity);
	}
}

void USwitchButton::SetBackgroundColor(FLinearColor InBackgroundColor)
{
	BackgroundColor = InBackgroundColor;
	if (MyButton.IsValid())
	{
		MyButton->SetBorderBackgroundColor(InBackgroundColor);
	}
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

bool USwitchButton::IsPressed() const
{
	if (MyButton.IsValid())
	{
		return MyButton->IsPressed();
	}

	return false;
}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
void USwitchButton::SetClickMethod(EButtonClickMethod::Type InClickMethod)
{
	ClickMethod = InClickMethod;
	if (MyButton.IsValid())
	{
		MyButton->SetClickMethod(ClickMethod);
	}
}

void USwitchButton::SetTouchMethod(EButtonTouchMethod::Type InTouchMethod)
{
	TouchMethod = InTouchMethod;
	if (MyButton.IsValid())
	{
		MyButton->SetTouchMethod(TouchMethod);
	}
}

void USwitchButton::SetPressMethod(EButtonPressMethod::Type InPressMethod)
{
	PressMethod = InPressMethod;
	if (MyButton.IsValid())
	{
		MyButton->SetPressMethod(PressMethod);
	}
}

void USwitchButton::InitIsFocusable(bool InIsFocusable)
{
	IsFocusable = InIsFocusable;
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

FReply USwitchButton::SlateHandleClicked()
{
	if (bIsSwitchingStyleOnClick)
	{
		// 点击时切换样式
		ToggleStyle();
	}
	
	// 广播点击事件
	OnClicked.Broadcast();

	return FReply::Handled();
}

void USwitchButton::SlateHandlePressed()
{
	OnPressed.Broadcast();
	BroadcastBinaryPostStateChange(UWidgetPressedStateRegistration::Bit, true);
}

void USwitchButton::SlateHandleReleased()
{
	OnReleased.Broadcast();
	BroadcastBinaryPostStateChange(UWidgetPressedStateRegistration::Bit, false);
}

void USwitchButton::SlateHandleHovered()
{
	OnHovered.Broadcast();
	BroadcastBinaryPostStateChange(UWidgetHoveredStateRegistration::Bit, true);
}

void USwitchButton::SlateHandleUnhovered()
{
	OnUnhovered.Broadcast();
	BroadcastBinaryPostStateChange(UWidgetHoveredStateRegistration::Bit, false);
}

#if WITH_ACCESSIBILITY
TSharedPtr<SWidget> USwitchButton::GetAccessibleWidget() const
{
	return MyButton;
}
#endif

#if WITH_EDITOR

const FText USwitchButton::GetPaletteCategory()
{
	return LOCTEXT("PoiSlateCore","Poi Widget");
}

void USwitchButton::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// 获取变更的属性名称
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// 检查是否是bIsUsingAlternateStyle属性变更
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USwitchButton, bIsUsingAlternateStyle))
	{
		SetUseAlternativeStyle(bIsUsingAlternateStyle);
	}
}

#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE