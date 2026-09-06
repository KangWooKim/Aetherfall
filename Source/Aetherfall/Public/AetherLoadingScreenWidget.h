#pragma once

#include "CoreMinimal.h"
#include "AetherLoadingScreenTypes.h"
#include "Blueprint/UserWidget.h"
#include "AetherLoadingScreenWidget.generated.h"

/** 로딩 표시 모델을 Blueprint 화면에 연결하는 확장 지점이다. 기본 C++ 구현은 별도 화면을 그리지 않는다. */
UCLASS(Blueprintable)
class AETHERFALL_API UAetherLoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Aetherfall|Loading Screen")
	void ApplyLoadingScreenViewModel(const FAetherLoadingScreenViewModel& ViewModel);
};
