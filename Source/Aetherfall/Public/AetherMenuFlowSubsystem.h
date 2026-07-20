#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherMenuFlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAetherMenuFlowState : uint8
{
	Idle,
	Transitioning,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAetherMenuFlowChangedSignature, EAetherMenuFlowState, State, FText, Message);

UCLASS()
class AETHERFALL_API UAetherMenuFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Menu")
	EAetherMenuFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Menu")
	bool IsTransitionInProgress() const { return FlowState == EAetherMenuFlowState::Transitioning; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool ContinueGame();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool StartNewGame(bool bOverwriteExistingProgress);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool LoadGame();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	void QuitGame(APlayerController* PlayerController);

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Menu")
	FAetherMenuFlowChangedSignature OnMenuFlowChanged;

private:
	bool OpenMap(FName MapName, const FText& LoadingMessage);
	void SetFlowState(EAetherMenuFlowState NewState, const FText& Message);
	void HandlePostLoadMap(UWorld* LoadedWorld);

	EAetherMenuFlowState FlowState = EAetherMenuFlowState::Idle;
	FName GameplayMapName = FName(TEXT("/Game/Aetherfall/Maps/M_VerticalSlice"));
	FName MainMenuMapName = FName(TEXT("/Game/Aetherfall/Maps/M_MainMenu"));
};
