// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/EquipmentDefinition.h"
#include "Core/EquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentDefinition)

UEquipmentDefinition::UEquipmentDefinition()
{
	InstanceType = UEquipmentInstance::StaticClass();
}
