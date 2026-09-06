#pragma once

#include "CoreMinimal.h"
#include "AetherAurelBossPhasePolicy.h"
#include "AetherEnemyActionPresentationPolicy.h"
#include "GameFramework/Character.h"
#include "AetherEnemyBase.generated.h"

class AAetherfallCharacter;
class UAetherHealthComponent;
class UAnimationAsset;
class UAnimMontage;
class USkeletalMeshComponent;
class USoundBase;
class UStaticMeshComponent;

/** 적 기본형과 보스 유형을 구분해 조정 데이터 및 행동 선택에 사용한다. */
UENUM(BlueprintType)
enum class EAetherEnemyArchetype : uint8
{
	Skirmisher,
	Vanguard,
	Brute,
	Cael,
	Aurel
};

/** 적 공격의 사거리·예고·피해·회복 시간을 데이터로 정의한다. */
USTRUCT(BlueprintType)
struct FAetherEnemyAttackPatternData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName PatternName = FName(TEXT("Standard"));

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName DefenseHintLabel = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Range = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float WindupDuration = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float RecoveryDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float RangePadding = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float TelegraphRadius = 95.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float SelectionWeight = 1.0f;
};

/** 적 유형별 이동·전투·공격 패턴의 기본 설정을 묶는다. */
USTRUCT(BlueprintType)
struct FAetherEnemyArchetypeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Archetype")
	EAetherEnemyArchetype Archetype = EAetherEnemyArchetype::Vanguard;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype")
	FName DisplayName = FName(TEXT("Vanguard"));

	UPROPERTY(EditDefaultsOnly, Category = "Archetype")
	FName CombatRoleLabel = FName(TEXT("Balanced"));

	UPROPERTY(EditDefaultsOnly, Category = "Archetype")
	FLinearColor ArchetypeColor = FLinearColor(1.0f, 0.72f, 0.25f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "0.0"))
	float MovementSpeed = 260.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "0.1"))
	float VisualScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "0.0"))
	float AttackCooldown = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "0.0"))
	float DetectionRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype", meta = (ClampMin = "0.0"))
	float StopDistance = 125.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Attack Tuning", meta = (ClampMin = "0.0"))
	float AttackStartRangeBuffer = 14.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Attack Tuning", meta = (ClampMin = "0.0"))
	float AttackWindupTurnSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Spacing", meta = (ClampMin = "0.0"))
	float EnemySpacingRadius = 155.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Spacing", meta = (ClampMin = "0.0"))
	float EnemySpacingStrength = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitReactionDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitKnockbackStrength = 280.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitKnockbackUpwardStrength = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerKnockbackStrength = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerKnockbackUpwardStrength = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Archetype|Boss")
	bool bResistAttackInterruptsDuringWindup = false;

	/** 일반 적의 가중 선택과 보스의 순차 선택에 사용하는 공격 수치 목록이다. */
	UPROPERTY(EditDefaultsOnly, Category = "Archetype")
	TArray<FAetherEnemyAttackPatternData> AttackPatterns;
};

/** 적 캐릭터의 추적, 공격 예고·판정·회복, 피격과 패리 경직, 보스 단계 및 시청각 실행을 조정한다. */
UCLASS()
class AETHERFALL_API AAetherEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAetherEnemyBase();

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FORCEINLINE UAetherHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FORCEINLINE bool IsHitReacting() const { return bIsHitReacting; }

	/** 기존 공격·피격 타이머와 공격 슬롯을 해제하고 패리 경직 및 밀려남을 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy")
	void ApplyParryStagger(AActor* StaggerCauser);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy")
	void RefreshParryStagger(float StaggerDuration);

	/** 공격 슬롯과 기존 행동을 정리하여 처형 연출 동안 적의 공격 및 추적을 억제한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy")
	void ApplyExecutionSuppression(AActor* SuppressionCauser, float SuppressionDuration);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FORCEINLINE bool IsParryStaggered() const { return bIsParryStaggered; }

	/** 원형 프로필을 찾아 전투 수치와 시각 크기를 적용하고 체력 및 보스 단계 상태를 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy")
	void ApplyEnemyArchetype(EAetherEnemyArchetype NewArchetype);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FORCEINLINE EAetherEnemyArchetype GetEnemyArchetype() const { return EnemyArchetype; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FName GetEnemyArchetypeName() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FName GetEnemyCombatRoleLabel() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FLinearColor GetEnemyArchetypeColor() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FORCEINLINE bool IsAttackWindingUpForHUD() const { return bIsAttackWindingUp; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FName GetActiveAttackPatternName() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	FName GetActiveAttackDefenseHintLabel() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	bool IsBossPhaseOne() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Enemy")
	bool IsBossPhaseTwo() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy|Execution")
	void SetPendingExecutionDeathMontage(UAnimMontage* NewDeathMontage);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Enemy|Weapon")
	void RefreshWeaponAttachment();

protected:
	virtual void BeginPlay() override;

private:
	void RefreshPrototypeVisualMode();
	/** 일반 적 세 종류와 두 보스의 기본 능력치·공격 목록을 구성한다. 실제 적용값은 원형 프로필에서 복사한다. */
	void InitializeDefaultArchetypeProfiles();
	const FAetherEnemyArchetypeData* FindArchetypeProfile(EAetherEnemyArchetype ArchetypeToFind) const;
	void UpdateTarget();
	/** 감지 범위 안의 플레이어 방향과 주변 적 분리 방향을 합쳐 이동 입력을 만든다. 경로 탐색 요청은 수행하지 않는다. */
	void ChaseTarget(float DeltaTime);
	/** 행동 상태와 공격 패턴, 재사용 시간, 동시 공격 슬롯을 확인한 뒤 공격 예고를 시작한다. */
	void TryAttackTarget();
	/** 선택한 패턴을 복사해 공격 도중 설정 변화를 분리하고, 연출과 예고 타이머를 시작한다. */
	void BeginAttackWindup(const FAetherEnemyAttackPatternData& AttackPattern);
	void UpdateAttackWindupFacing(float DeltaTime);
	/** 예고 종료 시 평면 거리를 다시 검사해 플레이어 방어 판정에 피해를 전달하고, 패리 경직이 없으면 회복 단계로 이동한다. */
	void ResolveAttack();
	void EndAttackRecovery();
	void ReleaseAttackSlot();
	void DrawAttackTelegraph() const;
	/** 보스 전환과 공격 예고 중 저항 조건을 확인하고, 중단 가능한 공격의 타이머 및 슬롯을 정리한 뒤 경직을 적용한다. */
	void BeginHitReaction(AActor* DamageCauser);
	void EndHitReaction();
	void EndParryStagger();
	void EndExecutionSuppression();
	float PlayPendingExecutionDeathMontage();
	float PlayEnemyActionMontage(UAnimMontage* Montage, const FString& Reason) const;
	FAetherEnemyActionVisualAssets BuildEnemyActionVisualAssets() const;
	FAetherEnemyActionVisualTuning BuildEnemyActionVisualTuning() const;
	FAetherEnemyActionSoundSet BuildEnemyActionSoundSet() const;
	void UpdatePrototypeLoopAnimation();
	float PlayEnemyActionAnimation(UAnimationAsset* Animation, const FString& Reason);
	/** 대체 연출 우선 설정을 존중하고 재생 실패 시 몽타주 또는 대체 애니메이션을 순서대로 시도한다. */
	float PlayEnemyActionVisual(const FAetherEnemyActionVisualSelection& VisualSelection, const FString& Reason);
	/** 대체 사망 연출 우선 설정을 확인한 뒤 처형용 사망 몽타주와 일반 사망 연출을 순서대로 시도한다. */
	float PlayEnemyDeathVisual(const FAetherEnemyActionVisualSelection& VisualSelection, const FString& Reason);
	/** 동일한 반복 애니메이션이면 재시작하지 않고 필요한 재생 속도만 갱신한다. */
	void PlayEnemyLoopAnimation(UAnimationAsset* Animation, const FString& Reason, float PlayRate);
	void ApplyPrototypeAnimationRootMotionPolicy() const;
	void LogEnemyAnimationConfiguration() const;
	void PlayEnemySound(USoundBase* Sound, float VolumeMultiplier = 1.0f) const;
	FVector GetHitReactionDirection(AActor* DamageCauser) const;
	const FAetherEnemyAttackPatternData* SelectAttackPattern(float DistanceToTarget);
	const FAetherEnemyAttackPatternData& GetActiveAttackPattern() const;
	/** 월드의 살아 있는 다른 적을 순회해 가까울수록 강한 평면 분리 방향을 누적한다. */
	FVector CalculateEnemySeparationDirection() const;
	float GetMaxAttackRange() const;
	bool IsTargetWithinAttackRange(const FAetherEnemyAttackPatternData& AttackPattern, float ExtraPadding = 0.0f) const;
	bool IsTargetAttackable(float RangePadding = 0.0f) const;
	void AnnounceAurelPhaseOneIntro();
	bool TryStartAurelPhaseTwo(float CurrentHealth, float MaxHealth, AActor* DamageCauser);
	/** 보스 정책이 허용한 전환에서 기존 공격을 취소하고 2단계 패턴·속도·재사용 시간·보상을 적용한다. */
	void BeginAurelPhaseTwo(AActor* DamageCauser, const FAetherAurelBossPhaseTransitionPlan& TransitionPlan);
	void EndAurelPhaseShift();
	void AnnounceAurelLowHealth(float CurrentHealth, float MaxHealth);
	void ResetAurelBossRuntimeState();
	FAetherAurelBossPhaseTuning BuildAurelBossPhaseTuning() const;
	void RegisterAurelBossPhaseFeedback(const FAetherAurelBossPhaseFeedback& Feedback, const FColor& DebugColor) const;
	void ShowEnemyDebugMessage(const FString& Message, const FColor& Color) const;

	UFUNCTION()
	void HandleHealthChanged(UAetherHealthComponent* ChangedHealthComponent, float CurrentHealth, float MaxHealth, AActor* DamageCauser);

	/** 모든 전투 타이머와 공격 슬롯을 정리하고 충돌·이동을 끈 뒤 사망 연출 길이를 고려해 수명을 설정한다. */
	UFUNCTION()
	void HandleDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PrototypeBody;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	bool bAutoHidePrototypeVisualsWhenSkeletalMeshAssigned = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Weapon", meta = (AllowPrivateAccess = "true"))
	FName WeaponAttachSocketName = FName(TEXT("hand_r"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float DetectionRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackDamage = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackWindupDuration = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackRecoveryDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackRangePadding = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Attack Tuning", meta = (ClampMin = "0.0"))
	float AttackStartRangeBuffer = 14.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Attack Tuning", meta = (ClampMin = "0.0"))
	float AttackWindupTurnSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float AttackTelegraphRadius = 95.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy|AI|Debug")
	bool bDrawAttackTelegraphDebug = false;

	UPROPERTY(EditAnywhere, Category = "Enemy|AI|Debug", meta = (ClampMin = "0.1", EditCondition = "bDrawAttackTelegraphDebug"))
	float AttackTelegraphDebugRadiusScale = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Enemy|AI|Debug")
	bool bShowEnemyScreenDebugMessages = false;

	/** 일반 적의 가중 선택과 보스의 순차 선택에 사용하는 공격 수치 목록이다. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Attacks")
	TArray<FAetherEnemyAttackPatternData> AttackPatterns;

	/** 아우렐의 2단계에서 사용할 별도 공격 목록이다. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel")
	TArray<FAetherEnemyAttackPatternData> AurelPhaseTwoAttackPatterns;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.05", ClampMax = "0.95"))
	float AurelPhaseTwoHealthThresholdPercent = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.0"))
	float AurelPhaseShiftDuration = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.0"))
	float AurelPhaseShiftAetherReward = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.1"))
	float AurelPhaseTwoAttackCooldownMultiplier = 0.78f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.0"))
	float AurelPhaseTwoMovementSpeed = 255.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Boss|Aurel", meta = (ClampMin = "0.05", ClampMax = "0.95"))
	float AurelLowHealthWarningPercent = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Archetype", meta = (AllowPrivateAccess = "true"))
	EAetherEnemyArchetype EnemyArchetype = EAetherEnemyArchetype::Vanguard;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Archetype")
	TArray<FAetherEnemyArchetypeData> ArchetypeProfiles;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI", meta = (ClampMin = "0.0"))
	float StopDistance = 125.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Spacing")
	bool bUseEnemySpacing = true;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Spacing", meta = (ClampMin = "0.0"))
	float EnemySpacingRadius = 155.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI|Spacing", meta = (ClampMin = "0.0"))
	float EnemySpacingStrength = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitReactionDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitKnockbackStrength = 280.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
	float HitKnockbackUpwardStrength = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerKnockbackStrength = 180.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Parry", meta = (ClampMin = "0.0"))
	float ParryStaggerKnockbackUpwardStrength = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> QuickAttackWindupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> StandardAttackWindupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> HeavyAttackWindupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> AttackHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> AttackMissSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> HitReactionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> ParryStaggerSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio")
	TObjectPtr<USoundBase> DeathSound;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> QuickAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> StandardAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> ParryStaggerMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	bool bUsePrototypeEnemyAnimationDriver = true;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "0.0"))
	float PrototypeMoveAnimationSpeedThreshold = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	bool bPreferPrototypeFallbackActionAnimations = true;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "1.0"))
	float PrototypeMoveAnimationReferenceSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "0.05"))
	float PrototypeMoveAnimationMinPlayRate = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "0.05"))
	float PrototypeMoveAnimationMaxPlayRate = 0.80f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "0.05"))
	float PrototypeIdleAnimationPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback", meta = (ClampMin = "0.05"))
	float PrototypeActionAnimationPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> MoveAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> QuickAttackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> StandardAttackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> HeavyAttackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> HitReactionAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> ParryStaggerAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Animation|Fallback")
	TObjectPtr<UAnimationAsset> DeathAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio", meta = (ClampMin = "0.0"))
	float EnemySoundVolume = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemySoundVolumeVariance = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio", meta = (ClampMin = "0.1"))
	float EnemySoundPitchMin = 0.94f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Assets|Audio", meta = (ClampMin = "0.1"))
	float EnemySoundPitchMax = 1.06f;

	TWeakObjectPtr<AAetherfallCharacter> TargetCharacter;
	FAetherEnemyAttackPatternData ActiveAttackPattern;
	double LastAttackTime = -100.0;
	bool bIsDead = false;
	bool bIsAttackWindingUp = false;
	bool bIsAttackRecovering = false;
	bool bIsHitReacting = false;
	bool bIsParryStaggered = false;
	bool bIsExecutionSuppressed = false;
	bool bResistAttackInterruptsDuringWindup = false;
	FAetherAurelBossPhasePolicy AurelBossPhase;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> PendingExecutionDeathMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentPrototypeAnimation;

	bool bCurrentPrototypeAnimationLooping = false;
	float CurrentPrototypeAnimationPlayRate = 1.0f;

	FTimerHandle AttackWindupTimerHandle;
	FTimerHandle AttackRecoveryTimerHandle;
	FTimerHandle HitReactionTimerHandle;
	FTimerHandle ParryStaggerTimerHandle;
	FTimerHandle ExecutionSuppressionTimerHandle;
	FTimerHandle AurelPhaseShiftTimerHandle;
};
