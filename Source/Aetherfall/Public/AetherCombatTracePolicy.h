#pragma once

#include "CoreMinimal.h"

class AActor;

/** 전투 스윕의 시작·끝 위치와 반경, 디버그 표시값을 전달한다. */
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

/** 근접 공격의 구형 스윕 시작점·끝점·반경과 디버그 표시값을 계산한다. 실제 월드 충돌 검사는 호출자가 수행한다. */
class AETHERFALL_API FAetherCombatTracePolicy
{
public:
	/** 공격자 위치와 전방으로 검사 구간을 만들고, 반경·길이·디버그 설정의 최소 범위를 보정한다. */
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
