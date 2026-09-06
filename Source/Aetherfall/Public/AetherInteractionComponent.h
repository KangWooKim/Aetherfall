#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherInteractionComponent.generated.h"

/** 플레이어 주변에서 상호작용 인터페이스를 구현한 가장 가까운 액터를 선택하고 실행을 위임한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 선택 대상과 소유자가 살아 있으면 인터페이스 실행을 요청한다. 반환값은 대상 기능의 성공 여부가 아닌 요청 전달 여부다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Interaction")
	AActor* GetFocusedInteractableActor() const { return FocusedInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Interaction")
	FText GetFocusedInteractionPrompt() const;

private:
	/** 월드 액터를 순회하며 인터페이스와 비어 있지 않은 문구를 확인한 뒤 반경 안의 최근접 대상을 약한 참조로 보관한다. */
	void RefreshFocusedInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aetherfall|Interaction", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float InteractionRadius = 220.0f;

	TWeakObjectPtr<AActor> FocusedInteractableActor;
};
