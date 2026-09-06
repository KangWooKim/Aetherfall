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

/** 플레이어 처형과 적 사망 몽타주를 한 쌍으로 정의하여 순차 선택에 사용한다. */
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

/** 플레이어의 전투 행동과 자원, 판정·애니메이션·피드백을 연결하는 컴포넌트다. 조건 계산은 정책 객체에 맡기고 월드와 Actor에 대한 실행은 이곳에서 수행한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherCombatComponent();

	/** 연속 처리가 필요한 회피 이동과 스태미나 회복만 갱신한다. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 행동 허용 판정을 통과하면 약공격을 시작한다. 공격 도중 입력은 다음 콤보 예약으로 기록한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartLightAttack();

	/** 강공격의 상태 충돌을 검사한 뒤 비용과 판정 시간을 적용하는 실행 경로로 넘긴다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartHeavyAttack();

	/** 유효한 처형 대상을 확인하고 주변 적 억제, 연출, 타격·종료 타이머를 함께 시작한다. 공격 중 요청은 대상이 있을 때 예약한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartExecution();

	/** 행동과 재사용 시간을 검사한 뒤 게이지를 소비한다. 참격 상태와 바라보는 방향을 설정하고 판정 실행을 준비한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartAetherSlash();

	/** 비용과 재사용 조건을 통과하면 입력 방향을 저장하고 회피를 시작한다. 루트 모션, 시간 기반 변위, 발사 이동 중 설정된 경로를 사용한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartDodge();

	/** 방어 상태로 전환하고 이동 속도 보정을 적용한다. 방어 비용은 실제 피격 시 계산한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StartGuard();

	/** 현재 방어 중일 때만 해제하고 방어 전 이동 속도를 복원한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void StopGuard();

	/** 비용을 소비하고 패링 판정 시간을 연다. 시간이 끝나면 실패 후 회복 상태로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void TryParry();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void SimulateIncomingHit();

	/** 사망·회복 무적·처형 보호를 먼저 검사한다. 이후 패링 성공, 방어 및 방어 파괴, 일반 피해 순서로 처리한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void ReceiveIncomingHit(float DamageAmount, AActor* DamageCauser);

	/** 전투 지연 작업과 상태를 정리하고 체력·자원·재사용 시간·알림 상태를 재도전용 값으로 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void ResetPlayerPrototypeState();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat")
	void GrantPrototypeAetherGauge(float Amount, const FString& Reason);

	/** 알림 기반 판정이 켜지고 현재 약공격일 때만 해당 콤보의 근접 판정을 실행한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleLightAttackHitNotify();

	/** 강공격 상태의 애니메이션 알림을 근접 판정으로 연결하고 남은 판정 타이머를 해제한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleHeavyAttackHitNotify();

	/** 참격 상태의 발사 알림을 투사체 생성 경로로 연결하고 남은 발사 타이머를 해제한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Combat|Animation")
	void HandleAetherSlashFireNotify();

	/** 처형이 아직 해결되지 않았을 때 대체 판정 타이머를 취소하고 공통 타격 해결 경로를 호출한다. */
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

	/** 회복한 체력에 맞춰 위험 알림의 재생 상태를 갱신한다. */
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

	/** 패리 경직, 생존, 체력 비율, 평면 거리를 모두 만족하는 적만 처형 대상으로 인정한다. */
	bool IsEnemyExecutionReady(const AAetherEnemyBase* Enemy) const;

protected:
	/** 소유 캐릭터와 사망 이벤트를 연결하고 전투 자원의 초기값을 적용한다. */
	virtual void BeginPlay() override;

private:
	FAetherCombatActionStateSnapshot BuildCombatActionStateSnapshot() const;
	FAetherCombatActionRuntimeFlags BuildCombatActionRuntimeFlags() const;
	void ApplyCombatActionRuntimeFlags(const FAetherCombatActionRuntimeFlags& RuntimeFlags);
	/** 새 행동 모드가 요구하는 플래그 묶음을 한 번에 적용한다. 타이머 해제와 자원 변경은 별도 계획으로 처리한다. */
	void SetCombatActionMode(EAetherCombatActionMode Mode);
	EAetherCombatActionMode GetCombatActionMode() const;
	/** 정책이 지정한 타이머만 TimerManager에서 해제해 전환 후 오래된 지연 콜백이 남지 않게 한다. */
	void ApplyCombatActionTimerClearPlan(const FAetherCombatActionTimerClearPlan& TimerClearPlan);
	/** 적용 가능한 계획에 지정된 자원과 소비 시각만 갱신한다. 정책의 계산과 실제 멤버 변경을 연결하는 지점이다. */
	void ApplyCombatResourceMutationPlan(const FAetherCombatResourceMutationPlan& ResourceMutationPlan);
	/** 다음 콤보의 비용을 확인해 소비하고, 설정에 따라 즉시 판정하거나 애니메이션 알림을 기다린다. */
	void BeginLightAttack();
	/** 반격 가능 여부를 보관한 뒤 비용을 소비한다. 실행 계획에 맞춰 강공격 연출과 타격·종료 시점을 연결한다. */
	void BeginHeavyAttack();
	/** 아직 해결되지 않은 유효한 처형 대상이 있으면 종료 시 마지막 대체 판정을 시도한 뒤 처형 상태를 정리한다. */
	void EndExecution();
	/** 공격 상태와 지연 타격을 정리한다. 예약된 처형을 먼저 처리하고, 그다음 약공격 연속 입력 또는 콤보 초기화 지연을 처리한다. */
	void EndCurrentAttack();
	void EndDodge();
	void EndParryWindow();
	void EndParryRecovery();
	void EndParryCounterWindow();
	void OpenParryCounterWindow();
	/** 피해 원인을 우선 타격 대상으로 보관하고 별도 반격 비용 소비 없이 강공격 상태의 자동 반격을 예약한다. */
	void BeginAutomaticParryCounter(AActor* CounterTarget);
	/** 피격 회복 무적을 켜고 종료 타이머를 갱신한다. 회피 시작과는 별도의 보호 상태다. */
	void BeginDamageInvulnerability();
	/** 회복 무적을 해제하고 요청된 경우 GameMode를 통해 적의 다음 공격을 추가 지연한다. */
	void EndDamageInvulnerability();
	/** 진행 중 행동과 예약 판정을 중단하고 넉백을 적용한다. 기존 회복 무적 종료 타이머는 유지한다. */
	void BeginHitReaction(AActor* DamageCauser);
	void EndHitReaction();
	FVector GetHitReactionDirection(AActor* DamageCauser) const;
	/** 완화 곡선의 이번 프레임 증가량만큼 이동한다. 스윕이 장애물에 막히면 남은 코드 기반 회피 변위를 소진 처리한다. */
	void UpdateDodgeMovement(float DeltaTime);
	void ClearDodgeMovementState();
	void ResetCombo();
	/** 행동 정책이 회복을 허용할 때 프레임 시간에 따른 회복 계획을 적용한다. */
	void RegenerateStamina(float DeltaTime);
	void AddAetherGauge(float Amount, const FString& Reason);
	/** 양수 비용은 잔량을 확인한 뒤 차감하고 자원 알림 상태를 갱신한다. 부족하면 상태를 바꾸지 않고 실패를 반환한다. */
	bool SpendAetherGauge(float Amount, const FString& Reason);
	/** 피해 적용 성공 후 피드백과 위험 알림을 갱신하고, 살아 있으면 피격 후 무적 및 선택적 경직을 시작한다. */
	bool ApplyIncomingDamage(float DamageAmount, AActor* DamageCauser, bool bTriggerHitReaction = true);
	/** 사망·재시작 시 타이머와 예약 입력, 처형 대상, 이동 보정, 무적 상태를 함께 정리한다. */
	void ClearCombatRuntimeState();
	bool IsOwnerDead() const;
	/** 소유 캐릭터를 제외하는 구 스윕으로 약공격 후보를 수집하고 실제 피해 대상 선택 단계로 넘긴다. */
	void PerformPrototypeTrace(int32 ComboStep);
	/** 이전 판정 결과만 비우고 예상 후보 수를 위한 용량을 확보한다. */
	void PrepareReusableTraceHitResults(int32 ExpectedHitCount);
	/** 락온된 피격 대상을 우선하여 한 명만 공격하고, 우선 대상이 없으면 선택된 나머지 대상에 피해를 적용한다. */
	void ApplyPrototypeDamage(const TArray<FHitResult>& HitResults, int32 ComboStep);
	AActor* GetLockedCombatTarget() const;
	/** 체력 컴포넌트가 피해를 수락한 경우에만 명중 게이지와 타격 연출을 지급한다. */
	bool ApplyPrototypeDamageToActor(AActor* TargetActor, int32 ComboStep, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage);
	/** 강공격이 여전히 활성 상태일 때 구 스윕을 수행하고 경직 보너스를 포함하는 피해 경로로 넘긴다. */
	void PerformHeavyAttackTrace();
	/** 피격 목록에서 우선 대상을 선택하고, 패리 경직 보너스와 명중 보상을 실제 피해 적용 결과에 따라 처리한다. */
	void ApplyHeavyAttackDamage(const TArray<FHitResult>& HitResults);
	bool ApplyHeavyDamageToActor(AActor* TargetActor, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage, bool bStaggerBonus);
	void RefreshExecutionOpportunityAfterHeavyCounter(AActor* TargetActor, bool bStaggerBonus) const;
	/** 월드의 투사체 풀에서 검기를 획득하고, 풀을 사용할 수 없으면 직접 생성하여 발사 설정을 전달한다. */
	void PerformAetherSlashTrace();
	/** 처형 가능한 락온 대상을 우선하고, 없으면 평면 거리상 가장 가까운 적을 찾는다. */
	AAetherEnemyBase* FindExecutionTarget() const;
	const FAetherExecutionVariant* SelectExecutionVariant();
	/** 피해 이벤트 전에 처리 완료를 기록하고 보류 대상을 비워, 알림·타이머·종료 경로의 중복 피해를 방지한다. */
	void ResolveExecutionImpact();
	void HandleExecutionImpactFallback();
	/** 처형 대체 타이머와 보류 대상을 해제하고, 아직 타격하지 않은 적의 예정 사망 연출을 취소한다. */
	void ClearPendingExecutionImpact();
	/** 처형 대상과 죽은 적을 제외한 주변 적의 공격을 지정 시간 동안 억제한다. */
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
	/** 방어 전 최대 이동 속도를 한 번 보관한 뒤 보정 배율을 적용해 반복 호출 시 속도가 중복 감소하지 않게 한다. */
	void ApplyGuardMovementModifier();
	/** 보관된 방어 전 이동 속도를 복원하고 캐시 사용 상태를 해제한다. */
	void RestoreGuardMovementModifier();
	/** 행동별 정지 설정을 존중하며 입력 방향과 현재 이동 속도를 정리한다. */
	void StopOwnerMovementForCombatAction(bool bRespectActionSetting = true);
	/** 전투 종료 상태에 맞춰 이동 모드를 바꾸고, 이동 잠금 시 락온도 해제한다. */
	void SetOwnerMovementEnabled(bool bEnabled);
	void ShowCombatDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;
	void PlayImpactCameraFeedback(float Strength, const FString& Reason) const;
	/** 타격 종류에 맞는 카메라·시청각 효과를 재생하고 플레이어와 대상에 개별 히트 스톱을 적용한다. */
	void PlayImpactFeedback(EAetherCombatFeedbackType FeedbackType, float CameraStrength, float HitStopDuration, const FString& Reason, AActor* ImpactTarget = nullptr) const;
	/** 약한 참조로 액터의 생존을 확인한 뒤 타이머에서 시간 배율을 복원한다. */
	void ApplyHitStopToActor(AActor* Actor, float Duration) const;
	/** 메시, 스켈레톤 호환성, 애니메이션 인스턴스를 확인한 후 몽타주를 재생한다. */
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
	/** 설정 에셋의 비어 있지 않은 몽타주 목록을 우선하며, 콤보 범위를 벗어나면 마지막 항목을 사용한다. */
	UAnimMontage* GetLightAttackMontage(int32 ComboStep) const;
	/** 월드 회피 방향을 캐릭터 전후좌우 축과 비교하여 방향별 몽타주를 선택한다. */
	UAnimMontage* GetDodgeMontageForDirection(const FVector& WorldDirection) const;
	FAetherCombatFeedbackAssets BuildCombatFeedbackAssets() const;
	/** 타격 종류에 대응하는 효과와 효과음만 선택해 대상의 충격 위치에서 재생한다. */
	void PlayFeedbackAssets(EAetherCombatFeedbackType FeedbackType, AActor* ImpactTarget) const;
	FVector GetFeedbackImpactLocation(AActor* ImpactTarget) const;

	/** 전투 상태와 이동을 정리한 뒤 게임 모드에 패배 후 재시작 예약을 요청한다. */
	UFUNCTION()
	void HandleOwnerDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser);

	/** 행동별 덮어쓰기가 활성화된 데이터만 사용한다. 나머지는 이 컴포넌트의 기본 설정을 따른다. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Authoring")
	TObjectPtr<UAetherCombatActionDataAsset> CombatActionDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "1"))
	int32 MaxComboSteps = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "0.1"))
	float LightAttackDuration = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", meta = (ClampMin = "0.0"))
	float ComboResetDelay = 0.85f;

	/** 활성화하면 애니메이션에서 해당 타격 알림 API를 호출해야 한다. */
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

	/** 루트 모션이 회피 변위를 담당할 때 코드 기반 회피 이동을 적용하지 않도록 구분한다. */
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

	/** 플레이어 처형과 적 사망 몽타주를 짝지어 선택하는 연출 데이터다. */
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

	/** 실제 자원 변경은 자원 정책의 결과를 적용하는 함수에 모은다. */
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

	/** 외부 Actor를 소유하지 않도록 약한 참조로 추적한다. */
	TWeakObjectPtr<AAetherfallCharacter> OwnerCharacter;
	TWeakObjectPtr<AActor> PreferredHitTarget;
	TWeakObjectPtr<AAetherEnemyBase> PendingExecutionTarget;
	/** 근접 판정마다 원소를 비우고 할당 용량은 재사용하는 결과 버퍼다. */
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
