#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeDamageTarget.generated.h"

class UAetherHealthComponent;
class UCapsuleComponent;
class UStaticMeshComponent;

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
