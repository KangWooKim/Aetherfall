#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherMainMenuPlayerController.generated.h"

class UAetherMainMenuWidget;

/** 로컬 플레이어의 메뉴 위젯을 생성하고 UI 전용 입력·포커스를 설정한 뒤 화면 준비를 알린다. */
UCLASS()
class AETHERFALL_API AAetherMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAetherMainMenuPlayerController();

protected:
	/** 메뉴 Blueprint 클래스를 우선 로드하고 실패하면 C++ 위젯으로 대체해 UI 입력과 키보드 포커스를 연결한다. */
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Menu")
	TSubclassOf<UAetherMainMenuWidget> MenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Menu")
	TSoftClassPtr<UAetherMainMenuWidget> MenuWidgetBlueprintClass;

	UPROPERTY(Transient)
	TObjectPtr<UAetherMainMenuWidget> MenuWidget;
};
