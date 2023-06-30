#include "GrapplingRope.h"
#include "Components/SplineMeshComponent.h"

void AGrapplingRope::SetPoints(FVector EndPoint, FVector StartPoint, FVector StartTangent, FVector EndTangent)
{
	FTransform ActorTransform = GetActorTransform();
	FVector LocalLocation = ActorTransform.InverseTransformPosition(StartPoint);
	FVector StartPos = LocalLocation;
	FVector EndPos = ActorTransform.InverseTransformPosition(EndPoint);
	GetSplineMeshComponent()->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
}
