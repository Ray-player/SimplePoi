#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SampleCommonFunctionLibrary.generated.h"

/**
 * 常用函数库
 */
UCLASS()
class SIMPLEPOI_API USampleCommonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure,Category="PoiCommonFunction|Input")
	static bool CastInputValueAsBoolean(const FInputActionValue& Value){return Value.Get<bool>();}
	UFUNCTION(BlueprintPure,Category="PoiCommonFunction|Input")
	static float CastInputValueAsAxis1D(const FInputActionValue& Value){return Value.Get<float>();}
	UFUNCTION(BlueprintPure,Category="PoiCommonFunction|Input")
	static FVector2D CastInputValueAsAxis2D(const FInputActionValue& Value){return Value.Get<FVector2D>();}
	UFUNCTION(BlueprintPure,Category="PoiCommonFunction|Input")
	static FVector CastInputValueAsAxis3D(const FInputActionValue& Value){return Value.Get<FVector>();}
	UFUNCTION(BlueprintPure,Category="PoiCommonFunction|Math")
	static float RoundToDecimalPlaces(const float Value, const int32 DecimalPlaces)
	{
		// 计算缩放因子
		const float Scale = FMath::Pow(10.0f, DecimalPlaces);
		// 缩放值，四舍五入，然后恢复原始比例
		return FMath::RoundToFloat(Value * Scale) / Scale;
	}
};
