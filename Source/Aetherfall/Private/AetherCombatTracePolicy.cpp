#include "AetherCombatTracePolicy.h"

#include "GameFramework/Actor.h"

FAetherCombatTraceRequest FAetherCombatTracePolicy::BuildMeleeSphereTrace(
	const AActor* TraceOwner,
	float HeightOffset,
	float TraceDistance,
	float TraceRadius,
	float DebugDuration,
	const FColor& HitColor,
	const FColor& MissColor,
	float DebugLineThickness,
	float DebugSphereThickness,
	int32 DebugSphereSegments)
{
	FAetherCombatTraceRequest Request;
	Request.Radius = FMath::Max(1.0f, TraceRadius);
	Request.DebugDuration = FMath::Max(0.0f, DebugDuration);
	Request.HitColor = HitColor;
	Request.MissColor = MissColor;
	Request.DebugLineThickness = FMath::Max(0.0f, DebugLineThickness);
	Request.DebugSphereThickness = FMath::Max(0.0f, DebugSphereThickness);
	Request.DebugSphereSegments = FMath::Max(4, DebugSphereSegments);

	if (!TraceOwner)
	{
		return Request;
	}

	const FVector Forward = TraceOwner->GetActorForwardVector().GetSafeNormal();
	const FVector Direction = Forward.IsNearlyZero() ? FVector::ForwardVector : Forward;
	Request.Start = TraceOwner->GetActorLocation() + FVector(0.0f, 0.0f, HeightOffset);
	Request.End = Request.Start + Direction * FMath::Max(0.0f, TraceDistance);
	return Request;
}
