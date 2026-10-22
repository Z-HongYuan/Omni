// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/OmniImportReporter.h"

class UMaterialInterface;
class UMaterialInstanceConstant;
class UToonProfile;

namespace Omni::CharacterImport
{
UToonProfile* ResolveBodyToonProfileTemplate(UMaterialInterface* Parent, UToonProfile* ExplicitTemplate);
/** 设置实例覆盖并记录导入器所有权，标记待保存；不保存资产。 */
void SetOwnedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* Profile, const FString& GroupKey = FString());

/** 移除归属记录；只有仍指向原记录对象的覆盖会被清空，保留用户替换。 */
void ClearOwnedToonProfile(UMaterialInstanceConstant* Material);

/** 优先复用本组唯一持有的 Profile（支持移动和重命名），其次查找目标包，均不存在时复制模板。 */
UToonProfile* ResolveOrCreateSharedToonProfile(const FString& Package, const TArray<UMaterialInstanceConstant*>& ExistingMIs,
	UToonProfile* Template, const FImportReporter& Report);
/** 绑定已解析的组 Profile，不创建资产；Profile 为空时只移除导入器拥有的覆盖。 */
void ApplySharedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* SharedProfile, const FString& GroupKey,
	const FImportReporter& Report);
}
