// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/EngineBaseTypes.h"

/*
 * 项目中使用的物理通道定义
 *
 * 使用方式：代码里一律引用 Omni_TraceChannel_XXX，不要直接写 ECC_GameTraceChannelN，
 * 这样将来调整通道分配时只需要改这一个文件。
 *
 * 两个必须遵守的约定：
 * 1. 改动这里的通道分配时，必须同步改 Config/DefaultEngine.ini 里的
 *    [/Script/Engine.CollisionProfile] 段的 +DefaultChannelResponses，两者必须一一对应，
 *    否则代码用的通道号和引擎实际配置的通道会对不上。
 * 2. 通道号一旦被资产（碰撞预设、物理材质、蓝图里的检测设置）序列化引用就不能再挪位置，
 *    新需求只能往后追加，否则旧资产会静默地指向别的通道。
 */

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

/*
 * 引擎总共提供 18 个自定义 Trace 通道（ECC_GameTraceChannel1 ~ 18），
 * 6 及以后全部空闲，留给后续玩法插件按需追加。
 */

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
