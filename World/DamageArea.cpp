#include "DamageArea.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values
ADamageArea::ADamageArea()
{
	bReplicates = true; //class is replicated(spawns on all machines) will exis independetly of the server
	PrimaryActorTick.bCanEverTick = false;

	KillZoneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));//создаём мэш в BP
	KillZoneMesh->SetupAttachment(RootComponent);//прикрепляем его к RootComponent
	SetRootComponent(KillZoneMesh);//делаем мэш RootComponent

	//Mesh Collision
	KillZoneMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	KillZoneMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	KillZoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//AreaBox Collision
	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaSphere"));
	AreaBox->SetupAttachment(RootComponent);
	AreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ADamageArea::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{//for server
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaBox->OnComponentBeginOverlap.AddDynamic(this, &ADamageArea::OnBoxOverlap);
		AreaBox->OnComponentEndOverlap.AddDynamic(this, &ADamageArea::OnBoxEndOverlap);
	}
}

void ADamageArea::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(OtherActor);
	Actor = OtherActor;
	if (OwnerCharacter)
	{
		GetWorldTimerManager().SetTimer(
			KillZoneTimer,
			this,
			&ADamageArea::UpdateDamage,
			DamageDelay,
			true
		);
	}
}

void ADamageArea::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (OwnerCharacter)
	{
		GetWorldTimerManager().ClearTimer(KillZoneTimer);
	}
	if (GEngine)
	{
		FString DebugMessage = TEXT("You Survived!");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugMessage);
	}
}

void ADamageArea::UpdateDamage()
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(Actor);
	AController* OwnerController = OwnerCharacter->Controller;
	UGameplayStatics::ApplyDamage(OwnerCharacter, Damage, OwnerController, this, UDamageType::StaticClass());
}


// Called every frame


