#pragma once

#include "CoreMinimal.h"
#include "PoiEventSubsystem.h"
#include "Engine/DataAsset.h"
#include "PoiSettingsPDA.generated.h"


/**
 * 负责项目UI与事件响应数据化存储的资产类
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class SIMPLEPOI_API UPoiSettingsPDA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoiAsset", meta=(DisplayName="项目标题"))
	FText ProjectTitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoiAsset", meta=(DisplayName="子标题目录"))
	TArray<FTextArrayStruct> SubTitleArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoiAsset", meta=(DisplayName="聚焦信息数组"))
	TArray<FFocusMessageStruct> FocusInfoArray;
	// 公共Action绑定,在pawn中初始化和存储实例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoiAsset Actions", meta=(DisplayName="公共Action绑定"))
	TArray<TSoftClassPtr<UPoiActionSet>> CommonActionSets;
	// 私有Action绑定,在pawn中仅加载类(SubClassOf),不生成具体实例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoiAsset Actions", meta=(DisplayName="私有Action绑定"))
	TArray<TSoftClassPtr<UPoiActionSet>> PrivateActionSets;

	// 获取子标题的父标题字符串
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PoiAsset")
	FString GetParentTitleStr(const FString& InTitle) const
	{
		for (auto Element : SubTitleArray)
		{
			if (Element.TitleText.EqualTo(FText::FromString(InTitle)))
			{
				return ProjectTitle.ToString();
			}
			for (auto ChildElement : Element.ChildrenText)
			{
				if (ChildElement.EqualTo(FText::FromString(InTitle)))
				{
					return Element.TitleText.ToString();
				}
			}
		}
		return FString("");
	}
};
