#pragma once

#include "CoreMinimal.h"
#include "AetherCombatActionGatePolicy.h"
#include "AetherCombatActionStatePolicy.h"
#include "AetherCombatActionTimerPolicy.h"
#include "AetherCombatActionTuningPolicy.h"
#include "AetherCombatAudioCuePolicy.h"
#include "AetherCombatFeedbackPolicy.h"
#include "AetherCombatResourcePolicy.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "AetherCombatComponent.generated.h"

class AAetherfallCharacter;
class AAetherEnemyBase;
class UAnimMontage;
class UAetherCombatActionDataAsset;
class UAetherHealthComponent;
class UParticleSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FAetherExecutionVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Execution")
	FName VariantName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Execution")
	TObjectPtr<UAnimMontage> PlayerExecutionMontage;

	UPROPERTY(EditAnywhere, Category = "Execution")
	TObjectPtr<UAnimMontage> EnemyDeathMontage;
};

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartLightAttack();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartHeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartExecution();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartAetherSlash();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartDodge();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartGuard();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StopGuard();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void TryParry();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void SimulateIncomingHit();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void ReceiveIncomingHit(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void ResetPlayerPrototypeState();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void GrantPrototypeAetherGauge(float Amount, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleLightAttackHitNotify();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleHeavyAttackHitNotify();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleAetherSlashFireNotify();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleExecutionImpactNotify();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsHeavyAttacking() const { return bIsHeavyAttacking; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsExecuting() const { return bIsExecuting; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsAetherSlashing() const { return bIsAetherSlashing; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsDodging() const { return bIsDodging; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsGuarding() const { return bIsGuarding; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsParryWindowActive() const { return bIsParryWindowActive; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsParryCounterWindowActive() const { return bIsParryCounterWindowActive; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsDamageInvulnerable() const { return bIsDamageInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE bool IsHitReacting() const { return bIsHitReacting; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE int32 GetCurrentComboStep() const { return CurrentComboStep; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE float GetCurrentAetherGauge() const { return CurrentAetherGauge; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void NotifyOwnerHealed();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE float GetMaxAetherGauge() const { return MaxAetherGauge; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	float GetAetherSlashCost() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	float GetAetherSlashCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	bool IsAetherSlashReady() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	bool ShouldBlockMovementInput() const;

	bool IsEnemyExecutionReady(const AAetherEnemyBase* Enemy) const;

protected:
	virtual void BeginPlay() override;

private:
	FAetherCombatActionStateSnapshot BuildCombatActionStateSnapshot() const;
	FAetherCombatActionRuntimeFlags BuildCombatActionRuntimeFlags() const;
	void ApplyCombatActionRuntimeFlags(const FAetherCombatActionRuntimeFlags& RuntimeFlags);
	void SetCombatActionMode(EAetherCombatActionMode Mode);
	EAetherCombatActionMode GetCombatActionMode() const;
	void ApplyCombatActionTimerClearPlan(const FAetherCombatActionTimerClearPlan& TimerClearPlan);
	void ApplyCombatResourceMutationPlan(const FAetherCombatResourceMutationPlan& ResourceMutationPlan);
	void BeginLightAttack();
	void BeginHeavyAttack();
	void EndExecution();
	void EndCurrentAttack();
	void EndDodge();
	void EndParryWindow();
	void EndParryRecovery();
	void EndParryCounterWindow();
	void OpenParryCounterWindow();
	void BeginAutomaticParryCounter(AActor* CounterTarget);
	void BeginDamageInvulnerability();
	void EndDamageInvulnerability();
	void BeginHitReaction(AActor* DamageCauser);
	void EndHitReaction();
	FVector GetHitReactionDirection(AActor* DamageCauser) const;
	void UpdateDodgeMovement(float DeltaTime);
	void ClearDodgeMovementState();
	void ResetCombo();
	void RegenerateStamina(float DeltaTime);
	void AddAetherGauge(float Amount, const FString& Reason);
	bool SpendAetherGauge(float Amount, const FString& Reason);
	bool ApplyIncomingDamage(float DamageAmount, AActor* DamageCauser, bool bTriggerHitReaction = true);
	void ClearCombatRuntimeState();
	bool IsOwnerDead() const;
	void PerformPrototypeTrace(int32 ComboStep);
	void PrepareReusableTraceHitResults(int32 ExpectedHitCount);
	void ApplyPrototypeDamage(const TArray<FHitResult>& HitResults, int32 ComboStep);
	AActor* GetLockedCombatTarget() const;
	bool ApplyPrototypeDamageToActor(AActor* TargetActor, int32 ComboStep, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage);
	void PerformHeavyAttackTrace();
	void ApplyHeavyAttackDamage(const TArray<FHitResult>& HitResults);
	bool ApplyHeavyDamageToActor(AActor* TargetActor, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage, bool bStaggerBonus);
	void RefreshExecutionOpportunityAfterHeavyCounter(AActor* TargetActor, bool bStaggerBonus) const;
	void PerformAetherSlashTrace();
	AAetherEnemyBase* FindExecutionTarget() const;
	const FAetherExecutionVariant* SelectExecutionVariant();
	void ResolveExecutionImpact();
	void HandleExecutionImpactFallback();
	void ClearPendingExecutionImpact();
	int32 SuppressNearbyEnemiesForExecution(AAetherEnemyBase* ExecutionTarget, float SuppressionDuration) const;
	FAetherGuardStaminaTuning BuildGuardStaminaTuning() const;
	const TArray<float>& ResolveLightAttackStaminaCosts() const;
	const TArray<float>& ResolveLightAttackDamageValues() const;
	float ResolveLightAttackDuration() const;
	float ResolveLightTraceDistance() const;
	float ResolveLightTraceRadius() const;
	float ResolveHeavyAttackDuration() const;
	float ResolveHeavyAttackImpactDelay() const;
	float ResolveHeavyAttackStaminaCost() const;
	float ResolveHeavyAttackDamage() const;
	float ResolveHeavyStaggerDamageMultiplier() const;
	float ResolveHeavyTraceDistance() const;
	float ResolveHeavyTraceRadius() const;
	float ResolveAetherSlashCost() const;
	float ResolveAetherSlashDamage() const;
	float ResolveAetherSlashDuration() const;
	float ResolveAetherSlashImpactDelay() const;
	float ResolveAetherSlashCooldown() const;
	float ResolveAetherSlashProjectileSpeed() const;
	float ResolveAetherSlashTraceDistance() const;
	float ResolveAetherSlashTraceRadius() const;
	UAnimMontage* ResolveHeavyAttackMontage(bool bCounterHeavy) const;
	UAnimMontage* ResolveAetherSlashMontage() const;
	void ApplyGuardMovementModifier();
	void RestoreGuardMovementModifier();
	void StopOwnerMovementForCombatAction(bool bRespectActionSetting = true);
	void SetOwnerMovementEnabled(bool bEnabled);
	void ShowCombatDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;
	void PlayImpactCameraFeedback(float Strength, const FString& Reason) const;
	void PlayImpactFeedback(EAetherCombatFeedbackType FeedbackType, float CameraStrength, float HitStopDuration, const FString& Reason, AActor* ImpactTarget = nullptr) const;
	void ApplyHitStopToActor(AActor* Actor, float Duration) const;
	void PlayActionMontage(UAnimMontage* Montage, const FString& Reason) const;
	void PlayActionSound(USoundBase* Sound, float VolumeMultiplier = 1.0f) const;
	float GetRandomizedAudioVolume(float BaseVolume, float Variance) const;
	float GetRandomizedAudioPitch(float MinPitch, float MaxPitch) const;
	FAetherPlayerDangerCueConfig BuildPlayerDangerCueConfig() const;
	void UpdatePlayerDangerAudioCue(const UAetherHealthComponent* HealthComponent);
	void ResetPlayerDangerAudioCues();
	void PlayWarningSound(USoundBase* Sound, const FString& Reason) const;
	FAetherResourceCueConfig BuildResourceCueConfig() const;
	void UpdateAetherResourceAudioCues(float PreviousAetherGauge);
	void ResetAetherResourceAudioCues();
	void PlayAetherResourceSound(USoundBase* Sound, const FString& Reason) const;
	UAnimMontage* GetLightAttackMontage(int32 ComboStep) const;
	UAnimMontage* GetDodgeMontageForDirection(const FVector& WorldDirection) const;
	FAetherCombatFeedbackAssets BuildCombatFeedbackAssets() const;
	void PlayFeedbackAssets(EAetherCombatFeedbackType FeedbackType, AActor* ImpactTarget) const;
	FVector GetFeedbackImpactLocation(AActor* ImpactTarget) const;

	UFUNCTION()
	void HandleOwnerDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Authoring")
	TObjectPtr<UAetherCombatActionDataAsset> CombatActionDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "1"))
	int32 MaxComboSteps = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "0.1"))
	float LightAttackDuration = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "0.0"))
	float ComboResetDelay = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation Notify")
	bool bUseAnimationNotifiesForAttackTraces = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.1"))
	float HeavyAttackDuration = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.0"))
	float HeavyAttackImpactDelay = 0.22f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.0"))
	float HeavyAttackStaminaCost = 34.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.0"))
	float HeavyAttackDamage = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "1.0"))
	float HeavyStaggerDamageMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.1"))
	float ExecutionDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0"))
	float ExecutionRange = 260.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExecutionHealthThresholdPercent = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0"))
	float ExecutionDamage = 9999.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0"))
	float ExecutionCrowdPauseRadius = 450.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0"))
	float HeavyCounterExecutionStaggerRefreshDuration = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution")
	bool bUseExecutionImpactNotify = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.05", ClampMax = "0.95", EditCondition = "bUseExecutionImpactNotify"))
	float ExecutionImpactFallbackNormalizedTime = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Aether", meta = (ClampMin = "1.0"))
	float MaxAetherGauge = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Aether", meta = (ClampMin = "0.0"))
	float LightHitAetherGain = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Aether", meta = (ClampMin = "0.0"))
	float HeavyHitAetherGain = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Aether", meta = (ClampMin = "0.0"))
	float ParrySuccessAetherGain = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Aether", meta = (ClampMin = "0.0"))
	float ExecutionAetherGain = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashCost = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashDamage = 32.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.1"))
	float AetherSlashDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashImpactDelay = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashCooldown = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "1.0"))
	float AetherSlashProjectileSpeed = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashTraceDistance = 650.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|AetherSlash", meta = (ClampMin = "0.0"))
	float AetherSlashTraceRadius = 85.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Resource", meta = (ClampMin = "1.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Resource")
	TArray<float> LightAttackStaminaCosts;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	TArray<float> LightAttackDamageValues;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Resource", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Resource", meta = (ClampMin = "0.0"))
	float StaminaRegenPerSecond = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float DodgeStaminaCost = 28.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float DodgeStrength = 1150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float DodgeDuration = 0.28f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float DodgeCooldown = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	bool bUseRootMotionForDodge = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (EditCondition = "!bUseRootMotionForDodge", EditConditionHides))
	bool bUseTimedDodgeMovement = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.0", EditCondition = "!bUseRootMotionForDodge", EditConditionHides))
	float DodgeTravelDistance = 330.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.1", EditCondition = "!bUseRootMotionForDodge", EditConditionHides))
	float DodgeEaseOutExponent = 2.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Movement")
	bool bStopMovementOnCombatAction = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Movement")
	bool bStopMovementWhenDodgeEnds = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "0.0"))
	float GuardStaminaCostPerHit = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard")
	bool bScaleGuardStaminaCostByIncomingDamage = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "1.0", EditCondition = "bScaleGuardStaminaCostByIncomingDamage"))
	float GuardStaminaDamageReference = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "0.0", EditCondition = "bScaleGuardStaminaCostByIncomingDamage"))
	float MinGuardStaminaCostPerHit = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "0.0", EditCondition = "bScaleGuardStaminaCostByIncomingDamage"))
	float MaxGuardStaminaCostPerHit = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GuardDamageReduction = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard")
	bool bSlowMovementWhileGuarding = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Guard", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "bSlowMovementWhileGuarding"))
	float GuardMovementSpeedMultiplier = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.0"))
	float ParryStaminaCost = 16.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.01"))
	float ParryWindowDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.0"))
	float ParryRecoveryDuration = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.0"))
	float ParryCounterWindowDuration = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry Counter")
	bool bAutoCounterOnParrySuccess = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry Counter", meta = (ClampMin = "0.0"))
	float AutoParryCounterImpactDelay = 0.18f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry Counter", meta = (ClampMin = "0.1"))
	float AutoParryCounterDuration = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float DamageInvulnerabilityDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float HitReactionDuration = 0.28f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float EnemyAttackDelayAfterHitRecovery = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float HitKnockbackStrength = 420.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	float HitKnockbackUpwardStrength = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Prototype", meta = (ClampMin = "0.0"))
	float PrototypeIncomingDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace", meta = (ClampMin = "0.0"))
	float PrototypeTraceDistance = 185.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace", meta = (ClampMin = "0.0"))
	float PrototypeTraceRadius = 75.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	float PrototypeTraceHeightOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Debug")
	bool bShowCombatScreenDebugMessages = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.0"))
	float HeavyTraceDistance = 210.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Heavy", meta = (ClampMin = "0.0"))
	float HeavyTraceRadius = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	bool bShowOptionalAnimationSlots = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides))
	TObjectPtr<UAnimMontage> HeavyCounterAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides, DisplayName = "Fallback Execution Montage (Optional)", ToolTip = "Used only when Execution Variants has no valid player execution montage."))
	TObjectPtr<UAnimMontage> ExecutionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides, DisplayName = "Execution Variants (Player + Enemy Death)", ToolTip = "Cycles through paired player execution montages and enemy death montages."))
	TArray<FAetherExecutionVariant> ExecutionVariants;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides))
	TObjectPtr<UAnimMontage> AetherSlashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides))
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> DodgeForwardMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> DodgeBackwardMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> DodgeLeftMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> DodgeRightMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> GuardStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> GuardEndMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> GuardBreakMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation")
	TObjectPtr<UAnimMontage> GuardBlockHitMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (EditCondition = "bShowOptionalAnimationSlots", EditConditionHides))
	TObjectPtr<UAnimMontage> ParryAttemptMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Animation", meta = (DisplayName = "Parry Counter Montage (Success Only)"))
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> LightHitImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> HeavyHitImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> HeavyCounterHitImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> ExecutionImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> ParryImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> PlayerHitImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> GuardBlockImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	TObjectPtr<UParticleSystem> AetherSlashImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX", meta = (ClampMin = "0.01"))
	float ImpactEffectScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ImpactEffectTargetHeightAlpha = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|VFX")
	float ImpactEffectForwardOffset = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> LightHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> HeavyHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> HeavyCounterHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> ExecutionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> ParrySuccessSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> PlayerHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> GuardBlockSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio")
	TObjectPtr<USoundBase> AetherSlashHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio", meta = (ClampMin = "0.0"))
	float ImpactSoundVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ImpactSoundVolumeVariance = 0.04f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio", meta = (ClampMin = "0.1"))
	float ImpactSoundPitchMin = 0.96f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio", meta = (ClampMin = "0.1"))
	float ImpactSoundPitchMax = 1.04f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> LightAttackSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> HeavyAttackSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> HeavyCounterSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> ExecutionStartSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> AetherSlashCastSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> DodgeSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> GuardRaiseSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> GuardLowerSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> GuardBreakSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action")
	TObjectPtr<USoundBase> ParryAttemptSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action", meta = (ClampMin = "0.0"))
	float ActionSoundVolume = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActionSoundVolumeVariance = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action", meta = (ClampMin = "0.1"))
	float ActionSoundPitchMin = 0.97f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Action", meta = (ClampMin = "0.1"))
	float ActionSoundPitchMax = 1.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning")
	bool bEnablePlayerDangerWarningSounds = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (EditCondition = "bEnablePlayerDangerWarningSounds"))
	TObjectPtr<USoundBase> LowHealthWarningSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (EditCondition = "bEnablePlayerDangerWarningSounds"))
	TObjectPtr<USoundBase> CriticalHealthWarningSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (EditCondition = "bEnablePlayerDangerWarningSounds"))
	TObjectPtr<USoundBase> DefeatedWarningSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float LowHealthWarningThresholdPercent = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float CriticalHealthWarningThresholdPercent = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.0", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float WarningSoundVolume = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float WarningSoundVolumeVariance = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.1", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float WarningSoundPitchMin = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Warning", meta = (ClampMin = "0.1", EditCondition = "bEnablePlayerDangerWarningSounds"))
	float WarningSoundPitchMax = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource")
	bool bEnableAetherResourceCueSounds = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (EditCondition = "bEnableAetherResourceCueSounds"))
	TObjectPtr<USoundBase> AetherSlashReadySound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (EditCondition = "bEnableAetherResourceCueSounds"))
	TObjectPtr<USoundBase> AetherGaugeFullSound;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (ClampMin = "0.0", EditCondition = "bEnableAetherResourceCueSounds"))
	float AetherResourceSoundVolume = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableAetherResourceCueSounds"))
	float AetherResourceSoundVolumeVariance = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (ClampMin = "0.1", EditCondition = "bEnableAetherResourceCueSounds"))
	float AetherResourceSoundPitchMin = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Assets|Audio|Resource", meta = (ClampMin = "0.1", EditCondition = "bEnableAetherResourceCueSounds"))
	float AetherResourceSoundPitchMax = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.01"))
	float ImpactCameraKickDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float LightHitCameraKickStrength = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float HeavyHitCameraKickStrength = 14.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float ExecutionCameraKickStrength = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float ParryCameraKickStrength = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float PlayerHitCameraKickStrength = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float GuardBlockCameraKickStrength = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float HitStopTimeDilation = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float LightHitStopDuration = 0.035f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float HeavyHitStopDuration = 0.055f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float HeavyCounterHitStopDuration = 0.07f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float ExecutionHitStopDuration = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float ParryHitStopDuration = 0.045f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float PlayerHitStopDuration = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Feedback", meta = (ClampMin = "0.0"))
	float GuardBlockHitStopDuration = 0.035f;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	float CurrentStamina = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	float CurrentAetherGauge = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	int32 CurrentComboStep = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsAttacking = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsHeavyAttacking = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsExecuting = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsAetherSlashing = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bQueuedLightAttack = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bQueuedExecution = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsDodging = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsGuarding = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsParryWindowActive = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsParryRecovering = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsParryCounterWindowActive = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsDamageInvulnerable = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bDelayEnemyAttackAfterDamageInvulnerability = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Runtime")
	bool bIsHitReacting = false;

	double LastStaminaSpendTime = 0.0;
	double LastDodgeTime = -100.0;
	double LastAetherSlashTime = -100.0;
	FVector ActiveDodgeDirection = FVector::ZeroVector;
	float DodgeMovementElapsed = 0.0f;
	float DodgeMovementAlpha = 0.0f;
	bool bHasGuardMovementSpeedCache = false;
	float CachedPreGuardMaxWalkSpeed = 0.0f;
	int32 NextExecutionVariantIndex = 0;
	bool bExecutionImpactResolved = false;
	bool bExecutionActiveInputReported = false;
	FAetherPlayerDangerCueState PlayerDangerCueState;
	FAetherResourceCueState ResourceCueState;

	TWeakObjectPtr<AAetherfallCharacter> OwnerCharacter;
	TWeakObjectPtr<AActor> PreferredHitTarget;
	TWeakObjectPtr<AAetherEnemyBase> PendingExecutionTarget;
	TArray<FHitResult> ReusableTraceHitResults;
	FTimerHandle AttackEndTimerHandle;
	FTimerHandle ComboResetTimerHandle;
	FTimerHandle DodgeEndTimerHandle;
	FTimerHandle ParryWindowTimerHandle;
	FTimerHandle ParryRecoveryTimerHandle;
	FTimerHandle ParryCounterWindowTimerHandle;
	FTimerHandle DamageInvulnerabilityTimerHandle;
	FTimerHandle HitReactionTimerHandle;
	FTimerHandle HeavyImpactTimerHandle;
	FTimerHandle AetherSlashImpactTimerHandle;
	FTimerHandle ExecutionImpactTimerHandle;
	FTimerHandle ExecutionTimerHandle;
};
