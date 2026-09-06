// Copyright Epic Games, Inc. All Rights Reserved.

/** 게임 모듈의 빌드 의존성을 선언한다. 공개 API에 필요한 입력·UMG와 내부 Slate 구현을 구분한다. */
using UnrealBuildTool;

public class Aetherfall : ModuleRules
{
	public Aetherfall(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Slate 모듈은 위에서 이미 등록되어 있다. 아래 줄은 비활성 예제다.
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// 온라인 기능이 필요할 때 사용할 의존성 예제다. 현재 빌드에는 포함하지 않는다.
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// Steam 온라인 기능을 도입하려면 프로젝트 플러그인 설정도 함께 검토해야 한다.
	}
}
