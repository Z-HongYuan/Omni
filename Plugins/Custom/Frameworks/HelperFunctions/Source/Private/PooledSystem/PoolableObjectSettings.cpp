// Copyright © 2026 张鸿源. All Rights Reserved.


#include "PooledSystem/PoolableObjectSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PoolableObjectSettings)

FText UPoolableObjectSettings::GetSectionDescription() const
{
	return NSLOCTEXT("PooledSystem", "PoolableObjectSettings",
	                 "1.Actor / Object 实现接口 PoolableObjectInterface，"
	                 "并在 OnPoolActivated / OnPoolDeactivated 中处理激活与休眠逻辑（如显示/隐藏、启用/禁用碰撞）。\n\n"
	                 "2.通过 UPoolableObjectManager::GetObject(Class) 获取实例，"
	                 "内部优先从休眠池唤醒（性能最优），池空时才创建新对象。"
	                 "你可以在下方 DefaultMaxSizes 中为不同 Class 设置创建上限。\n\n"
	                 "3.用完必须调用 UPoolableObjectManager::ReturnToPool(Object) 归还，"
	                 "切勿直接 Destroy——归还后会进入休眠池等待下次复用。");
}
