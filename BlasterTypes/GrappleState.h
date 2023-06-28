#pragma once

UENUM(BlueprintType)
enum class EGrappleState : uint8
{
	EGS_Retracted UMETA(DisplayName = "Retracted"),
	EGS_Firing UMETA(DisplayName = "Firing"),
	EGS_NearingTarget UMETA(DisplayName = "NearingTarget"),
	EGS_OnTarget UMETA(DisplayName = "OnTarget")
};