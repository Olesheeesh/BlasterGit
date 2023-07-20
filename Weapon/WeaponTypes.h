#pragma once
#define TRACE_LENGTH 80000.f //macro
#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)//создали не в weapon class'е, чтоб при использовании в других классах, не включать весь класс weapon
enum class EWeaponType : uint8
{
	//silver weapons pack
	EWT_None UMETA(DisplayName = "None"),
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_ShotGun UMETA(DisplayName = "Shot Gun"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	EWT_GrapplingHook UMETA(DisplayName = "Grappling Hook"),

	//Sci-Fi weapons
	EWT_SF_Pistol UMETA(DisplayName = "SciFi Pistol"),
	EWT_SF_ShotGun UMETA(DisplayName = "SciFi Shotgun"),
	EWT_MAX UMETA(DisplayName = "DeffaultMAX")

};

UENUM(BlueprintType)
enum class EWeaponSocketType : uint8
{
	EWST_Custom UMETA(DisplayName = "Custom"),
	EWST_SilverWeapon UMETA(DisplayName = "Default | Silver Weapon"),
	EWST_SciFiWeapon UMETA(DisplayName = "Default | SciFiWeapon Weapon"),
};