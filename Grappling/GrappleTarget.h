// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrappleTarget.generated.h"

UCLASS()
class BLASTER_API AGrappleTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	friend class UGrappleComponent;
	AGrappleTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SetActive(bool bActive);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnyWhere, Category = "Target Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Properties", meta = (AllowPrivateAccess = "true"))
	class UBillboardComponent* Billboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* WidgetComponent;

private:
	

	

};
