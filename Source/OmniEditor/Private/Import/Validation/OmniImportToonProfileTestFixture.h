// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Engine/ToonProfile.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionSubstrate.h"
#include "Materials/MaterialInstanceConstant.h"

namespace Omni::CharacterImport::Tests
{
inline const FMaterialParameterInfo DefaultProfileParameter(TEXT("FixtureToonProfile"));

/** 创建带原生 Toon Profile 参数的临时母材质。 */
inline UMaterial* MakeToonParent(UToonProfile* DefaultProfile = nullptr)
{
	UMaterial* Material = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
	auto* Toon = CastChecked<UMaterialExpressionSubstrateToonBSDF>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionSubstrateToonBSDF::StaticClass(), 0, 0));
	auto* Parameter = CastChecked<UMaterialExpressionSubstrateToonProfileParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionSubstrateToonProfileParameter::StaticClass(), -200, 0));
	Parameter->SetParameterName(DefaultProfileParameter.Name);
	Parameter->ToonProfile = DefaultProfile ? DefaultProfile : NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
	Toon->ToonProfile.Connect(0, Parameter);
	Material->GetEditorOnlyData()->FrontMaterial.Connect(0, Toon);
	Material->UpdateCachedExpressionData();
	return Material;
}

/** 测试母材质只声明一个 Profile，读取实例自身覆盖。 */
inline UToonProfile* GetLocalProfile(const UMaterialInstanceConstant* Material)
{
	return Material->ToonProfileParameterValues.Num() == 1 ? Material->ToonProfileParameterValues[0].ParameterValue.Get() : nullptr;
}
}
