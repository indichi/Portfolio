#pragma once

#include "CoreMinimal.h"
#include "S1GameInstance.generated.h"

class AS1Player;

UCLASS()
class S1_API US1GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AS1Player> OtherPlayerClass;
};
