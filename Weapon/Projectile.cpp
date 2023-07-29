// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));//construct the collisionBox
	SetRootComponent(CollisionBox);//set to root component
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//collision settings
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,//particle system
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
	if(HasAuthority())//чтоб OnHit event просчитывался только на сервере
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);//& - компьютер не создаёт локальную копию переменной, а сразу к ней обращается
	}
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime,
		false
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactHitMaterial, FVector(10.0f), Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), 5.0f);
	
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
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
}

void AProjectile::ApplyExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringControler = FiringPawn->GetController();
		if (FiringControler)
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
				FiringControler //instigator controller
			);
		}
	}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}


