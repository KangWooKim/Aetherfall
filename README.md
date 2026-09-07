# Aetherfall

Unreal Engine 5.4 / C++로 개발 중인 Windows용 3인칭 액션 RPG 개인 프로젝트입니다. 전투 행동의 판단과 실행, 체크포인트 저장·월드 복원, 검기 투사체의 수명 관리가 핵심 코드 검토 대상입니다.

개발자와 Codex가 소스코드를 공동 작성했으며, 검증 등의 작업은 Codex에 맡겼습니다. 확인되지 않은 개인 기여율·개발 기간·성능 개선 수치는 제시하지 않습니다.

## 프로그래머용 문서

| 읽을 순서 | 문서 | 확인할 내용 |
| --- | --- | --- |
| 1 | [프로젝트 개요와 아키텍처](Docs/03_Technical/ProjectOverview.md) | 범위, 계층별 책임, 수명, 게임 흐름, 실행 조건 |
| 2 | [핵심 기술과 코드 발췌](Docs/03_Technical/CoreTechnologies.md) | 전투·저장·풀링·적 행동·설정의 처리 순서와 한계 |
| 3 | [전체 소스 찾아보기](Docs/03_Technical/SourceIndex.md) | Source 전체 146개 파일의 책임과 고정된 코드 링크 |

문서의 소스 링크는 분석한 [코드 커밋](https://github.com/KangWooKim/Aetherfall/commit/2dfcf0cd57cdd7972c35c6d25d4c38c22fcd6ae9)에 고정되어 있습니다. 링크를 누르면 해당 파일의 설명한 줄이 강조됩니다. 이후 main이 바뀌어도 이 문서가 분석한 코드를 확인할 수 있습니다.

## 주요 구성

- Character와 Enhanced Input Controller, 전투·체력·인벤토리·락온·상호작용 컴포넌트.
- 전투 허용 조건·상태·자원·타이머·대상 선택 정책과 실행 컴포넌트.
- GameMode의 진행 라벨과 체크포인트, SaveGame 스키마 2, 종류별 월드 복원.
- WorldSubsystem 기반 검기 풀과 반환 시 상태 초기화.
- C++ 적 행동·공격 패턴·Aurel 페이즈, 메뉴·로딩·컷신·일시 정지·설정 서비스.

## 현재 검증 상태

2026-09-07 로컬 검증 기준입니다. 주석 정비 대상 모듈 144개 파일은 주석 외 코드·전처리·인코딩 보존 검사를 통과했습니다. 이번 동기화에는 기존 Target 파일 2개도 포함했습니다. 원격에 있던 소스 135개와 비교한 비주석 코드는 동일하며, 원격에 빠져 있던 모듈·Target·검증 소스 11개를 추가했습니다.

격리 사본의 UHT는 통과했지만 C++ 빌드는 기존 [SOverlay include 경로](https://github.com/KangWooKim/Aetherfall/blob/2dfcf0cd57cdd7972c35c6d25d4c38c22fcd6ae9/Source/Aetherfall/Private/AetherLoadingScreenSubsystem.cpp#L17) 오류로 실패했습니다. 주석 변경 전 구현 파일로도 같은 오류를 재현했습니다. 이번 업로드에서 이 오류를 수정하지 않았습니다. 런타임 명령, 전체 경로 플레이, 실제 플레이타임, 성능·Shipping 패키지는 검증 완료 상태가 아닙니다.

## 저장소 범위와 실행 조건

이 저장소는 소스 검토용입니다. Source 전체를 포함하지만 게임 Content, 맵·Blueprint, 상용 에셋, 프로젝트 설명 파일과 Config를 함께 배포하는 실행 가능한 전체 프로젝트는 아닙니다. Source만 복제해 즉시 게임을 빌드·플레이할 수 있다고 안내하지 않습니다.

로컬 확인 환경은 UE 5.4, Visual Studio 2022 MSVC 14.34, Windows SDK 10.0.22000입니다. 전체 실행에는 해당 엔진과 프로젝트 설정·필요 에셋이 추가로 필요합니다.

Marketplace/Fab의 모델·애니메이션·VFX·음향·환경 자산을 활용했습니다. 해당 자산을 직접 제작한 것으로 표시하지 않으며 유료 에셋 원본은 이 저장소에 포함하지 않습니다. 기본 대화 TTS는 시간 기반 Mock이고, 현재 적 행동을 Behavior Tree나 GAS 기반이라고 소개하지 않습니다.
