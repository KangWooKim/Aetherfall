# Aetherfall

Unreal Engine 5.4 / C++로 개발 중인 Windows용 3인칭 액션 RPG 개인 프로젝트입니다. 전투 행동의 판단과 실행, 체크포인트 저장·월드 복원, 검기 투사체의 생성·재사용·반환 처리가 핵심 구현입니다.

개발자와 Codex가 소스코드를 공동 작성한 개인 프로젝트입니다.

## 프로그래머용 문서

| 읽을 순서 | 문서 | 확인할 내용 |
| --- | --- | --- |
| 1 | [프로젝트 개요와 아키텍처](Docs/03_Technical/ProjectOverview.md) | 게임 흐름, 계층별 역할, 생명주기, 기술 환경 |
| 2 | [핵심 기술과 코드 발췌](Docs/03_Technical/CoreTechnologies.md) | 전투·저장·풀링·적 행동·설정의 처리 순서와 설계 |
| 3 | [전체 소스 찾아보기](Docs/03_Technical/SourceIndex.md) | Source 전체 146개 파일의 역할과 고정된 코드 링크 |

문서의 소스 링크는 분석한 [코드 커밋](https://github.com/KangWooKim/Aetherfall/commit/2dfcf0cd57cdd7972c35c6d25d4c38c22fcd6ae9)에 고정되어 있습니다. 링크를 누르면 해당 파일의 설명한 줄이 강조됩니다. 이후 main이 바뀌어도 이 문서가 분석한 코드를 확인할 수 있습니다.

## 주요 구성

- Character와 Enhanced Input Controller, 전투·체력·인벤토리·락온·상호작용 컴포넌트.
- 전투 가능 여부·상태·스태미나·게이지·타이머·대상 선택을 계산하는 로직과 실행 컴포넌트.
- GameMode의 진행 라벨과 체크포인트, SaveGame 스키마 2, 종류별 월드 복원.
- WorldSubsystem 기반 검기 풀과 반환 시 상태 초기화.
- C++ 적 행동·공격 패턴·Aurel 페이즈, 메뉴·로딩·컷신·일시 정지·설정 서비스.

## 기술 환경과 자료 구성

이 저장소에는 C++ 게임 모듈과 Game/Editor Target을 포함한 Source 전체 146개 파일, 프로그래머용 기술 문서를 정리했습니다. 게임 Content, 맵·Blueprint, 프로젝트 설명 파일과 Config는 별도로 관리합니다.

개발 환경은 UE 5.4, Visual Studio 2022 MSVC 14.34, Windows SDK 10.0.22000입니다. 게임 실행 프로젝트는 소스와 함께 해당 엔진, 프로젝트 설정, 맵·Blueprint·애니메이션 등의 에셋으로 구성됩니다.

모델·애니메이션·VFX·음향·환경에는 Marketplace/Fab 자산을 활용했습니다. 기본 대화 TTS는 경과 시간으로 완료를 알리는 모의 구현이며, 적 행동은 C++의 이동·회전과 공격 상태 처리로 구성했습니다.
