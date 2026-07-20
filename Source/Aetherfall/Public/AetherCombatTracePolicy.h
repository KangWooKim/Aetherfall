#pragma once

#include "CoreMinimal.h"

class AActor;

struct FAetherCombatTraceRequest
{
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	float Radius = 1.0f;
	float DebugDuration = 0.1f;
	FColor HitColor = FColor::Green;
	FColor MissColor = FColor::Orange;
	float DebugLineThickness = 3.0f;
	float DebugSphereThickness = 2.0f;
	int32 DebugSphereSegments = 16;

	FColor GetDebugColor(bool bHit) const { return bHit ? HitColor : MissColor; }
};

class AETHERFALL_API FAetherCombatTracePolicy
{
public:
	static FAetherCombatTraceRequest BuildMeleeSphereTrace(
		const AActor* TraceOwner,
		float HeightOffset,
		float TraceDistance,
		float TraceRadius,
		float DebugDuration,
		const FColor& HitColor,
		const FColor& MissColor,
		float DebugLineThickness,
		float DebugSphereThickness,
		int32 DebugSphereSegments);
};
