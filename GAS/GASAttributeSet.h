#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASAttributeSet.generated.h"

//Макрос ATTRIBUTE_ACCESSORS используется для создания функций доступа к атрибутам.Он определяет функции для каждого атрибута
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
	//GAMEPLAYATTRIBUTE_PROPERTY_GETTER -  генерирует функцию-геттер, которая возвращает объект атрибута с указанным именем свойства (PropertyName). Например, для атрибута Health будет сгенерирована функция GetHealthAttribute(), которая вернет объект атрибута Health.
	//GAMEPLAYATTRIBUTE_VALUE_GETTER - енерирует функцию-геттер, которая возвращает только значение атрибута, без дополнительной информации. Например, для атрибута Health будет сгенерирована функция GetHealth(), которая вернет текущее значение атрибута Health.
	//GAMEPLAYATTRIBUTE_VALUE_SETTER - генерирует функцию-сеттер, которая устанавливает новое значение атрибута. Например, для атрибута Health будет сгенерирована функция SetHealthAttribute(), которая позволит установить новое значение для атрибута Health.
	//GAMEPLAYATTRIBUTE_VALUE_INITTER - генерирует функцию инициализации значения атрибута. Например, для атрибута Health будет сгенерирована функция InitHealthAttribute(), которая может использоваться для установки начального значения атрибута Health.


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

	ATTRIBUTE_ACCESSORS(UGASAttributeSet, Health);//использование макроса ATTRIBUTE_ACCESSORS для автоматической генерации функций доступа к атрибуту Health в классе UGASAttributeSet.

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
