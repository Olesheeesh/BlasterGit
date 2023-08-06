// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreBoardWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* PlayersS;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayersNumText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapNameText;

	bool IsActive = false;
};
