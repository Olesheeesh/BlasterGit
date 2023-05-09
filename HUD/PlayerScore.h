// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScore.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UPlayerScore : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	class UImage* isDead;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Kills;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Deaths;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Ping;
};
