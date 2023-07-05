#pragma once
#define TRACE_LENGTH 80000.f //macro

UENUM(BlueprintType)//создали не в weapon class'е, чтоб при использовании в других классах, не включать весь класс weapon
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_ShotGun UMETA(DisplayName = "Shot Gun"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	EWT_GrapplingHook UMETA(DisplayName = "Grappling Hook"),

	EWT_MAX UMETA(DisplayName = "DeffaultMAX")

};
