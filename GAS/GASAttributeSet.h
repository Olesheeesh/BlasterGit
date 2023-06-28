#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASAttributeSet.generated.h"

//������ ATTRIBUTE_ACCESSORS ������������ ��� �������� ������� ������� � ���������.�� ���������� ������� ��� ������� ��������
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
	//GAMEPLAYATTRIBUTE_PROPERTY_GETTER -  ���������� �������-������, ������� ���������� ������ �������� � ��������� ������ �������� (PropertyName). ��������, ��� �������� Health ����� ������������� ������� GetHealthAttribute(), ������� ������ ������ �������� Health.
	//GAMEPLAYATTRIBUTE_VALUE_GETTER - ��������� �������-������, ������� ���������� ������ �������� ��������, ��� �������������� ����������. ��������, ��� �������� Health ����� ������������� ������� GetHealth(), ������� ������ ������� �������� �������� Health.
	//GAMEPLAYATTRIBUTE_VALUE_SETTER - ���������� �������-������, ������� ������������� ����� �������� ��������. ��������, ��� �������� Health ����� ������������� ������� SetHealthAttribute(), ������� �������� ���������� ����� �������� ��� �������� Health.
	//GAMEPLAYATTRIBUTE_VALUE_INITTER - ���������� ������� ������������� �������� ��������. ��������, ��� �������� Health ����� ������������� ������� InitHealthAttribute(), ������� ����� �������������� ��� ��������� ���������� �������� �������� Health.


UCLASS()
class BLASTER_API UGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGASAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	 * Health
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;//defining health attribute

	ATTRIBUTE_ACCESSORS(UGASAttributeSet, Health);//������������� ������� ATTRIBUTE_ACCESSORS ��� �������������� ��������� ������� ������� � �������� Health � ������ UGASAttributeSet.

	UFUNCTION()
	virtual void OnRep_Health(FGameplayAttributeData& OldHealth);

	/*
	 * AttackPower
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData AttackPower;//defining health attribute

	ATTRIBUTE_ACCESSORS(UGASAttributeSet, AttackPower);//macro to add getter and setter

	UFUNCTION()
	virtual void OnRep_AttackPower(FGameplayAttributeData& OldAttackPower);

	/*
	 * Uses
	 */

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Uses, Category = "Attributes")
	FGameplayAttributeData Uses;//defining uses attribute

	ATTRIBUTE_ACCESSORS(UGASAttributeSet, Uses);//macro to add getter and setter

	UFUNCTION()
	virtual void OnRep_Uses(FGameplayAttributeData& OldAttackPower);
}; 
