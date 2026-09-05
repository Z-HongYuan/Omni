// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../DataTypes/Graph/CppGraph.h"
#include "./RandomHelper.h"

static bool randomIsExec(RandomHelper* randomHelper) { return randomHelper->randomBool(0.5); }

static Vector2 rerouteNodeSize() { return Vector2(10, 10); }

static Vector2 fullRandomNodePos(RandomHelper* randomHelper)
{
	return Vector2(randomHelper->randomDouble(-1000.0, 1000.0), randomHelper->randomDouble(-1000.0, 1000.0));
}

static Vector2 randomNodePos(RandomHelper* randomHelper, const Box& neighbourBox, bool isLeftBox)
{
	return Vector2(isLeftBox ? randomHelper->randomDouble(neighbourBox.Max.x + 100.0, neighbourBox.Max.x + 400.0)
							 : randomHelper->randomDouble(neighbourBox.Min.x - 700.0, neighbourBox.Min.x - 400.0),
		randomHelper->randomDouble(neighbourBox.Min.y - 300.0, neighbourBox.Max.y + 100.0));
}

static Vector2 randomNodeSize(RandomHelper* randomHelper)
{
	return Vector2(randomHelper->randomDouble(200.0, 350.0), randomHelper->randomDouble(150.0, 250.0));
}

static Box fullRandomNodeBox(RandomHelper* randomHelper, bool isRerouteNode)
{
	return Box::PosSize(fullRandomNodePos(randomHelper), isRerouteNode ? rerouteNodeSize() : randomNodeSize(randomHelper));
}

static Box randomNodeBox(RandomHelper* randomHelper, const Box& neighbourBox, bool isLeftBox, bool isRerouteNode)
{
	return Box::PosSize(
		randomNodePos(randomHelper, neighbourBox, isLeftBox), isRerouteNode ? rerouteNodeSize() : randomNodeSize(randomHelper));
}

static Vector2 randomPinOffset(RandomHelper* randomHelper,
	bool isLeft,
	const std::map<size_t, CppPin>& pinList,
	size_t nodeId,
	Vector2 nodeSize,
	bool isOwnerRerouteNode)
{
	if (isOwnerRerouteNode) return Vector2(isLeft ? -8 : 8, 0);
	auto maxPinOffsetYit = std::max_element(pinList.begin(),
		pinList.end(),
		[&](const auto& aPair, const auto& bPair)
		{
			const auto& a = aPair.second;
			const auto& b = bPair.second;
			if (a.ownerNodeId != nodeId || a.isLeft != isLeft) return true;
			if (b.ownerNodeId != nodeId || b.isLeft != isLeft) return false;
			return a.offset.y < b.offset.y;
		});
	double maxPinOffsetY = maxPinOffsetYit == pinList.end() || maxPinOffsetYit->second.ownerNodeId != nodeId
								   || maxPinOffsetYit->second.isLeft != isLeft
							   ? 0
							   : maxPinOffsetYit->second.offset.y;
	return Vector2(isLeft ? 15 : nodeSize.x - 15, maxPinOffsetY + randomHelper->randomDouble(12.0, 22.0));
}

inline void setPinExec(CppGraph* graph, size_t pinId, bool isExec)
{
	std::vector<size_t> pinIdToCheck = {pinId};
	for (size_t i = 0; i < pinIdToCheck.size(); ++i)
	{
		size_t pinId_ = pinIdToCheck[i];
		auto& pin = graph->pinList.at(pinId_);
		if (pin.isExec == isExec) continue;
		pin.isExec = isExec;
		for (auto& [_, link] : graph->linkList)
			if (pin.isLeft ? (link.rightPinId == pinId_) : (link.leftPinId == pinId_))
				pinIdToCheck.push_back(pin.isLeft ? link.leftPinId : link.rightPinId);
	}
}

class GraphBuilder;
class LinkBuilder;
class StartNodeBuilder;
class NodeBuilder;
class CommentBuilder;

class GraphBuilder
{
public:
	GraphBuilder(int64_t seed, const CppGraphConfig& graphConfig) : randomHelper(seed) { graph.graphConfig = graphConfig; }
	StartNodeBuilder addNode(bool isRerouteNode = false); // add a new node (not connected to anything)
	CommentBuilder addComment();						  // add a new comment (not containing any nodes)
	GraphBuilder exec();								  // set all pins as exec
	GraphBuilder nonExec();								  // set all pins as non exec

	const CppGraph& getGraph() const { return graph; }

	int64_t getSeed() const { return randomHelper.getSeed(); }

private:
	CppGraph graph;
	RandomHelper randomHelper;
};

class LinkBuilder
{
public:
	LinkBuilder(CppGraph* graph, RandomHelper* randomHelper, size_t linkId) :
		graph(graph), randomHelper(randomHelper), linkId(linkId)
	{
	}
	NodeBuilder linkRight(const LinkBuilder& link);		 // link the right pin of the link to the left pin of the given link
	NodeBuilder linkLeft(LinkBuilder& link);			 // link the left pin of the link to the right pin of the given link
	NodeBuilder linkRight(const StartNodeBuilder& node); // link the right pin of the link to the given node
	NodeBuilder linkLeft(StartNodeBuilder& node);		 // link the left pin of the link to the given node
	NodeBuilder linkRight(bool isRerouteNode = false);	 // link the left pin of the link to a new node
	NodeBuilder linkLeft(bool isRerouteNode = false);	 // link the right pin of the link to a new node

	LinkBuilder exec();	   // turn the link into exec link
	LinkBuilder nonExec(); // turn the link into non exec link

	LinkBuilder pinOffset(double leftOffset, double rightOffset); // set the offset in Y of the pins left and right

	NodeBuilder getNode(bool bLeft) const;

	size_t getLinkID() const { return linkId; }

private:
	LinkBuilder setExec(bool isExec); // set the link as exec or not (used by exec and nonExec methods)

	CppGraph* graph;
	RandomHelper* randomHelper;
	size_t linkId;
};

class StartNodeBuilder
{
public:
	StartNodeBuilder(CppGraph* graph, RandomHelper* randomHelper, size_t nodeId) :
		graph(graph), randomHelper(randomHelper), nodeId(nodeId)
	{
	}
	// create a new pin on the right side of the node and link it to the left pin of the given link
	NodeBuilder linkRight(LinkBuilder& link);
	// create a new pin on the left side of the node and link it to the right pin of the given link
	NodeBuilder linkLeft(LinkBuilder& link);
	// create a new pin on the right side of the node and link it to the given node
	NodeBuilder linkRight(const StartNodeBuilder& node);
	NodeBuilder linkLeft(StartNodeBuilder& node); // create a new pin on the left side of the node and link it to the given node
	NodeBuilder linkRight(bool isRerouteNode = false); // create a new pin on the right side of the node and link it to a new node
	NodeBuilder linkLeft(bool isRerouteNode = false);  // create a new pin on the left side of the node and link it to a new node
	StartNodeBuilder select();						   // select the node
	StartNodeBuilder comment(CommentBuilder& comment) const; // include the node in the given comment

	// move the current node below to the given node
	StartNodeBuilder placeBelow(const StartNodeBuilder& node);
	// move the current node below to the given comment
	StartNodeBuilder placeBelow(const CommentBuilder& comment);

	StartNodeBuilder nodeSize(Vector2 size); // set the size of the node

	size_t getNodeID() const { return nodeId; }

protected:
	CppGraph* graph;
	RandomHelper* randomHelper;
	size_t nodeId;
};

class NodeBuilder : public StartNodeBuilder
{
public:
	NodeBuilder(CppGraph* graph, RandomHelper* randomHelper, size_t nodeId, size_t linkId) :
		StartNodeBuilder(graph, randomHelper, nodeId), link(graph, randomHelper, linkId)
	{
	}
	NodeBuilder select();								// select the node
	NodeBuilder comment(CommentBuilder& comment) const; // include the node in the given comment

	// move the current node below to the given node
	NodeBuilder placeBelow(const StartNodeBuilder& node);
	// move the current node below to the given comment
	NodeBuilder placeBelow(const CommentBuilder& comment);
	NodeBuilder exec();	   // turn the created link into exec link
	NodeBuilder nonExec(); // turn the created link into non exec link

	NodeBuilder pinOffset(double leftOffset, double rightOffset); // set the offset in Y of the pins left and right

	NodeBuilder nodeSize(Vector2 size); // set the size of the node

	LinkBuilder& getLink() { return link; }

private:
	LinkBuilder link; // get the link that created this node
};

class CommentBuilder
{
public:
	CommentBuilder(CppGraph* graph, size_t commentId) : graph(graph), commentId(commentId) {}
	CommentBuilder select();							   // select the comment
	CommentBuilder comment(CommentBuilder& comment) const; // include the comment in the given comment

	size_t getCommentId() const { return commentId; }

private:
	CppGraph* graph;
	size_t commentId;
};

inline StartNodeBuilder GraphBuilder::addNode(bool isRerouteNode)
{
	size_t nodeId = graph.nodeList.size();
	auto nodeBox = fullRandomNodeBox(&randomHelper, isRerouteNode);
	graph.nodeList[nodeId] = CppNode(nodeId, "Node" + std::to_string(nodeId), nodeBox, isRerouteNode);
	return StartNodeBuilder(&graph, &randomHelper, nodeId);
}

inline CommentBuilder GraphBuilder::addComment()
{
	size_t commentId = graph.commentList.size();
	graph.commentList[commentId] = CppComment(commentId, "Comment " + std::to_string(commentId), Box(0, 0, 0, 0));
	return CommentBuilder(&graph, commentId);
}

inline GraphBuilder GraphBuilder::exec()
{
	for (auto& [_, pin] : graph.pinList) pin.isExec = true;
	return *this;
}

inline GraphBuilder GraphBuilder::nonExec()
{
	for (auto& [_, pin] : graph.pinList) pin.isExec = false;
	return *this;
}

inline NodeBuilder LinkBuilder::linkRight(const LinkBuilder& link)
{
	size_t leftPinId = graph->linkList.at(linkId).leftPinId;
	size_t rightPinId = graph->linkList.at(link.linkId).rightPinId;
	size_t newLinkId = graph->linkList.size();
	graph->linkList[newLinkId] = CppLink(newLinkId, leftPinId, rightPinId);
	setExec(graph->pinList.at(rightPinId).isExec); // ensure the exec flag is the same between the two pins
	size_t linkNodeId = graph->pinList.at(rightPinId).ownerNodeId;
	return NodeBuilder(graph, randomHelper, linkNodeId, newLinkId);
}

inline NodeBuilder LinkBuilder::linkLeft(LinkBuilder& link)
{
	auto nodeBuilder = link.linkRight(*this);
	auto linkLinkId = nodeBuilder.getLink().getLinkID();
	auto linkNodeId = graph->pinList[graph->linkList.at(linkLinkId).leftPinId].ownerNodeId;
	return NodeBuilder(graph, randomHelper, linkNodeId, linkLinkId);
}

inline NodeBuilder LinkBuilder::linkRight(const StartNodeBuilder& node)
{
	size_t leftPinId = graph->linkList.at(linkId).leftPinId;
	auto& cppNode = graph->nodeList.at(node.getNodeID());
	size_t newLinkId = graph->linkList.size();
	size_t rightPinId = graph->pinList.size();
	if (cppNode.isRerouteNode)
	{
		// get first left pin of reroute node
		for (size_t i = 0; i < graph->pinList.size(); ++i)
		{
			if (graph->pinList.at(i).ownerNodeId == node.getNodeID() && graph->pinList.at(i).isLeft)
			{
				rightPinId = i;
				break;
			}
		}
	}
	if (rightPinId == graph->pinList.size())
	{
		// create a new pin on the left side of the node
		size_t rightPinId_ = graph->pinList.size();
		auto isExec = graph->pinList.at(leftPinId).isExec;
		bool rightPinIsLeft = true;
		Vector2 nodeSize = cppNode.box.getSize();
		auto rightPinOffset
			= randomPinOffset(randomHelper, rightPinIsLeft, graph->pinList, node.getNodeID(), nodeSize, cppNode.isRerouteNode);
		graph->pinList[rightPinId_] = CppPin(rightPinId_,
			node.getNodeID(),
			"Node" + std::to_string(node.getNodeID()) + "Pin" + std::to_string(rightPinId_),
			isExec,
			rightPinIsLeft,
			rightPinOffset);
		graph->linkList[newLinkId] = CppLink(newLinkId, leftPinId, rightPinId_);
	}
	else
		graph->linkList[newLinkId] = CppLink(newLinkId, leftPinId, rightPinId);
	return NodeBuilder(graph, randomHelper, node.getNodeID(), newLinkId);
}

inline NodeBuilder LinkBuilder::linkLeft(StartNodeBuilder& node)
{
	auto nodeBuilder = node.linkRight(*this);
	return NodeBuilder(graph, randomHelper, node.getNodeID(), nodeBuilder.getLink().getLinkID());
}

inline NodeBuilder LinkBuilder::linkRight(bool isRerouteNode)
{
	size_t leftPinId = graph->linkList.at(linkId).leftPinId;
	size_t leftNodeId = graph->pinList.at(leftPinId).ownerNodeId;
	auto& leftNode = graph->nodeList.at(leftNodeId);

	size_t newNodeId = graph->nodeList.size();
	auto nodeBox = randomNodeBox(randomHelper, leftNode.box, true, isRerouteNode);
	graph->nodeList[newNodeId] = CppNode(newNodeId, "Node" + std::to_string(newNodeId), nodeBox, isRerouteNode);
	StartNodeBuilder startNodeBuilder(graph, randomHelper, newNodeId);
	return linkRight(startNodeBuilder);
}

inline NodeBuilder LinkBuilder::linkLeft(bool isRerouteNode)
{
	size_t rightPinId = graph->linkList.at(linkId).rightPinId;
	size_t rightNodeId = graph->pinList.at(rightPinId).ownerNodeId;
	auto& rightNode = graph->nodeList.at(rightNodeId);

	size_t newNodeId = graph->nodeList.size();
	auto nodeBox = randomNodeBox(randomHelper, rightNode.box, false, isRerouteNode);
	graph->nodeList[newNodeId] = CppNode(newNodeId, "Node" + std::to_string(newNodeId), nodeBox, isRerouteNode);
	StartNodeBuilder startNodeBuilder(graph, randomHelper, newNodeId);
	return linkLeft(startNodeBuilder);
}

inline LinkBuilder LinkBuilder::exec() { return setExec(true); }

inline LinkBuilder LinkBuilder::nonExec() { return setExec(false); }

inline LinkBuilder LinkBuilder::pinOffset(double leftOffset, double rightOffset)
{
	size_t leftPinId = graph->linkList.at(linkId).leftPinId;
	size_t rightPinId = graph->linkList.at(linkId).rightPinId;
	graph->pinList[leftPinId].offset.y = leftOffset;
	graph->pinList[rightPinId].offset.y = rightOffset;
	return *this;
}

inline NodeBuilder LinkBuilder::getNode(bool bLeft) const
{
	size_t pinId = bLeft ? graph->linkList.at(linkId).leftPinId : graph->linkList.at(linkId).rightPinId;
	size_t nodeId = graph->pinList.at(pinId).ownerNodeId;
	return NodeBuilder(graph, randomHelper, nodeId, linkId);
}

inline LinkBuilder LinkBuilder::setExec(bool isExec)
{
	size_t leftPinId = graph->linkList.at(linkId).leftPinId;
	setPinExec(graph, leftPinId, isExec);
	return *this;
}

inline NodeBuilder StartNodeBuilder::linkRight(LinkBuilder& link)
{
	size_t rightPinId = graph->linkList.at(link.getLinkID()).rightPinId;
	auto leftPinIsExec = graph->pinList.at(rightPinId).isExec;
	auto& cppNode = graph->nodeList.at(nodeId);
	size_t newLinkId = graph->linkList.size();
	size_t leftPinId = graph->pinList.size();
	if (cppNode.isRerouteNode)
	{
		// get first right pin of reroute node
		for (size_t i = 0; i < graph->pinList.size(); ++i)
		{
			if (graph->pinList.at(i).ownerNodeId == nodeId)
			{
				if (!graph->pinList.at(i).isLeft)
				{
					leftPinId = i;
					leftPinIsExec = graph->pinList.at(i).isExec;
					break;
				}
				else
					leftPinIsExec = graph->pinList.at(i).isExec;
			}
		}
	}
	if (leftPinId == graph->pinList.size())
	{
		// create a new pin on the right side of the node
		bool leftPinIsLeft = false;
		Vector2 nodeSize = cppNode.box.getSize();
		auto leftPinOffset
			= randomPinOffset(randomHelper, leftPinIsLeft, graph->pinList, nodeId, nodeSize, cppNode.isRerouteNode);
		graph->pinList[leftPinId] = CppPin(leftPinId,
			nodeId,
			"Node" + std::to_string(nodeId) + "Pin" + std::to_string(leftPinId),
			leftPinIsExec,
			leftPinIsLeft,
			leftPinOffset);
	}
	graph->linkList[newLinkId] = CppLink(newLinkId, leftPinId, rightPinId);
	size_t linkNodeId = graph->pinList.at(rightPinId).ownerNodeId;
	return NodeBuilder(graph, randomHelper, linkNodeId, newLinkId);
}

inline NodeBuilder StartNodeBuilder::linkLeft(LinkBuilder& link)
{
	auto nodeBuilder = link.linkRight(*this);
	auto linkLinkId = nodeBuilder.getLink().getLinkID();
	auto linkNodeId = graph->pinList[graph->linkList.at(linkLinkId).rightPinId].ownerNodeId;
	return NodeBuilder(graph, randomHelper, linkNodeId, linkLinkId);
}

inline NodeBuilder StartNodeBuilder::linkRight(const StartNodeBuilder& node)
{
	size_t newLinkId = graph->linkList.size();
	auto& leftCppNode = graph->nodeList.at(nodeId);
	size_t leftPinId = graph->pinList.size();
	auto leftPinIsExec = randomIsExec(randomHelper);
	if (leftCppNode.isRerouteNode)
	{
		// get first right pin of reroute node
		for (size_t i = 0; i < graph->pinList.size(); ++i)
		{
			if (graph->pinList.at(i).ownerNodeId == nodeId)
			{
				if (!graph->pinList.at(i).isLeft)
				{
					leftPinId = i;
					leftPinIsExec = graph->pinList.at(i).isExec;
					break;
				}
				else
					leftPinIsExec = graph->pinList.at(i).isExec;
			}
		}
	}
	if (leftPinId == graph->pinList.size())
	{
		// create a new pin on the right side of the node
		bool leftPinIsLeft = false;
		Vector2 leftNodeSize = leftCppNode.box.getSize();
		auto leftPinOffset
			= randomPinOffset(randomHelper, leftPinIsLeft, graph->pinList, nodeId, leftNodeSize, leftCppNode.isRerouteNode);
		graph->pinList[leftPinId] = CppPin(leftPinId,
			nodeId,
			"Node" + std::to_string(nodeId) + "Pin" + std::to_string(leftPinId),
			leftPinIsExec,
			leftPinIsLeft,
			leftPinOffset);
	}
	auto& rightCppNode = graph->nodeList.at(node.getNodeID());
	size_t rightPinId = graph->pinList.size();
	if (rightCppNode.isRerouteNode)
	{
		// get first left pin of reroute node
		for (size_t i = 0; i < graph->pinList.size(); ++i)
		{
			if (graph->pinList.at(i).ownerNodeId == node.getNodeID() && graph->pinList.at(i).isLeft)
			{
				rightPinId = i;
				setPinExec(graph, rightPinId, leftPinIsExec); // ensure the exec flag is the same between the two pins
				break;
			}
		}
	}
	if (rightPinId == graph->pinList.size())
	{
		// create a new pin on the left side of the node
		auto rightPinIsExec = leftPinIsExec;
		bool rightPinIsLeft = true;
		Vector2 rightNodeSize = rightCppNode.box.getSize();
		auto rightPinOffset = randomPinOffset(
			randomHelper, rightPinIsLeft, graph->pinList, node.getNodeID(), rightNodeSize, rightCppNode.isRerouteNode);
		graph->pinList[rightPinId] = CppPin(rightPinId,
			node.getNodeID(),
			"Node" + std::to_string(node.getNodeID()) + "Pin" + std::to_string(rightPinId),
			rightPinIsExec,
			rightPinIsLeft,
			rightPinOffset);
	}
	graph->linkList[newLinkId] = CppLink(newLinkId, leftPinId, rightPinId);
	return NodeBuilder(graph, randomHelper, node.getNodeID(), newLinkId);
}

inline NodeBuilder StartNodeBuilder::linkLeft(StartNodeBuilder& node)
{
	auto nodeBuilder = node.linkRight(*this);
	return NodeBuilder(graph, randomHelper, node.getNodeID(), nodeBuilder.getLink().getLinkID());
}

inline NodeBuilder StartNodeBuilder::linkRight(bool isRerouteNode)
{
	auto& leftNode = graph->nodeList.at(nodeId);

	size_t newNodeId = graph->nodeList.size();
	auto nodeBox = randomNodeBox(randomHelper, leftNode.box, true, isRerouteNode);
	graph->nodeList[newNodeId] = CppNode(newNodeId, "Node" + std::to_string(newNodeId), nodeBox, isRerouteNode);
	StartNodeBuilder startNodeBuilder(graph, randomHelper, newNodeId);
	return linkRight(startNodeBuilder);
}

inline NodeBuilder StartNodeBuilder::linkLeft(bool isRerouteNode)
{
	auto& rightNode = graph->nodeList.at(nodeId);

	size_t newNodeId = graph->nodeList.size();
	auto nodeBox = randomNodeBox(randomHelper, rightNode.box, false, isRerouteNode);
	graph->nodeList[newNodeId] = CppNode(newNodeId, "Node" + std::to_string(newNodeId), nodeBox, isRerouteNode);
	StartNodeBuilder startNodeBuilder(graph, randomHelper, newNodeId);
	return linkLeft(startNodeBuilder);
}

inline StartNodeBuilder StartNodeBuilder::select()
{
	graph->selectedNodeIdList.push_back(nodeId);
	return *this;
}

inline StartNodeBuilder StartNodeBuilder::comment(CommentBuilder& comment) const
{
	auto& graphComment = graph->commentList[comment.getCommentId()];
	auto& graphNode = graph->nodeList.at(nodeId);
	if (graphComment.box.getSize().x == 0) graphComment.box = graphNode.box;
	else
		graphComment.box += graphNode.box;
	return *this;
}

inline StartNodeBuilder StartNodeBuilder::placeBelow(const StartNodeBuilder& node)
{
	const auto& graphNode = graph->nodeList.at(node.getNodeID());
	auto& currentGraphNode = graph->nodeList.at(nodeId);
	double diffY = graphNode.box.Max.y + randomHelper->randomDouble(20, 50) - currentGraphNode.box.Min.y;
	currentGraphNode.box = currentGraphNode.box.offsetBox(Vector2(0, diffY));
	return *this;
}

inline StartNodeBuilder StartNodeBuilder::placeBelow(const CommentBuilder& comment)
{
	const auto& graphComment = graph->commentList[comment.getCommentId()];
	auto& currentGraphNode = graph->nodeList.at(nodeId);
	double diffY = graphComment.box.Max.y + randomHelper->randomDouble(20, 50) - currentGraphNode.box.Min.y;
	currentGraphNode.box = currentGraphNode.box.offsetBox(Vector2(0, diffY));
	return *this;
}

inline StartNodeBuilder StartNodeBuilder::nodeSize(Vector2 size)
{
	auto& currentGraphNode = graph->nodeList.at(nodeId);
	currentGraphNode.box = Box::PosSize(currentGraphNode.box.Min, size);
	return *this;
}

inline NodeBuilder NodeBuilder::select()
{
	StartNodeBuilder::select();
	return *this;
}

inline NodeBuilder NodeBuilder::comment(CommentBuilder& comment) const
{
	StartNodeBuilder::comment(comment);
	return *this;
}

inline NodeBuilder NodeBuilder::placeBelow(const StartNodeBuilder& node)
{
	StartNodeBuilder::placeBelow(node);
	return *this;
}

inline NodeBuilder NodeBuilder::placeBelow(const CommentBuilder& comment)
{
	StartNodeBuilder::placeBelow(comment);
	return *this;
}

inline NodeBuilder NodeBuilder::exec()
{
	link.exec();
	return *this;
}

inline NodeBuilder NodeBuilder::nonExec()
{
	link.nonExec();
	return *this;
}

inline NodeBuilder NodeBuilder::pinOffset(double leftOffset, double rightOffset)
{
	link.pinOffset(leftOffset, rightOffset);
	return *this;
}

inline NodeBuilder NodeBuilder::nodeSize(Vector2 size)
{
	StartNodeBuilder::nodeSize(size);
	return *this;
}

inline CommentBuilder CommentBuilder::select()
{
	graph->selectedCommentIdList.push_back(commentId);
	return *this;
}
inline CommentBuilder CommentBuilder::comment(CommentBuilder& comment) const
{
	auto& graphComment = graph->commentList[comment.getCommentId()];
	auto& currentComment = graph->commentList[commentId];
	if (graphComment.box.getSize().x == 0) graphComment.box = currentComment.box;
	else
		graphComment.box += currentComment.box;
	return *this;
}