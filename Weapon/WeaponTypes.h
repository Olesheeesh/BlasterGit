#pragma once

UENUM(BlueprintType)//создали не в weapon class'е, чтоб при использовании в других классах, не включать весь класс weapon
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),

	EWT_MAX UMETA(DisplayName = "DeffaultMAX")

};
