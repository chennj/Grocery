#include "BTree.h"

namespace algorithm_demo
{
	BTree::BTree()
	{
		m_Node = std::make_shared<BTreeNode>();
	}
	BTree::~BTree()
	{
	}

	// 查找目标key的位置
	int BTree::Query(const BTreeNode & node, int key)
	{
		// 参数说明
		// ------------------------------------------------------------------
		// node		: B树的结点
		// key		: 待查找的目标
		// return	: 返回node的位置
		// ------------------------------------------------------------------

		// 如果结点时叶子结点，说明它没有childs
		if (node.IsLeaf)
		{
			// 判断是否存在元素
			if (node.Length() == 0) return false;
			
			// 二分查找
			int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

			// 如果没找到返回-1
			if (pos == node.Length() || node.Keys[pos].Key != key) {
				return -1;
			} 
			return pos;
		}
		// 如果不是叶子结点
		else
		{
			// 二分查找
			int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

			// 如果找到
			if (pos != node.Length() && node.Keys[pos].Key == key) {
				return pos;
			}

			// 没找到，递归
			return Query(node.Childs[pos], key);
		}
	}

	// 查找key的后继结点
	int BTree::Successor(const BTreeNode & node, int key, int backup)
	{
		// 参数说明
		// ------------------------------------------------------------------
		// node		: B树的结点
		// key		: 待查找的目标
		// backup	: 备选最优解
		// return	: 返回node的位置
		// ------------------------------------------------------------------

		// 查找第一个大于key的结点的位置
		int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

		// 如果时叶子结点，那么不用继续递归了
		if (node.IsLeaf)
		{
			// 找到则返回，因为子树里的key值都比backup小
			if (pos < node.Length())
				return node.Keys[pos].Key;
			else
				return backup;
		}

		// 没找到则递归
		if (pos == node.Length()) {
			return Successor(node.Childs[1], key, backup);
		}

		// 找到了更新备胎
		backup = node.Keys[pos].Key;

		// 可能还有更好的，继续递归，传入新的备胎
		return Successor(node.Childs[pos], key, backup);
	}
}