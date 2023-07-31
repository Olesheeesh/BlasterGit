#include "SingularityGrenade.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASingularityGrenade::ASingularityGrenade()
{
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(ProjectileMesh);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent->bShouldBounce = false;
}

void ASingularityGrenade::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{//for server
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ASingularityGrenade::OnSphereOverlap);
		StartSpawnSingularityTimer();
	}
}

void ASingularityGrenade::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->bDisableGameplay = true;
		OverlappingCharacters.Add(BlasterCharacter);
		FVector CharacterLocation = BlasterCharacter->GetActorLocation();
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("MeshLocVector = X:%f, Y:%f, Z:%f"), CharacterLocation.X, CharacterLocation.Y, CharacterLocation.Z));

		//MoveCharacterToEpicenter(BlasterCharacter, GetWorld()->GetDeltaSeconds());
		//StartTimer();
	}
}

void ASingularityGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(bUseGrenateAbility)
	{
		if (OverlappingCharacters.Num() > 0)
		{
			for (auto& Char : OverlappingCharacters)
			{
				//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("OverlappingCharacters.Num() = %d"), OverlappingCharacters.Num()));
				MoveCharacterToEpicenter(Char, DeltaTime);
			}
		}
	}
}

/*
* SpawnSingularity
*/

void ASingularityGrenade::StartSpawnSingularityTimer()
{
	GetWorldTimerManager().SetTimer(
		ExplosionTimer,
		this,
		&ASingularityGrenade::SpawnSingularityTimerFinished,
		TimeToSpawnSingularity
	);
}

void ASingularityGrenade::SpawnSingularityTimerFinished()
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("Stop?"));
		SpawnSingularity();
		StartCollapseSingularityTimer();
		StartExplosionTimer();
	}
}

/*
* Collapse Singularity Timer
*/

void ASingularityGrenade::StartCollapseSingularityTimer()
{
	GetWorldTimerManager().SetTimer(
		ExplosionTimer,
		this,
		&ASingularityGrenade::CollapseSingularityTimerFinished,
		TimeToCollapseSingularity
	);
}

void ASingularityGrenade::CollapseSingularityTimerFinished()
{
	bUseGrenateAbility = false;
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/*
 * Ability
 */

void ASingularityGrenade::SpawnSingularity_Implementation()
{
	if(SingularitySystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SingularitySystem,
			GetActorLocation(),
			GetActorRotation(),
			GetActorScale(),
			false
		);
	}
	bUseGrenateAbility = true;

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void ASingularityGrenade::MoveCharacterToEpicenter(class ABlasterCharacter* Character, float DeltaTime)
{
	FVector ExplosionLocation = ProjectileMesh->GetComponentLocation();
	//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("MeshLocVector = X:%f, Y:%f, Z:%f"), ExplosionLocation.X, ExplosionLocation.Y, ExplosionLocation.Z));
	FVector CharacterLocation = Character->GetActorLocation();
	//FVector Epicenter = ExplosionTransform.GetLocation() + CharacterTransform.GetLocation() * CharCapsuleRadius;
	FVector Epicenter = FVector(ExplosionLocation.X, ExplosionLocation.Y, 90.f);
	//Путем умножения InterpolationSpeed на DeltaTime, мы добиваемся того, чтобы скорость была нормализована относительно времени и не зависела от FPS.
	FVector NewCharacterLocation = FMath::Lerp(CharacterLocation, Epicenter, DeltaTime * InterpolationSpeed);
	Character->SetActorLocation(NewCharacterLocation);
}

/*
* ExplosionTimer
*/

void ASingularityGrenade::StartExplosionTimer()
{
	GetWorldTimerManager().SetTimer(
		SingularityTimer,
		this,
		&ASingularityGrenade::ExplosionTimerFinished,
		TimeToExplosion
	);
}

void ASingularityGrenade::ExplosionTimerFinished()
{
	bUseGrenateAbility = false;
	Destroy();
}

void ASingularityGrenade::Destroyed()
{
	if (OverlappingCharacters.Num() > 0)
	{
		for (auto& Char : OverlappingCharacters)
		{
			Char->bDisableGameplay = false;
		}
	}

	Super::Destroyed();
}

