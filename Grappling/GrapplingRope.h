#pragma once

#include "CoreMinimal.h"
#include "Engine/SplineMeshActor.h"
#include "GrapplingRope.generated.h"

UCLASS()
class BLASTER_API AGrapplingRope : public ASplineMeshActor
{
	GENERATED_BODY()

public:
	void SetPoints(FVector EndPoint, FVector StartPoint, FVector StartTangent, FVector EndTanget);

};
