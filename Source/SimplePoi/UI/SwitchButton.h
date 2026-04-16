// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "Styling/SlateTypes.h"
#include "Input/Reply.h"
#include "SwitchButton.generated.h"

class SButton;
class USlateWidgetStyleAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwitchButtonClickedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwitchButtonPressedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwitchButtonReleasedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwitchButtonHoverEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwitchButtonStyleChangedEvent, bool, bIsAlternativeStyle);

/**
 * 一个可以在点击时在两种不同样式之间切换的按钮
 */
UCLASS(MinimalAPI)
class USwitchButton : public UContentWidget
{
	GENERATED_BODY()

public:
	USwitchButton(const FObjectInitializer& ObjectInitializer);

	/** 点击时是否切换样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (DisplayName = "Switch Style"))
	bool bIsSwitchingStyleOnClick;

		/** 运行时使用的默认按钮样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (DisplayName = "Default Style"))
	FButtonStyle DefaultButtonStyle;

	/** 切换时使用的替代按钮样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (DisplayName = "Alternative Style"))
	FButtonStyle AlternativeButtonStyle;

	/** 按钮当前是否使用替代样式 */
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "State")
	bool bIsUsingAlternateStyle;

	/** 按钮内容的颜色乘数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (sRGB = "true"))
	FLinearColor ColorAndOpacity;

	/** 按钮背景的颜色乘数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (sRGB = "true"))
	FLinearColor BackgroundColor;

	/** 用户触发按钮'点击'所需的鼠标操作类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", AdvancedDisplay)
	TEnumAsByte<EButtonClickMethod::Type> ClickMethod;

	/** 用户触发按钮'点击'所需的触摸操作类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", AdvancedDisplay)
	TEnumAsByte<EButtonTouchMethod::Type> TouchMethod;

	/** 用户触发按钮'点击'所需的键盘/游戏手柄按钮按下操作类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", AdvancedDisplay)
	TEnumAsByte<EButtonPressMethod::Type> PressMethod;

	/** 有时按钮应该只能通过鼠标点击，而不应该可以通过键盘聚焦。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool IsFocusable;

	// 修改后的属性声明
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonClickedEvent OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonPressedEvent OnPressed;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonReleasedEvent OnReleased;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonHoverEvent OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonHoverEvent OnUnhovered;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FSwitchButtonStyleChangedEvent OnStyleChanged;

/** 设置按钮是否应该使用替代样式 */
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
	void SetUseAlternativeStyle(bool bUseAlternative);

	/** 获取按钮当前是否使用替代样式 */
	UFUNCTION(BlueprintCallable, Category = "Button|State")
	bool GetIsUsingAlternativeStyle() const { return bIsUsingAlternateStyle; }

	/** 在默认样式和替代样式之间切换 */
	UFUNCTION(BlueprintCallable, Category = "Button|Action")
	void ToggleStyle();

	/** 设置按钮内容的颜色乘数 */
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
	void SetColorAndOpacity(FLinearColor InColorAndOpacity);

	FLinearColor GetColorAndOpacity() const { return ColorAndOpacity; }

	/** 设置按钮背景的颜色乘数 */
	UFUNCTION(BlueprintCallable, Category = "Button|Appearance")
	void SetBackgroundColor(FLinearColor InBackgroundColor);

	FLinearColor GetBackgroundColor() const { return BackgroundColor; }

	/**
	 * 如果用户正在按下按钮，则返回true。
	 *
	 * @return 如果用户正在按下按钮则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintCallable, Category = "Button")
	bool IsPressed() const;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetClickMethod(EButtonClickMethod::Type InClickMethod);

	EButtonClickMethod::Type GetClickMethod() const { return ClickMethod; }

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetTouchMethod(EButtonTouchMethod::Type InTouchMethod);

	EButtonTouchMethod::Type GetTouchMethod() const { return TouchMethod; }

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetPressMethod(EButtonPressMethod::Type InPressMethod);

	EButtonPressMethod::Type GetPressMethod() const { return PressMethod; }

	bool GetIsFocusable() const { return IsFocusable; }

//~ 开始UWidget接口
	virtual void SynchronizeProperties() override;
	//~ 结束UWidget接口

	//~ 开始UVisual接口
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ 结束UVisual接口

protected:
	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// 结束UPanelWidget
	
	/** 处理来自Slate的实际点击事件并转发它 */
	FReply SlateHandleClicked();
	void SlateHandlePressed();
	void SlateHandleReleased();
	void SlateHandleHovered();
	void SlateHandleUnhovered();

	// 在SWidget构造之前在构造函数中初始化IsFocusable。
	void InitIsFocusable(bool InIsFocusable);

	//~ 开始UWidget接口
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override { return Content; }
	virtual const FText GetPaletteCategory() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ 结束UWidget接口

#if WITH_ACCESSIBILITY
	virtual TSharedPtr<SWidget> GetAccessibleWidget() const override;
#endif
	
	/** 此UWidget拥有的底层Slate按钮的缓存指针 */
	TSharedPtr<SButton> MyButton;
};
