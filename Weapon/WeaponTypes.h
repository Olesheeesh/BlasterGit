#pragma once

UENUM(BlueprintType)//������� �� � weapon class'�, ���� ��� ������������� � ������ �������, �� �������� ���� ����� weapon
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),

	EWT_MAX UMETA(DisplayName = "DeffaultMAX")

};
