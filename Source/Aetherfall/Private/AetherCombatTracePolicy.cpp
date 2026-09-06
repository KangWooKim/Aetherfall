/** 근접 공격의 구형 스윕 시작점·끝점·반경과 디버그 표시값을 계산한다. 실제 월드 충돌 검사는 호출자가 수행한다. */
#include "AetherCombatTracePolicy.h"

#include "GameFramework/Actor.h"

/** 공격자 위치와 전방으로 검사 구간을 만들고, 반경·길이·디버그 설정의 최소 범위를 보정한다. */
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
