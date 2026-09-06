#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AetherInteractableInterface.generated.h"

UINTERFACE(BlueprintType)
/** 구체적인 상호작용 액터 종류와 무관하게 안내 문구 조회와 상호작용 실행을 요청하는 계약이다. */
class AETHERFALL_API UAetherInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/** 상호작용 문구와 실행을 공통 API로 제공한다. 호출자는 Unreal 인터페이스 실행 경로를 사용한다. */
class AETHERFALL_API IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	/** 호출자에게 보여줄 문구를 반환한다. 빈 문구는 현재 상호작용 후보에서 제외하는 기준으로 쓰인다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aetherfall|Interaction")
	FText GetInteractionPrompt(AActor* Interactor) const;

	/** 소유 액터의 구체적인 상호작용 처리를 실행한다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aetherfall|Interaction")
	void Interact(AActor* Interactor);
};
