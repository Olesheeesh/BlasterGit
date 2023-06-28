// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
#define ECC_PermanentStaticMesh ECollisionChannel::ECC_GameTraceChannel2
#define ECC_GrappleTarget ECollisionChannel::ECC_GameTraceChannel3

UENUM(BlueprintType)
enum class EGASAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	Attack,
	GhostDash

};
