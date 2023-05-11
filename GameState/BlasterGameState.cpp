// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, BlasterPlayerStates);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}

}

void ABlasterGameState::MulticastData_Implementation(const TArray<ABlasterPlayerState*>& Data)
{
	BlasterPlayerStates = Data;
	UE_LOG(LogTemp, Log, TEXT("MULTICASTBlasterPlayerStates.Num(): %d"), BlasterPlayerStates.Num());
}

void ABlasterGameState::GetAllPlayerStates_Implementation()
{
	BlasterPlayerStates.Empty();
	UE_LOG(LogTemp, Log, TEXT("GS1"));
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			// получаем PlayerState для каждого игрока
			ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerController->PlayerState);
			if (BlasterPlayerState)
			{
				// добавляем PlayerState в массив BlasterPlayerState
				BlasterPlayerStates.Add(BlasterPlayerState);
				UE_LOG(LogTemp, Log, TEXT("ServerPlayerState: %s"), *BlasterPlayerState->GetPlayerName());
				UE_LOG(LogTemp, Log, TEXT("ServerPlayerState.Num(): %d"), BlasterPlayerStates.Num());
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("GS2"));
	// Replicate the updated array to all clients
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("__GSAutority1"));
		BlasterPlayerStates.Sort(); // ensure the array is sorted to ensure consistent order
		MulticastData(BlasterPlayerStates);
		UE_LOG(LogTemp, Log, TEXT("__GSAutority2"));
	}
}

