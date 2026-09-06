/** 획득·반환 후 같은 투사체가 재사용되고 사용 가능 개수가 유지되는지 확인하는 비배포용 실행 검증 명령이다. */
#include "AetherProjectilePoolSubsystem.h"
#include "AetherSlashProjectile.h"

#if !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherPoolingValidation, Log, All);

namespace
{
struct FAetherPoolingValidationState
{
	bool bFailed = false;
};

FAetherPoolingValidationState ValidationState;

void RecordCheck(bool bCondition, const TCHAR* CheckName)
{
	ValidationState.bFailed |= !bCondition;
	UE_LOG(
		LogAetherPoolingValidation,
		Display,
		TEXT("[AetherPoolingValidation] %s: %s"),
		bCondition ? TEXT("PASS") : TEXT("FAIL"),
		CheckName);
}

UWorld* FindRuntimeWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* Candidate = WorldContext.World();
		if (Candidate && (WorldContext.WorldType == EWorldType::Game || WorldContext.WorldType == EWorldType::PIE))
		{
			return Candidate;
		}
	}

	return nullptr;
}

/** 실행 월드의 풀에서 투사체를 두 차례 획득·반환하고 인스턴스 동일성을 결과 로그로 남긴다. */
void RunPoolingRuntimeValidation()
{
	ValidationState = FAetherPoolingValidationState();

	UWorld* World = FindRuntimeWorld();
	RecordCheck(World != nullptr, TEXT("runtime world is available"));
	if (!World)
	{
		return;
	}

	UAetherProjectilePoolSubsystem* ProjectilePool = World->GetSubsystem<UAetherProjectilePoolSubsystem>();
	RecordCheck(ProjectilePool != nullptr, TEXT("projectile pool subsystem is available"));
	if (!ProjectilePool)
	{
		return;
	}

	APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
	const FVector SpawnLocation = PlayerPawn ? PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 120.0f : FVector::ZeroVector;
	const FRotator SpawnRotation = PlayerPawn ? PlayerPawn->GetActorRotation() : FRotator::ZeroRotator;

	AAetherSlashProjectile* FirstProjectile = ProjectilePool->AcquireAetherSlashProjectile(PlayerPawn, PlayerPawn, SpawnLocation, SpawnRotation);
	RecordCheck(FirstProjectile != nullptr, TEXT("first Aether Slash projectile acquired"));
	if (!FirstProjectile)
	{
		return;
	}

	FirstProjectile->InitializeSlash(PlayerPawn, nullptr, SpawnRotation.Vector(), 1.0f, 100.0f, 150.0f, 32.0f);
	ProjectilePool->ReleaseAetherSlashProjectile(FirstProjectile);
	RecordCheck(FirstProjectile->IsAvailableForPool(), TEXT("released projectile is marked available"));

	AAetherSlashProjectile* SecondProjectile = ProjectilePool->AcquireAetherSlashProjectile(PlayerPawn, PlayerPawn, SpawnLocation, SpawnRotation);
	RecordCheck(SecondProjectile == FirstProjectile, TEXT("pool reuses released Aether Slash projectile"));
	if (SecondProjectile)
	{
		ProjectilePool->ReleaseAetherSlashProjectile(SecondProjectile);
	}

	RecordCheck(ProjectilePool->GetAvailableAetherSlashProjectileCount() >= 1, TEXT("pool keeps at least one projectile available"));

	UE_LOG(
		LogAetherPoolingValidation,
		Display,
		TEXT("[AetherPoolingValidation] RESULT: %s"),
		ValidationState.bFailed ? TEXT("FAIL") : TEXT("PASS"));
}
}

FAutoConsoleCommand RunPoolingValidationCommand(
	TEXT("Aether.Pooling.ValidateRuntime"),
	TEXT("Validates Aetherfall projectile pooling reuse contracts in PIE or game runtime."),
	FConsoleCommandDelegate::CreateStatic(&RunPoolingRuntimeValidation));

#endif
