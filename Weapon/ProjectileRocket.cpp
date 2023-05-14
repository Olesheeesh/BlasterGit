// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;// sets up that when the projectile moves, its orientation will correspond to the direction of motion
	RocketMovementComponent->SetIsReplicated(true);// This guarantees that the movement of the projectile will be synchronized on all devices
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if(ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,//stops playing sound when root is destroyed
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,//pointer type
			false
		);
	}
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit self"));
		return;
	}
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringContoler = FiringPawn->GetController();
		if (FiringContoler)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //world context object
				Damage, //base damage
				10.f, //min damage
				GetActorLocation(), //origin(center of radius)
				DamageInnerRadius,
				DamageOuterRadius,
				1.f, //damage will decrease steadily for actors, the farther away they are from InnerRadius and that damage decrease will be linear
				UDamageType::StaticClass(), //DamageTypeClass
				TArray<AActor*>(),//can add actors to ignore
				this,
				FiringContoler //instigator controller
			);
		}
	}
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime,
		false
	);
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if(ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::Destroyed()
{
}