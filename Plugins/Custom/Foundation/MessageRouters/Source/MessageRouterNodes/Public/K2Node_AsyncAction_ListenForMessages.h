// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "K2Node_AsyncAction.h"
#include "K2Node_AsyncAction_ListenForMessages.generated.h"

#define UE_API MESSAGEROUTERNODES_API

/** 消息监听专用蓝图节点，按负载类型生成输出引脚和事件处理流程。 */
UCLASS(MinimalAPI)
class UK2Node_AsyncAction_ListenForMessage : public UK2Node_AsyncAction
{
	GENERATED_BODY()

public:
	//~UEdGraphNode 接口
	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~UEdGraphNode 接口结束

	//~UK2Node 接口
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	//~UK2Node 接口结束

protected:
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
		UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext) override;

private:
	/** 在代理事件执行链末端接入 GetPayload 和负载赋值流程。 */
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	/** 使负载输出通配符与输入的消息结构类型一致。 */
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
};

#undef UE_API
