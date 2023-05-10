// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetBlueprint.h"
#include "ScoreBoardWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UScoreBoardWidget : public UWidgetBlueprint
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayersNumText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapNameText;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* PlayersS;

};