#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeDamageTarget.generated.h"

class UAetherHealthComponent;
class UCapsuleComponent;
class UStaticMeshComponent;

/** 전투 피해와 사망 이벤트를 확인할 수 있도록 체력 컴포넌트와 단순 충돌·표시를 제공하는 실험용 표적이다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeDamageTarget : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeDamageTarget();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype")
	FORCEINLINE UAetherHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleHealthChanged(UAetherHealthComponent* ChangedHealthComponent, float CurrentHealth, float MaxHealth, AActor* DamageCauser);

	/** 표적의 충돌을 끄고 짧은 지연 후 제거하여 사망 판정과 잔상 상태를 확인할 수 있게 한다. */
	UFUNCTION()
	void HandleDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser);

	void ShowTargetDebugMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA")
	bool bShowTargetScreenDebugMessages = false;
};
