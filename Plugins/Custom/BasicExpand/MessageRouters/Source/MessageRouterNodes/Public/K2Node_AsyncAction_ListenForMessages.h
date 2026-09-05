// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "K2Node_AsyncAction.h"
#include "K2Node_AsyncAction_ListenForMessages.generated.h"

#define UE_API MESSAGEROUTERNODES_API

/**
 * 
 */
UCLASS(MinimalAPI)
class UK2Node_AsyncAction_ListenForMessage : public UK2Node_AsyncAction
{
	GENERATED_BODY()

public:
	//~UEdGraphNode interface
	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~End of UEdGraphNode interface

	//~UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	//~End of UK2Node interface

protected:
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
		UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext) override;

private:
	// 将 GetPayload 流程添加到代理处理器逻辑链的末端
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	// 确保输出的有效载荷通配符和输入有效载类型匹配
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
};

#undef UE_API
