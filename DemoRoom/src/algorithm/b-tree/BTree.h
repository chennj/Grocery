#pragma once

#include <memory>
#include <vector>
#include <algorithm>

namespace algorithm_demo
{
	template<typename T>
	struct NodeData
	{
		T Key;
		NodeData() {}
		NodeData(T _key) { Key = _key; }
		bool operator<(const NodeData & m)const { //定义比较方式，这一步很重要
			return Key < m.Key;
		}
	};

	struct BTreeNode
	{
		std::vector<NodeData<int>> Keys;
		std::vector<BTreeNode> Childs;
		bool IsLeaf = false;

		int Length() const {
			return Keys.size();
		}

	};

	class BTree
	{
	public:
		std::shared_ptr<BTreeNode> m_Node;

	public:
		BTree();
		~BTree();

	public:
		static int Query(const BTreeNode & node, int key);
		static int Successor(const BTreeNode & node, int key, int backup);
	};
}