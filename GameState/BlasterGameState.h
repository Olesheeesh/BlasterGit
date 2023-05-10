// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	UFUNCTION(Server, Reliable)
	void GetAllPlayerStates();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastData(const TArray<ABlasterPlayerState*>& Data);

	UPROPERTY(Replicated)//replicate so that all clients know who are top scoring players as soon as the game ends
	TArray<class ABlasterPlayerState*> TopScoringPlayers;

	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> BlasterPlayerStates;
	
private:
	float TopScore = 0.f;
};
