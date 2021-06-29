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

	// ����Ŀ��key��λ��
	int BTree::Query(const BTreeNode & node, int key)
	{
		// ����˵��
		// ------------------------------------------------------------------
		// node		: B���Ľ��
		// key		: �����ҵ�Ŀ��
		// return	: ����node��λ��
		// ------------------------------------------------------------------

		// ������ʱҶ�ӽ�㣬˵����û��childs
		if (node.IsLeaf)
		{
			// �ж��Ƿ����Ԫ��
			if (node.Length() == 0) return false;
			
			// ���ֲ���
			int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

			// ���û�ҵ�����-1
			if (pos == node.Length() || node.Keys[pos].Key != key) {
				return -1;
			} 
			return pos;
		}
		// �������Ҷ�ӽ��
		else
		{
			// ���ֲ���
			int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

			// ����ҵ�
			if (pos != node.Length() && node.Keys[pos].Key == key) {
				return pos;
			}

			// û�ҵ����ݹ�
			return Query(node.Childs[pos], key);
		}
	}

	// ����key�ĺ�̽��
	int BTree::Successor(const BTreeNode & node, int key, int backup)
	{
		// ����˵��
		// ------------------------------------------------------------------
		// node		: B���Ľ��
		// key		: �����ҵ�Ŀ��
		// backup	: ��ѡ���Ž�
		// return	: ����node��λ��
		// ------------------------------------------------------------------

		// ���ҵ�һ������key�Ľ���λ��
		int pos = std::lower_bound(node.Keys.begin(), node.Keys.end(), key) - node.Keys.end();

		// ���ʱҶ�ӽ�㣬��ô���ü����ݹ���
		if (node.IsLeaf)
		{
			// �ҵ��򷵻أ���Ϊ�������keyֵ����backupС
			if (pos < node.Length())
				return node.Keys[pos].Key;
			else
				return backup;
		}

		// û�ҵ���ݹ�
		if (pos == node.Length()) {
			return Successor(node.Childs[1], key, backup);
		}

		// �ҵ��˸��±�̥
		backup = node.Keys[pos].Key;

		// ���ܻ��и��õģ������ݹ飬�����µı�̥
		return Successor(node.Childs[pos], key, backup);
	}
}