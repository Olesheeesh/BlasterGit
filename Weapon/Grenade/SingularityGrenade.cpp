#include "SingularityGrenade.h"

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
		StartCollapseSingularityTimer();
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
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("OverlappingCharacters.Num() = %d"), OverlappingCharacters.Num()));
				MoveCharacterToEpicenter(Char, DeltaTime);
			}
		}
	}
}

void ASingularityGrenade::StartCollapseSingularityTimer()
{
	GetWorldTimerManager().SetTimer(
		ExplosionTimer,
		this,
		&ASingularityGrenade::CollapseSingularityTimerFinished,
		TimeToSpawnSingularity
	);
}

void ASingularityGrenade::CollapseSingularityTimerFinished()
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("Stop?"));
		SpawnSingularity();
		StartExplosionTimer();
	}
}

void ASingularityGrenade::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->bDisableGameplay = true;
		OverlappingCharacters.Add(BlasterCharacter);
		
		//MoveCharacterToEpicenter(BlasterCharacter, GetWorld()->GetDeltaSeconds());
		//StartTimer();
	}
}

void ASingularityGrenade::SpawnSingularity()
{
	bUseGrenateAbility = true;
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void ASingularityGrenade::MoveCharacterToEpicenter(class ABlasterCharacter* Character, float DeltaTime)
{
	FTransform ExplosionTransform = ProjectileMesh->GetComponentTransform();
	FTransform CharacterTransform = Character->GetActorTransform();
	FVector Epicenter = ExplosionTransform.GetLocation() + CharacterTransform.GetLocation() * CharCapsuleRadius;
	FVector NewCharacterLocation = FMath::Lerp(CharacterTransform.GetLocation(), ExplosionTransform.GetLocation(), DeltaTime * InterpolationSpeed);
	Character->SetActorLocation(NewCharacterLocation);
}

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

