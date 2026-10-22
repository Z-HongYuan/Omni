// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/EngineBaseTypes.h"

// 本文件保留碰撞通道规划与配置示例，以下宏和配置均处于注释状态。
// 启用通道时，代码中的分配必须与 DefaultEngine.ini 的 CollisionProfile 配置对应。
// 资产序列化引用通道号后，调整分配需同时考虑已有碰撞预设和蓝图引用。

//~交互检测
// CustomInteraction 的 AbilityTask_WaitInteractableTargets_SingleLineTrace 使用
// 对应碰撞预设 Interactable_OverlapDynamic / Interactable_BlockDynamic
// #define Omni_TraceChannel_Interaction					ECC_GameTraceChannel1

//~武器检测
// 命中物理资产（PhysicsAsset）而不是胶囊体，用于需要骨骼级精度的命中
// #define Omni_TraceChannel_Weapon						ECC_GameTraceChannel2

// 命中 Pawn 胶囊体而不是物理资产，用于不需要骨骼级精度的快速检测
// #define Omni_TraceChannel_Weapon_Capsule				ECC_GameTraceChannel3

// 穿透型检测，命中多个目标而不在第一个命中处停止（霰弹、穿透弹）
// #define Omni_TraceChannel_Weapon_Multi					ECC_GameTraceChannel4

//~辅助瞄准
// 预留给射击类玩法插件，目前未使用
// #define Omni_TraceChannel_AimAssist						ECC_GameTraceChannel5

// 以下配置与前面的通道宏对应，保留为未启用的配置示例。

// [/Script/Engine.CollisionProfile]
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Omni_TraceChannel_Interaction")
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Omni_TraceChannel_Weapon")
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel3,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Omni_TraceChannel_Weapon_Capsule")
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel4,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Omni_TraceChannel_Weapon_Multi")
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel5,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Omni_TraceChannel_AimAssist")
// +Profiles=(Name="OmniPawnMesh",CollisionEnabled=QueryOnly,bCanModify=True,ObjectTypeName="Pawn",CustomResponses=((Channel="WorldStatic",Response=ECR_Ignore),(Channel="Pawn",Response=ECR_Ignore),(Channel="Camera",Response=ECR_Ignore),(Channel="PhysicsBody",Response=ECR_Ignore),(Channel="Vehicle",Response=ECR_Ignore),(Channel="Destructible",Response=ECR_Ignore),(Channel="Omni_TraceChannel_Weapon_Multi",Response=ECR_Overlap),(Channel="Omni_TraceChannel_Weapon")),HelpMessage="Character mesh")
// +Profiles=(Name="OmniPawnCapsule",CollisionEnabled=QueryOnly,bCanModify=True,ObjectTypeName="Pawn",CustomResponses=((Channel="Camera",Response=ECR_Ignore),(Channel="Destructible",Response=ECR_Ignore),(Channel="Omni_TraceChannel_Weapon_Capsule"),(Channel="Omni_TraceChannel_Weapon_Multi",Response=ECR_Overlap)),HelpMessage="Character capsule")
// +Profiles=(Name="Interactable_OverlapDynamic",CollisionEnabled=QueryOnly,bCanModify=True,ObjectTypeName="PhysicsBody",CustomResponses=((Channel="Pawn",Response=ECR_Overlap),(Channel="Visibility",Response=ECR_Ignore),(Channel="Camera",Response=ECR_Ignore),(Channel="PhysicsBody",Response=ECR_Ignore),(Channel="Vehicle",Response=ECR_Ignore),(Channel="Destructible",Response=ECR_Ignore),(Channel="Omni_TraceChannel_Interaction",Response=ECR_Overlap),(Channel="Omni_TraceChannel_Weapon_Multi",Response=ECR_Ignore)),HelpMessage="")
// +Profiles=(Name="Interactable_BlockDynamic",CollisionEnabled=QueryAndPhysics,bCanModify=True,ObjectTypeName="WorldDynamic",CustomResponses=((Channel="Omni_TraceChannel_Interaction",Response=ECR_Overlap)),HelpMessage="")
// +EditProfiles=(Name="BlockAll",CustomResponses=((Channel="Omni_TraceChannel_Interaction"),(Channel="Omni_TraceChannel_Weapon"),(Channel="Omni_TraceChannel_Weapon_Capsule"),(Channel="Omni_TraceChannel_Weapon_Multi")))
// +EditProfiles=(Name="BlockAllDynamic",CustomResponses=((Channel="Omni_TraceChannel_Interaction"),(Channel="Omni_TraceChannel_Weapon"),(Channel="Omni_TraceChannel_Weapon_Capsule"),(Channel="Omni_TraceChannel_Weapon_Multi")))
// +EditProfiles=(Name="InvisibleWall",CustomResponses=((Channel="Omni_TraceChannel_Interaction"),(Channel="Omni_TraceChannel_Weapon"),(Channel="Omni_TraceChannel_Weapon_Capsule"),(Channel="Omni_TraceChannel_Weapon_Multi")))
// +EditProfiles=(Name="InvisibleWallDynamic",CustomResponses=((Channel="Omni_TraceChannel_Interaction"),(Channel="Omni_TraceChannel_Weapon"),(Channel="Omni_TraceChannel_Weapon_Capsule"),(Channel="Omni_TraceChannel_Weapon_Multi")))
