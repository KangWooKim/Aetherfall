/** 전투 구간·열쇠·보상·기록·결말이 공유하는 라벨을 한곳에 정의하여 문자열 중복을 줄인다. */
#pragma once

#include "CoreMinimal.h"

namespace AetherPrototypeLabels
{
	inline FName ForestIntro()
	{
		static const FName Label(TEXT("ForestIntro"));
		return Label;
	}

	inline FName HamletEntry()
	{
		static const FName Label(TEXT("HamletEntry"));
		return Label;
	}

	inline FName LowerCryptElite()
	{
		static const FName Label(TEXT("LowerCryptElite"));
		return Label;
	}

	inline FName CathedralBoss()
	{
		static const FName Label(TEXT("CathedralBoss"));
		return Label;
	}

	inline FName CathedralEnding()
	{
		static const FName Label(TEXT("CathedralEnding"));
		return Label;
	}

	inline FName HamletKey()
	{
		static const FName Label(TEXT("HamletKey"));
		return Label;
	}

	inline FName HamletDoor()
	{
		static const FName Label(TEXT("HamletDoor"));
		return Label;
	}

	inline FName GateLowerCryptElite()
	{
		static const FName Label(TEXT("Gate_LowerCryptElite"));
		return Label;
	}

	inline FName CryptEliteReward()
	{
		static const FName Label(TEXT("CryptEliteReward"));
		return Label;
	}

	inline FName ForestShortcutChestReward()
	{
		static const FName Label(TEXT("ForestShortcutChestReward"));
		return Label;
	}

	inline FName AetherResonanceShard()
	{
		static const FName Label(TEXT("AetherResonanceShard"));
		return Label;
	}

	inline FName LoreHamlet001()
	{
		static const FName Label(TEXT("LORE_Hamlet_001"));
		return Label;
	}

	inline FName LoreCrypt001()
	{
		static const FName Label(TEXT("LORE_Crypt_001"));
		return Label;
	}
}
