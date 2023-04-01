#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void OnPossess(APawn* InPawn) override;
	void SetHUDWeaponAmmo(int32 AmmoAmount);

	UFUNCTION(Client, Reliable)
	void ShowAmmoHUD(bool ShowHUD);

	UFUNCTION()
	void OnRep_ShowAmmoHUD(bool ShowHUD);

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_ShowAmmoHUD)
	bool HUDIsShown;

	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

};
