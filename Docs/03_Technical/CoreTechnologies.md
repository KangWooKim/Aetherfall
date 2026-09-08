# Aetherfall 핵심 기술과 코드

[프로젝트 개요](ProjectOverview.md) · [전체 소스 색인](SourceIndex.md) · [README](../../README.md)

공개 소스 기준은 `c603eedb7ab0c396de6c8123bf010a7e85e633b5`입니다. 소스 링크는 이 커밋에 고정합니다.

코드 블록은 표시된 연속 구간을 그대로 발췌합니다. 기존 GIF는 전투·복원·설정 동작을 설명하는 시연 화면입니다. 화면만으로 객체 재사용, 내부 처리 시간이나 현재 버전의 실행 성능을 입증하지는 않습니다.

## 1. 전투 판단과 실행의 분리

![약공격 연계와 강공격](Media/combat-actions.gif)

입력은 [PlayerController의 행동 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPlayerController.cpp#L290)에서 전투 컴포넌트로 전달됩니다. `UAetherCombatComponent`가 행동 플래그, 예약 입력, 자원, 타이머와 몽타주 실행을 소유합니다. 분리된 계산 함수는 이 상태의 스냅샷을 받거나 적용할 값을 돌려줍니다.

[약공격 허용 판단](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatActionGatePolicy.cpp#L14)은 사망·방어·피격·처형·강공격·검기·회피를 먼저 검사합니다. 그 조건을 통과한 뒤 이미 약공격 중일 때에만 후속 입력 예약을 반환합니다.

```cpp
	if (State.bAttacking)
	{
		FAetherCombatActionGateResult Result = Blocked(TEXT("Light attack buffered"));
		Result.bShouldQueueLightAttack = true;
		return Result;
	}

```

[실제 코드: AetherCombatActionGatePolicy.cpp 51–57줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatActionGatePolicy.cpp#L51-L57)

[호출자](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L239)가 예약 플래그를 기록하고 반환합니다. [공격 종료](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L925)에서는 예약된 처형을 먼저 시도합니다. 약공격 예약은 직전 행동이 약공격이고 다음 콤보 단계가 남은 경우에 처리합니다. 예약은 입력마다 쌓이는 큐가 아니라 종류별 `bool`입니다.

| 구분 | 계산하거나 판단하는 부분 | 실제 적용 위치와 주의점 |
| --- | --- | --- |
| 행동 상태 | [StatePolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatActionStatePolicy.cpp#L1)가 모드에 맞는 플래그를 구성합니다. | 컴포넌트가 플래그를 저장합니다. 모든 상태 전이를 한 전이표로 제한하는 구조는 아닙니다. |
| 실행 시점 | [ExecutionPolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatActionExecutionPolicy.cpp#L1)가 Notify 대기, 대체 타격과 종료 시간을 계산합니다. | 컴포넌트가 월드 타이머와 몽타주를 실행합니다. |
| 중단 정리 | [TimerPolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatActionTimerPolicy.cpp#L1)가 중단 이유별 해제 대상을 구성합니다. | [적용 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L155)가 실제 핸들을 해제합니다. |
| 자원 | [ResourcePolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatResourcePolicy.cpp#L1)가 변경량을 계산합니다. | [컴포넌트](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L215)가 값을 갱신합니다. 스태미나 지출 계산 자체는 부족량을 거부하지 않으므로 시작 함수의 비용 검사가 필요합니다. |
| 타격 | [TracePolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatTracePolicy.cpp#L1)가 스윕 기하를 구성하고 [TargetSelectionPolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatTargetSelectionPolicy.cpp#L1)가 적중 후보를 고릅니다. | 월드 스윕과 피드백은 호출자가 실행합니다. 우선 대상이 선택되면 추가 대상 배열은 채우지 않습니다. |

`Policy`라는 이름이 순수 계산을 뜻하지는 않습니다. [DamagePolicy](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatDamagePolicy.cpp#L1)는 체력 컴포넌트를 찾아 실제로 `ApplyDamage`를 호출합니다. [HealthComponent](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherHealthComponent.cpp#L17)는 체력을 변경하고 `OnHealthChanged`, 사망 조건에 해당하면 `OnDeath`를 방송합니다. 체력 변경 이벤트 시점에는 사망 플래그 갱신이 아직 뒤에 있으므로 임의의 재진입까지 원자적으로 처리한다고 설명할 수 없습니다.

행동별 수치는 [CombatActionDataAsset](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Public/AetherCombatActionDataAsset.h#L1)에서 선택적으로 덮어씁니다. [약공격 비용 배열](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L1721)이 비어 있으면 컴포넌트 기본 배열로 돌아갑니다. 데이터의 수치 변경과 새로운 행동 종류 추가는 다릅니다. 새 행동에는 입력, 허용 조건, 상태·타이머 정리, 실행과 종료 경로를 함께 연결해야 합니다.

## 2. 처형 타격의 중복 처리 방지

[처형 시작](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L269)은 살아 있는 대상의 패리 경직, 체력 비율, 평면 거리를 확인합니다. 락온 대상이 조건을 만족하면 우선 사용하고, 아니면 월드의 적 중 가까운 후보를 고릅니다. 별도의 시야 가림 검사는 없습니다.

시작할 때 대기 대상을 저장하고 `bExecutionImpactResolved`를 `false`로 초기화합니다. Notify, 대체 타이머, 행동 종료의 보완 경로가 같은 [타격 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L1580)로 모입니다.

```cpp
	if (bExecutionImpactResolved)
	{
		return;
	}

	// 피해 콜백이 처형 종료 경로를 다시 호출해도 같은 타격이 반복되지 않도록 먼저 확정한다.
	bExecutionImpactResolved = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionImpactTimerHandle);
	}

	AAetherfallCharacter* Character = OwnerCharacter.Get();
	// 현재 대상은 지역 변수에 보관하고 보류 참조는 피해 이벤트 전에 비운다.
	AAetherEnemyBase* ExecutionTarget = PendingExecutionTarget.Get();
	PendingExecutionTarget.Reset();
```

[실제 코드: AetherCombatComponent.cpp 1582–1597줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L1582-L1597)

위 순서로 일반적인 중복 호출을 차단하고 대기 대상을 비운 후 피해를 적용합니다. 대상이나 소유자가 사라진 경우에도 그 시도는 소비되며 자동 재시도하지 않습니다. [정리 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L1635)는 핸들과 대상 참조를 해제하고 플래그를 다시 초기화합니다.

보장 범위는 **현재 처형의 대기 타격**입니다. 행동 세대를 구분하는 ID는 없습니다. 이벤트 수신자가 행동을 끝내고 새 처형을 시작하는 경우나 이전 애니메이션의 늦은 Notify까지 모두 구분하는 장치는 아닙니다. 약공격·강공격·검기 Notify에도 이 플래그가 공통 적용되는 것은 아닙니다.

## 3. 저장 스냅샷과 복원 순서

![체크포인트의 위치와 체력 복원](Media/checkpoint-restore.gif)

### 저장 데이터와 현재 액터의 분리

`AAetherGameModeBase`가 체크포인트, 완료 조우, 획득물과 해금 라벨을 소유합니다. [스냅샷 작성](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeCheckpointSnapshot.cpp#L41)이 이를 [SaveGame 데이터](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Public/AetherPrototypeSaveGame.h#L1)로 옮기고 [저장 서비스](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSaveSubsystem.cpp#L9)가 동기 `SaveGameToSlot`을 호출합니다. API의 반환은 저장 호출의 결과이며 디스크 내구성이나 다중 슬롯 트랜잭션을 보장하는 별도 구현은 없습니다.

현재 형식 번호는 **2**, 설명 라벨은 `PrototypeCheckpointV2`입니다. [로드 계획](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeSaveSchemaPolicy.cpp#L19)은 미래 버전을 거부하고 이전 버전을 보정 대상으로 분류합니다. 실제 판정은 정수 버전으로 수행합니다. 문자열 라벨 비교나 CRC 검사로 파일 전체의 무결성을 보증하는 구조는 아닙니다.

[읽기](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeCheckpointSnapshot.cpp#L70)는 체크포인트 랭크를 보완하고 마지막 완료 조우를 완료 집합에 합칩니다. 체력·회복 아이템, 진행 집합과 대화 재생 라벨을 옮기지만 모든 액터의 위치·AI·타이머를 직렬화하지는 않습니다. 라벨 집합을 배열로 쓸 때 정렬하지도 않습니다. 저장 대상을 추가하려면 저장 필드, 런타임 상태, 양방향 변환과 해당 액터의 복원 API를 함께 확인해야 합니다.

### 플레이어 복원 전 저장 요청의 처리

[로드](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherGameModeBase.cpp#L941)와 [재도전 준비](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherGameModeBase.cpp#L1083)는 스냅샷을 읽은 뒤 `bDeferPrototypeCheckpointSaveUntilLoadedStateApplied`를 설정합니다. 저장 함수는 이 플래그가 켜져 있으면 다음과 같이 반환합니다.

[저장 거부 분기](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherGameModeBase.cpp#L978-L985)에서 안내 후 반환하며 요청 데이터를 보관하지 않습니다.

즉, 복원 중 저장 요청을 **보관하지 않고 거부합니다**. [플레이어 적용](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherGameModeBase.cpp#L1016)이 위치·회전·체력·아이템을 복원한 다음 플래그를 끕니다. 앞서 거부한 요청을 다시 실행하는 단계는 없으며 다음 저장 이벤트가 필요합니다. 플레이어가 없어 조기 반환하면 이 플래그도 유지됩니다.

### 진행 라벨을 이용한 월드 상태 복원

최초 맵 로드와 같은 월드에서의 재도전은 진입 순서가 다릅니다.

```mermaid
flowchart TD
    A[최초 맵 진입 InitGame] --> B[슬롯 로드 · GameMode 진행 상태 설정]
    B --> C[각 진행 액터 BeginPlay에서 저장 라벨 반영]
    B --> D[GameMode BeginPlay 이후 다음 틱에 플레이어 적용]
    E[같은 월드에서 재도전] --> F[기존 조우 상태 보관 · 저장 스냅샷 읽기]
    F --> G[전투 정리 · 필요한 보스 및 라운드 초기화]
    G --> H[WorldRestorer로 현재 액터 복원]
    H --> I[플레이어 적용 · 저장 거부 플래그 해제]
```

[WorldRestorer](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeCheckpointWorldRestorer.cpp#L1)는 한 번의 액터 순회로 종류별 배열을 구성한 뒤 키 → 보상 → 기록물 → 레버 → 진행 게이트 → 열쇠 게이트 → 상자 → 조우 → 목표 순으로 복원합니다. 각 호출 전 유효성을 확인하며 다음 복원을 위해 액터 목록을 계속 보관하지 않습니다. 초기 맵 진입에서는 이 함수를 일괄 호출하는 대신 액터별 `BeginPlay`가 GameMode 상태를 읽습니다.

레버 복원은 활성 상태만 바꿉니다. 평상시 [레버 작동](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeLever.cpp#L60)이 여는 대상은 에디터의 `TargetGates`에 지정한 진행 게이트입니다. 복원 때에는 게이트가 자기 라벨과 조건으로 별도 복원됩니다. 같은 의미의 저장 라벨을 여러 독립 대상에 재사용하면 상태를 구분할 수 없으므로 콘텐츠 배치도 이 모델에 맞아야 합니다.

### 이어하기와 새 게임

[저장 요약](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSaveSubsystem.cpp#L37)은 슬롯 존재뿐 아니라 실제 로드·형식 지원·활성 체크포인트를 확인합니다. [이어하기](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherMenuFlowSubsystem.cpp#L25)가 검사하는 `bLoadable`은 기존 저장의 **로드 가능 여부**이며 저장 쓰기 권한이 아닙니다.

[새 게임](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherMenuFlowSubsystem.cpp#L46)은 기존 슬롯이 있으면 덮어쓰기 확인을 요구합니다. 확인 후 삭제가 성공해야 맵 열기를 요청합니다. 맵 전환이 뒤늦게 실패했을 때 삭제된 슬롯을 자동 복구하는 기능은 없습니다. `Transitioning` 동안의 일반 중복 요청은 거절하고 `PostLoadMap`에서 상태를 해제합니다. `OpenMap`의 성공 반환은 요청 제출을 뜻하며 맵 로드 완료나 플레이 가능 시점을 뜻하지 않습니다.

## 4. 검기 풀링과 반환 처리

![검기의 발사와 자원 소비](Media/aether-slash.gif)

### 추적 상한과 초과 요청 처리

[획득 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherProjectilePoolSubsystem.cpp#L34)는 사용할 수 있는 검기를 먼저 찾습니다. 없으면 생성하고, 보관 배열이 **12개 미만일 때만** 풀 소유 관계를 설정합니다. 초과 생성물은 풀 밖의 액터로 실행됩니다. 12는 추적·보관 상한이며 동시 활성 수, 월드 전체 검기 수나 누적 생성 수의 상한이 아닙니다.

```cpp
	if (AetherSlashProjectilePool.Num() < MaxTrackedAetherSlashProjectiles)
	{
		Projectile->SetOwningProjectilePool(this);
		AetherSlashProjectilePool.Add(Projectile);
	}
	else
	{
		Projectile->SetOwningProjectilePool(nullptr);
	}

	Projectile->ActivateForPool(Owner, Instigator, SpawnLocation, SpawnRotation);
```

[실제 코드: AetherProjectilePoolSubsystem.cpp 76–86줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherProjectilePoolSubsystem.cpp#L76-L86)

### 획득·초기화·반환·재획득

[발사 호출자](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCombatComponent.cpp#L1396)는 획득 이후 `InitializeSlash`과 `SetImpactAssets`를 실행합니다. `ActivateForPool`만으로 발사별 피해·속도·방향·효과 설정이 끝나지는 않습니다. 생성에 실패하면 발사 함수가 반환하며 이미 소비한 게이지를 환급하는 경로는 없습니다.

| 단계 | 현재 처리 |
| --- | --- |
| 활성화 | 소유자·인스티게이터·변환을 설정하고 가시성과 Tick을 켭니다. |
| 발사 초기화 | 방향을 정규화하고 이동 거리와 타격 상태를 초기화하며 수명을 설정합니다. |
| 비행 | [Tick](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSlashProjectile.cpp#L59)에서 남은 거리 안으로 이동 구간을 제한하고 Pawn 채널을 스윕합니다. 락온 우선 선택은 유도 비행을 뜻하지 않습니다. |
| 명중·거리 종료·수명 만료 | [종료 함수](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSlashProjectile.cpp#L295)가 완료 플래그를 먼저 설정한 뒤 반환 또는 파괴로 진행합니다. 첫 피해 적용 성공에서 종료하며 관통 투사체가 아닙니다. |
| 풀 반환 | [비활성화](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSlashProjectile.cpp#L95)가 수명·발사 참조·피해 수치·적중 배열을 초기화하고 Tick·표시·충돌을 끕니다. 배열의 `Reset`은 확보 용량을 유지할 수 있습니다. |
| 풀 밖 반환 | 풀의 추적 대상이 아니거나 소유 풀을 얻지 못하면 `Destroy`합니다. |
| 월드 종료 | [풀 해제](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherProjectilePoolSubsystem.cpp#L14)가 추적 액터의 풀 참조를 먼저 끊고 파괴한 뒤 배열을 비웁니다. |

현재 조회 API는 추적 수와 사용 가능 수를 제공합니다. 둘의 차이는 **추적 중인 활성 객체 수**를 설명할 수 있지만 풀 밖 객체, 누적 생성·반환 횟수까지 포함하지 않습니다. 재사용 경로는 확인할 수 있어도 할당량·프레임 비용 감소는 별도 계측이 필요합니다. 시각 표현에는 기본 도형 메시와 빛을 사용하므로 최종 아트 비용으로 일반화할 수도 없습니다.

## 5. 적 행동과 공격 슬롯

[적의 Tick](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherEnemyBase.cpp#L391)은 대상 갱신 후 예고 중에는 회전만 갱신하고, 회복·피격·패리 경직·처형 억제·페이즈 전환 중에는 추적과 공격 시작을 건너뜁니다. 추적은 플레이어 방향과 주변 적의 분리 방향을 더한 `AddMovementInput`입니다. AIController의 경로 탐색이나 Behavior Tree 실행은 이 경로에 없습니다.

[일반 패턴](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherEnemyAttackPatternPolicy.cpp#L18)은 거리와 양수 가중치 조건을 통과한 후보 중 추첨합니다. [Aurel](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherAurelBossPhasePolicy.cpp#L129)은 페이즈별 다음 인덱스에서 가능한 패턴을 순환 검색합니다. 페이즈 판단 구조체가 인덱스·일회성 안내 상태를 갖고, 적 액터가 이동 수치·타이머·보상을 적용합니다.

공격 시작은 [GameMode](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherGameModeBase.cpp#L698)를 거쳐 [단일 공격 슬롯](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeEnemyAttackSlotCoordinator.cpp#L14)을 요청합니다. 조정이 켜져 있으면 살아 있는 현재 소유자와 공격 지연 시각을 확인합니다.

```cpp
	if (CurrentTimeSeconds < AttackDelayUntil && CurrentAttackingEnemy.Get() != RequestingEnemy)
	{
		return false;
	}

	if (!CurrentAttackingEnemy.IsValid() || CurrentAttackingEnemy.Get() == RequestingEnemy)
	{
		CurrentAttackingEnemy = RequestingEnemy;
		return true;
	}

	return false;
```

[실제 코드: AetherPrototypeEnemyAttackSlotCoordinator.cpp 31–42줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherPrototypeEnemyAttackSlotCoordinator.cpp#L31-L42)

획득 후 적은 예고 → 거리 재판정과 플레이어 피격 처리 → 회복으로 진행하고 회복 종료·공격 중단 등에서 슬롯을 반환합니다. 다른 적의 반환 요청은 현재 소유자를 지우지 않습니다. 요청 대기열이나 순번 보장은 없고, 조정이 꺼져 있으면 요청을 통과시킵니다. 주변 적 분리는 각 적이 다른 적을 순회하므로 개체 증가 시 탐색 비용이 늘어날 가능성이 있으나 현재 병목이라는 측정 결과는 없습니다.

## 6. 일시 정지 중 그래픽 설정 확인 시간 관리

![일시 정지 중 화면 설정 확인 기한 만료](Media/paused-settings-timeout.gif)

[설정 적용](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSettingsSubsystem.cpp#L195)은 해상도 또는 창 모드가 바뀌면 적용 전 설정 묶음을 보관하고 확인 대기를 시작합니다. **15초는 기능 설정값**입니다. 남은 시간은 `FPlatformTime::Seconds()`로 계산하고 [CoreTicker 콜백](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSettingsSubsystem.cpp#L645)이 만료를 확인합니다.

```cpp
bool UAetherSettingsSubsystem::TickVideoConfirmation(float DeltaTime)
{
	if (!bAwaitingVideoConfirmation)
	{
		return false;
	}
	if (FPlatformTime::Seconds() < VideoConfirmationDeadlineSeconds)
	{
		return true;
	}

	VideoConfirmationTickerHandle.Reset();
	RevertVideoSettings();
	return false;
```

[실제 코드: AetherSettingsSubsystem.cpp 645–658줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSettingsSubsystem.cpp#L645-L658)

월드 일시 정지와 별개로 진행하는 코어 틱을 사용하지만 앱의 틱 자체가 막혀도 정확히 기한 순간에 실행된다는 뜻은 아닙니다. [확인](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSettingsSubsystem.cpp#L277)과 [취소·만료 복구](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherSettingsSubsystem.cpp#L295)는 틱커를 해제합니다. 복구 대상은 화면 모드만이 아니라 보관했던 사용자 설정 묶음 전체입니다. 서브시스템 종료에서도 틱커를 정리합니다.

`UGameUserSettings`의 그래픽 값과 별도 Settings SaveGame의 음량·입력·접근성 값은 다른 저장 경로를 사용합니다. 설정 적용 성공 반환만으로 두 저장 경로의 디스크 쓰기 성공을 보증하지 않습니다. 체크포인트 형식 버전과 설정 저장 형식 버전도 서로 다릅니다.

## 7. 연출 종료 이벤트와 상태 정리

[연출 요청](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCinematicDirectorSubsystem.cpp#L79)은 활성 연출이 있거나 정의가 비활성화되어 있으면 거절합니다. 수락하면 상태·입력 잠금·HUD 표시를 적용하고 시작 이벤트와 `OnCinematicPresentationRequested`를 방송합니다. 시퀀스 필드는 소프트 에셋 참조지만 현재 C++에는 실제 Level Sequence 플레이어 생성·재생 연결이 없습니다. 콘텐츠의 별도 연출 수신자가 필요합니다.

[종료](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCinematicDirectorSubsystem.cpp#L112)는 대체 월드 타이머를 해제하고 기록했던 컨트롤러의 잠금을 풀고 활성 상태를 비운 다음 완료 이벤트를 방송합니다.

```cpp
	const FAetherCinematicRuntimeState FinishedState = ActiveState;
	RestoreCinematicLocks(ActiveState.Definition);

	ActiveState = FAetherCinematicRuntimeState();
	ActiveCinematicStartTime = 0.0;

	OnCinematicFinished.Broadcast(FinishedState);
	OnCinematicFinishedNative.Broadcast(FinishedState);
```

[실제 코드: AetherCinematicDirectorSubsystem.cpp 203–210줄](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherCinematicDirectorSubsystem.cpp#L203-L210)

종료 콜백은 이전 활성 상태를 정리한 뒤 호출됩니다. 다만 모든 외부 콜백 재진입을 차단하는 세대 토큰은 없고 기존의 임의 입력 잠금 상태를 통째로 저장·복구하지도 않습니다. 새 요청이나 다른 잠금 소유자와 함께 사용할 때 이 경계를 고려해야 합니다.

연출 대체 종료는 **월드 타이머**, 연출 경과 표시와 로딩 화면 상태 시간은 **플랫폼 시간**을 사용합니다. 설정 확인 타이머와 같은 의미가 아닙니다. [로딩 준비 신호](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherLoadingScreenSubsystem.cpp#L92)도 호출자가 알린 상태일 뿐 셰이더·모든 에셋·플레이어 복원의 완료를 일괄 판정하지 않습니다.

## 8. 대화와 게임 진행의 연결

[대화 시작](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherDialogueComponent.cpp#L133)은 데이터의 트리거 라벨을 찾습니다. 다른 대화가 진행 중이면 중복을 제외한 트리거를 FIFO 배열에 보관합니다. 저장 요청과 달리 여기는 실제 대기열이 있습니다. `bSaveWhenPlayed` 대상은 대화를 시작할 때 재생 라벨에 추가하므로 마지막 문장을 본 사실과 같지 않습니다. 이 라벨이 디스크에 남으려면 이후 체크포인트 저장이 필요합니다.

대화 종료는 다음 대기를 꺼내 다시 시작 조건을 확인합니다. [기본 Mock TTS](https://github.com/KangWooKim/Aetherfall/blob/c603eedb7ab0c396de6c8123bf010a7e85e633b5/Source/Aetherfall/Private/AetherDialogueTtsService.cpp#L1)는 길이 추정과 완료 시각을 위한 카운트다운이며 음성을 합성하지 않습니다. 실제 음성 서비스는 별도 구현·연결 범위입니다.

## 9. 실행 성능의 현재 범위

### 측정 질문과 대상

**다른 게임이 실행되지 않은 상태에서 첫 전투 구간을 3회 측정하고, 앞서 다른 게임과 함께 실행한 3회도 별도로 보존했습니다.** 질문은 현재 전투·라운드 재시작을 반복할 때 프레임 시간, 스레드·GPU 시간과 프로세스 메모리가 어떻게 나타나는지입니다. 사전에 합격 FPS나 개선율을 정하지 않았으며 최적화 전후 비교로 해석하지 않습니다.

측정 ID는 AF-20260909-ForestIntro입니다. 2026년 9월 9일 07:30–07:35 KST의 동시 실행 자료를 A01–A03, 07:52–07:56 KST의 재측정 자료를 B01–B03으로 구분합니다. 모두 새 프로세스이며 같은 실행 파일·설정·입력 순서를 사용합니다. 공개 코드 기준은 이 문서 상단의 고정 커밋입니다. 측정에는 원본의 .uproject·Config·Content를 별도 사본으로 복사하고 **로딩 화면의 UE 5.4 호환 부분만 수정한 실행 파일**을 사용했습니다. 원본과 공개 저장소의 코드는 그대로 유지합니다.

| 측정 사본에서 달라진 부분 | 적용 내용 |
| --- | --- |
| AetherLoadingScreenSubsystem.cpp | SOverlay의 포함 경로를 Widgets/SOverlay.h로, SVerticalBox의 포함 헤더를 Widgets/SBoxPanel.h로 지정합니다. |
| AetherLoadingScreenSubsystem.h | Containers/Ticker.h를 포함하고 TickerHandle의 형식을 FTSTicker::FDelegateHandle로 맞춥니다. |

전투·적·저장·풀링 구현이나 맵·에셋·Config는 바꾸지 않았습니다. 따라서 아래 값은 공개 저장소의 수정 없는 빌드 결과가 아니라 **이 차이를 명시한 원본 실행 사본의 결과**입니다. 공개 저장소에는 실행에 필요한 전체 콘텐츠가 없으므로 소스만으로 같은 측정을 재현할 수 없습니다.

### 환경과 실행 조건

| 항목 | 실제 조건 |
| --- | --- |
| CPU / 메모리 | Core i9-9900K, 8코어 16스레드 / 16GiB, 메모리 모듈 보고 속도 2667MHz입니다. |
| GPU / 드라이버 | GeForce RTX 2060 6GiB / NVIDIA 616.56입니다. 관측한 전력 제한은 125W이며 기본 160W와 다릅니다. |
| OS / 엔진 | Windows 10 Pro 19045 / UE 5.4.3, CL 34507850입니다. |
| 구성 | Editor Development 실행 파일을 -game으로 실행한 독립 게임 창입니다. Shipping·패키지 빌드의 결과가 아닙니다. |
| 화면 | 1280×720 창 모드, 렌더 배율 100%, VSync 0, 최대 60 FPS입니다. |
| 렌더링 | DX12 / SM6, 품질 그룹 2, 동적 해상도 0, AA 방식 0입니다. r.RayTracing은 1, Lumen 하드웨어 RT는 0입니다. |
| 장면 | 기존 M_VerticalSlice의 ForestIntro 조우입니다. 외부 캐릭터·수목, 임시 구조물, HUD와 전투·CSV 디버그 표시를 포함합니다. |
| 진행 / 카메라 | 기본 3인칭 카메라에서 전진·락온·근접 공격·패리·회피·라운드 재시작을 실행합니다. 피격과 락온에 따라 카메라와 화면 구성이 변합니다. |
| 시간 / 난수 | 별도 고정 시뮬레이션 주기·배속·난수 시드를 지정하지 않습니다. 게임의 히트스톱 등 기존 시간 처리를 포함합니다. |
| 배경 부하 A | 다른 게임과 대화·개발 앱이 실행 중이었고 그 구성이 실행 사이에 달라졌습니다. 실행 전후 GPU 온도는 51–64°C였습니다. |
| 배경 부하 B | 다른 게임의 종료 상태를 확인한 뒤 재측정했습니다. 대화·개발 앱 등 일반 데스크톱 작업은 남아 있으며 실행 전후 GPU 온도는 44–62°C였습니다. |
| 전원 / 통제 범위 | 두 조건 모두 고성능 전원 계획입니다. 시스템을 초기화하거나 일반 앱·온도·파일 캐시를 고정하지 않았습니다. |

품질 그룹 조회값은 2이지만 게임 설정의 우선순위가 더 높은 세부 항목도 있습니다. 실행 로그는 r.MotionBlurQuality 0, r.SkeletalMeshLODBias 0, r.ViewDistanceScale 1, r.Shadow.DistanceScale 1, foliage.DensityScale 1, grass.densityScale 1을 유지했다고 기록합니다. 모든 렌더링 항목이 엔진 기본 품질 프리셋과 같다고 해석하지 않습니다.

실행 전후 다른 프로젝트의 빌드·성능 실행이 없는지 확인했습니다. B에서는 외부 읽기 전용 프로세스 목록을 약 5초마다 기록했고 알려진 다른 게임·프로젝트·빌드 프로세스가 관측되지 않았습니다. 조회 1회의 최대 경과 시간은 7.33ms이며 이 감시 작업과 일반 앱의 비용은 남아 있습니다. 목록 조회 사이의 모든 활동이 통제된 전용 벤치마크 환경은 아닙니다. 실행별 독립 UserDir를 사용해 기존 사용자 저장과 설정을 읽거나 덮어쓰지 않았습니다.

### 입력과 측정 구간

엔진 CSV Profiler의 csvCaptureFrames, csvExecCmds와 Enhanced Input의 Input.+key / Input.-key를 사용합니다. 게임에 계측 함수나 새 하니스를 추가하지 않습니다. 아래는 실제 실행 인자의 구성입니다. 측정 사본과 엔진·출력 경로는 변수로 표시합니다.

```text
<UE_ROOT>/Engine/Binaries/Win64/UnrealEditor.exe <MEASUREMENT_PROJECT>/Aetherfall.uproject
/Game/Aetherfall/Maps/M_VerticalSlice -game -d3d12 -windowed -ResX=1280 -ResY=720
-WinX=40 -WinY=60 -unattended -nosplash -NoVSync -UserDir=<RUN_USER_DIR>
-abslog=<RUN_LOG> -stdout -FullStdOutLogOutput -csvCaptureFrames=3780 -csvGpuStats
-csvMetadata=experiment=<RUN_ID>,scenario=ForestIntroMixedCombat -ExitAfterCsvProfiling
-ExecCmds="t.MaxFPS 60,r.VSync 0,r.SetRes 1280x720w,r.ScreenPercentage 100,
sg.ViewDistanceQuality 2,sg.AntiAliasingQuality 2,sg.ShadowQuality 2,
sg.GlobalIlluminationQuality 2,sg.ReflectionQuality 2,sg.PostProcessQuality 2,
sg.TextureQuality 2,sg.EffectsQuality 2,sg.FoliageQuality 2,sg.ShadingQuality 2,
t.IdleWhenNotForeground 0" -csvExecCmds="<아래 프레임별 명령>"
```

가독성을 위해 줄을 나누었으며 각 인자는 실제 실행에서 하나의 인자 문자열로 전달합니다. 캡처 프레임 300에서 W를 누르고 660에서 놓아 기존 조우에 진입합니다. 이후 1200·1800·2400·3000을 기준 프레임으로 아래 입력을 반복합니다. 누름은 Input.+key, 놓음은 Input.-key 명령이며 W 외에는 2프레임 뒤에 놓습니다.

| 기준 프레임 이후 | 입력 |
| --- | --- |
| 0 / 10 | R로 현재 라운드를 재시작하고 Tab으로 락온을 요청합니다. |
| 40–80 | W로 전진합니다. |
| 100 / 160 / 240 | E 강공격, LeftMouseButton 약공격, E 강공격을 요청합니다. |
| 330 / 420 | X 검기, Q 패리를 요청합니다. |
| 460 / 520 / 540 | 약공격, SpaceBar 회피, 약공격을 요청합니다. |

1800과 3600에서 r.VSync 조회 명령을 기록하고 **첫 경계가 있는 CSV 행부터 두 번째 경계 직전까지 1,800행**을 집계합니다. 파일의 첫 헤더를 제외한 데이터 행 번호를 0부터 셀 때 범위는 1799 이상 3599 미만입니다. 엔진 초기 더미 프레임과 준비 구간을 제외하며 준비 중 같은 전투 입력을 한 차례 실행합니다. 화면 캡처는 1680·3700, 설정·액터 조회는 1720·3660에 수행하여 집계 구간 밖에 둡니다.

충돌·미완료 CSV·구간 표식 누락·의도한 조우 미진입·설정 불일치·핵심 시간 값의 무효 표본·다른 프로젝트의 동시 빌드나 성능 실행을 제외 조건으로 정했습니다. B에는 알려진 다른 게임의 관측도 제외 조건으로 추가했습니다. 사전 탐색 3회는 결과에서 제외했습니다. A·B의 큰 지연이나 사망 구간은 제거하지 않았습니다. 같은 프레임에 입력하더라도 실제 경과 시간, 적의 난수 선택과 입력 허용 상태가 다르므로 실제 전투 부하까지 같지는 않습니다.

### 프레임과 처리 시간

**A는 다른 게임과 동시 실행, B는 해당 게임들이 종료된 뒤의 재측정**입니다. 모든 실행은 1,800개 표본입니다. 시간 단위는 ms이며, p95는 정렬한 표본의 ceil(0.95×N)번째 값입니다. 평균은 산술평균, 중앙값은 가운데 두 값의 평균입니다. FPS는 순간 FPS의 평균이 아니라 **1000 / 평균 프레임 시간**입니다.

| 실행 | 준비 / 집계 시간(s) | 평균(ms) | 중앙값(ms) | p95(ms) | 최대(ms) | 계산 FPS |
| --- | --- | --- | --- | --- | --- | --- |
| A01 | 30.52 / 30.18 | 16.76 | 16.68 | 18.63 | 28.36 | 59.65 |
| A02 | 37.59 / 33.05 | 18.36 | 16.93 | 22.10 | 637.11 | 54.46 |
| A03 | 35.92 / 36.10 | 20.06 | 20.24 | 24.41 | 28.20 | 49.86 |
| B01 | 30.60 / 30.00 | 16.67 | 16.67 | 16.95 | 26.42 | 60.00 |
| B02 | 30.38 / 30.00 | 16.67 | 16.67 | 16.91 | 28.94 | 60.00 |
| B03 | 30.43 / 30.00 | 16.67 | 16.67 | 16.96 | 26.45 | 60.00 |

| 실행 | 게임 스레드 평균 / p95(ms) | 렌더 스레드 평균 / p95(ms) | GPU 평균 / p95(ms) |
| --- | --- | --- | --- |
| A01 | 3.55 / 5.26 | 9.32 / 17.97 | 13.50 / 16.73 |
| A02 | 3.57 / 4.47 | 13.33 / 21.36 | 14.79 / 18.39 |
| A03 | 3.41 / 4.07 | 17.82 / 23.79 | 17.05 / 20.38 |
| B01 | 3.10 / 3.45 | 5.21 / 5.63 | 12.07 / 12.70 |
| B02 | 3.03 / 3.33 | 5.25 / 5.63 | 12.22 / 12.74 |
| B03 | 3.15 / 3.46 | 5.26 / 5.62 | 11.92 / 12.55 |

FrameTime은 엔진 프레임 종료 시각 사이의 간격이며 프레임 제한 대기와 계측 비용을 포함합니다. 나머지는 GameThreadTime, RenderThreadTime, GPUTime 필드의 값입니다. 이 값은 특정 전투 함수만의 시간이 아니며 서로 겹치는 CPU·GPU 시간을 합산하지 않습니다. 60 FPS 제한이 있으므로 여유 성능의 최대 처리량을 측정한 자료도 아닙니다.

### 메모리와 부하 차이

CSV 이름의 MB는 이 엔진 경로에서 바이트를 1024²로 나눈 값이므로 **MiB**로 표기합니다. PhysicalUsedMB는 프로세스 작업 집합, VirtualUsedMB는 Windows PagefileUsage를 바탕으로 한 커밋 사용량입니다. 실제 페이지 파일 읽기·쓰기 양이나 게임 객체만의 할당량을 뜻하지 않습니다. 피크는 집계 구간의 프레임별 관측 최대값입니다.

| 실행 | 프로세스 작업 집합 평균 / 피크(MiB) | 커밋 평균 / 피크(MiB) | 시스템 가용 메모리 최소(MiB) |
| --- | --- | --- | --- |
| A01 | 2752.48 / 2808.07 | 5807.49 / 5907.86 | 1253.56 |
| A02 | 2331.79 / 2906.13 | 5971.16 / 6070.39 | 58.66 |
| A03 | 2840.40 / 2865.23 | 5846.60 / 5909.22 | 191.43 |
| B01 | 2721.79 / 2758.21 | 5652.14 / 5708.05 | 3906.49 |
| B02 | 2731.09 / 2764.49 | 5734.81 / 5796.37 | 3917.86 |
| B03 | 2681.35 / 2708.61 | 5650.32 / 5681.62 | 4033.41 |

| 실행 | 전체 Actor 객체 수 범위 | 실행된 약공격 / 강공격 | 플레이어 사망 |
| --- | --- | --- | --- |
| A01 | 234–244 | 7 / 5 | 0 |
| A02 | 229–240 | 8 / 5 | 1 |
| A03 | 229–242 | 3 / 4 | 2 |
| B01 | 232–246 | 7 / 4 | 0 |
| B02 | 232–238 | 9 / 5 | 0 |
| B03 | 232–238 | 9 / 4 | 0 |

전체 Actor 값은 엔진의 ActorCount/TotalActorCount이며 기본 객체·아키타입을 제외한 객체의 생성과 소멸을 집계합니다. 죽은 적의 잔존 객체도 포함할 수 있어 살아 있는 적 수·Tick 중인 적 수·화면에 보이는 적 수와 다릅니다. ForestIntro 설정의 적 생성 수는 1, 라운드 처치 목표는 2였습니다. 생존·표시 상태는 구간 전후 조회와 화면으로 확인했으며 모든 프레임의 생존·가시 개체 수를 분리 계측하지 않았습니다. 클래스별 CSV 카운터에는 기본 기록 임계값 5가 있어 누락 값을 0마리로 해석하지 않습니다.

각 실행은 라운드 재시작을 세 번 포함하지만 공격 수락과 사망 여부가 달랐습니다. 따라서 표의 차이를 동일 전투 부하에서 발생한 순수한 처리 성능 편차로 해석하지 않습니다. 검기는 이 구간에서 자원·행동 조건을 통과한 발사가 없었으므로 **검기 반복 발사·풀 재사용의 성능 실험은 성립하지 않습니다**.

### 로딩 시간의 경계

동일 실행의 엔진 LogLoad에서 기록한 LoadMap 소요 시간입니다. 전투 구간 통계와 별도로 취급합니다.

| 실행 | LoadMap 기록(s) |
| --- | --- |
| A01 | 1.908913 |
| A02 | 3.816310 |
| A03 | 2.177078 |
| B01 | 3.308323 |
| B02 | 1.675700 |
| B03 | 1.622153 |

UEngine::LoadMap의 StartTime에서 LoadMapComplete 표식 이후 StopTime까지의 경과입니다. 프로세스 전체 기동 시간, 로딩 화면 해제, 조작 가능 시점이나 저장 슬롯 복원만의 비용이 아닙니다. 기존 저장이 없는 실행이며 OS 파일 캐시·DDC를 비우지 않았으므로 콜드 로딩 기준도 아닙니다.

### 근거와 해석의 한계

CSV Profiler의 마지막 헤더를 사용해 실행 중 추가된 열까지 해석하고, 이름이 중복되지 않는 위의 핵심 필드를 추출했습니다. 다음은 각 실행에서 FrameTime이 가장 큰 행의 실제 필드 발췌입니다.

```csv
Run,CSVDataRow0,FrameTime,GameThreadTime,RenderThreadTime,GPUTime,MemoryFreeMB
A01,2391,28.3618,14.2366,5.5234,12.1084,3176.7422
A02,2509,637.1103,3.0821,637.5896,16.5309,89.6289
A03,3075,28.2011,3.1147,27.6812,19.8750,294.8086
B01,1801,26.4202,12.4224,5.0618,11.5332,3982.8633
B02,3001,28.9375,14.8824,4.9298,11.9794,3949.3203
B03,3001,26.4518,12.5789,5.1174,11.7550,4067.3555
```

A02에는 637.11ms 프레임 지연이 있었고 같은 집계 구간에서 시스템 가용 메모리가 58.66MiB까지 낮아졌습니다. 메모리 압박을 동반한 관측이지만 이 지연의 원인을 특정 함수나 페이징으로 확정할 추적 자료는 없습니다. A03도 약 49.86 FPS였습니다. B의 계산값은 차례로 60.00 / 60.00 / 60.00 FPS였지만 실제 전투 결과·온도·캐시·일반 앱 부하가 같지 않아 두 조건의 차이를 특정 게임 종료의 인과 효과나 최적화 개선율로 계산하지 않습니다. 짧은 표본으로 지속적인 60 FPS를 보장하지도 않습니다.

오디오의 RecursiveApplyAdjuster 실패 로그가 전투 중 지속됐으며 그 처리·출력 비용을 포함합니다. 사운드 클래스 조정 대상 또는 관련 속성을 찾지 못한 엔진 분기에서 나오는 메시지이지만 프로젝트 측 근본 원인은 확정하지 않았습니다. 오디오 정상 동작, 오디오 비용 제외 결과 또는 해당 오류 수정 효과를 주장하지 않습니다.

시작 로그에는 r.ShaderCompiler.JobCache의 렌더 스레드 접근 경고도 남았습니다. 해당 경고와 화면 이상 또는 프레임 지연의 인과관계는 확인하지 않았습니다.

측정 모듈 UnrealEditor-Aetherfall.dll의 SHA-256은 4ae6d76d578336635ee9c3470c74bc3af24abcac9ae01aaa9e561ed267a08fcc입니다. 실행별 원본 CSV 식별값은 다음과 같습니다. 해시는 자료 식별을 위한 값이며 실행 사실 자체를 대신하지 않습니다.

| 실행 | 원본 CSV SHA-256 |
| --- | --- |
| A01 | 93da71660aee9a8455285d5c76f2a34cce3fc7c10dc4f425a9a09b42729113b4 |
| A02 | 9b6f3424b685e3191247e5fa35a44068cae8f0f782765e955687df5504adcf53 |
| A03 | 7a60680b752e49180818fafa546f9490b5ec27b1ecd0b11f8b37687cfd190127 |
| B01 | f9fd45ef1401c7328b5dd721c6ba90428b88a6fb121ea3da92af71111f529020 |
| B02 | 6caaa34d75f1ff2ee2265dd2cdf64f851890a1fda776c140971c7259df7d7b77 |
| B03 | 0751b2512a4b9519e6cc8c1fe795b35092bea21a7e12d2fe4573fa55b4fbf2a9 |

원본 CSV·로그·실행 인자·설정 조회·화면·집계 자료는 저장소 밖에 보존하며 공개 저장소에는 요약과 위 발췌만 제공합니다. 공개하지 않은 원본을 내려받을 수 있는 자료처럼 링크하지 않습니다. 다른 기기, 최종 아트, 더 많은 적, 패키지·Shipping 구성으로 이 값을 일반화하지 않습니다.

검기의 누적 생성·반환·할당 비용, 체크포인트 저장 호출·복원 지연, 디스크 쓰기 완료와 플레이 가능 시점은 별도 계측하지 않았습니다. 풀링을 끄는 기존 비교 기능도 없어 성능 개선율을 계산하지 않습니다. 기능 타이머·기존 GIF·과거 출처 불명 CSV로 이 미측정 항목을 대신하지 않습니다.
