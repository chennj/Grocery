#pragma once

#include <vector>
#include <map>
#include <limits>
#include <algorithm>
#include <string>

namespace DP
{
	//*****************
	//* 쳲������������� *
	//*****************
	class Fibonacci
	{
	public:
		// ---------------------------------------
		// 1�������ݹ�
		// ---------------------------------------
		// �㷨���Ӷȣ�o(2^n) * o(1) = o(2^n) ��2��n�η�
		int Fib(int n)
		{
			if (n == 1 || n == 2) return 1;
			return Fib(n - 1) + Fib(n - 2);
		}
		// ---------------------------------------

		// ---------------------------------------
		// 2.������¼�ĵݹ��㷨
		// ---------------------------------------
		// �ݹ��㷨��ʱ�临�Ӷ���ô���㣿������������������Խ��һ����������Ҫ��ʱ�䡣
		// �㷨���Ӷȣ��ݹ麯������ĸ��Ӷ� * �ݹ麯�����õĴ��� = o(1) * o(n) = o(n)
		// ʵ���ϣ���������¼���ĵݹ��㷨����һ�ô��ھ�������ĵݹ���ͨ������֦����
		// �������һ������������ĵݹ�ͼ����������������⣨���ݹ�ͼ�нڵ㣩�ĸ�����
		int fib(int N) {
			if (N < 1) return 0;
			// ����¼ȫ��ʼ��Ϊ 0
			std::vector<int> memo(N + 1, 0);
			// ���д�����¼�ĵݹ�
			return helper(memo, N);
		}

		int helper(std::vector<int>& memo, int n) {
			// base case
			if (n == 1 || n == 2) return 1;
			// �Ѿ������
			if (memo[n] != 0) return memo[n];
			memo[n] = helper(memo, n - 1) + helper(memo, n - 2);
			return memo[n];
		}
		// ---------------------------------------

		// ---------------------------------------
		// 3��dp ����ĵ����ⷨ
		// ---------------------------------------
		// �㷨���Ӷȣ�o(n)
		int fib_dp(int N) {
			if (N < 1) return 0;
			if (N == 1 || N == 2) return 1;
			std::vector<int> dp(N + 1, 0);
			// base case
			dp[1] = dp[2] = 1;
			for (int i = 3; i <= N; i++)
				dp[i] = dp[i - 1] + dp[i - 2];
			return dp[N];
		}

		// ---------------------------------------
		// 4�����Ž�
		// ---------------------------------------
		// �㷨���Ӷȣ�o(n) �ռ临�Ӷ�o(1)
		int fib_Optimal(int n) {
			if (n < 1) return 0;
			if (n == 2 || n == 1)
				return 1;
			int prev = 1, curr = 1;
			for (int i = 3; i <= n; i++) {
				int sum = prev + curr;
				prev = curr;
				curr = sum;
			}
			return curr;
		}
	};


	//************
	//* ����Ǯ���� *
	//************
	class Coins
	{
	public:
		// ʲô��״̬��
		// ״̬������״̬ת�ƹ����б仯����
		// dp�����У�״̬���������ݹ麯���У�״̬�ǲ���.

		// ״̬��Ŀ���� amount
		// ѡ��coins �������г�������Ӳ�����
		// �����Ķ��壺�ճ��ܽ��amount��������Ҫ CoinChange(coins, amount)öӲ��
		// base case��amount == 0 ʱ����Ҫ 0 öӲ�ҡ�amount < 0 ʱ�������ܴճ���
		// ˼·�� 
		// CoinChange({1,2,5}, 3, 11)
		// = min( CoinChange({1,2,5}, 3, 10), CoinChange({1,2,5}, 3, 9), CoinChange({1,2,5}, 3, 6) )

		// ---------------------------------------
		// 1�������ݹ�
		// ---------------------------------------
		// ʱ�临�Ӷȣ�ָ����
		// �������壺�ճ���� amount�� ������Ҫ CoinChange(amount)öӲ��
		int CoinChange(int* coins, int len, int amount) {
			// base case
			if (amount == 0)return 0;
			if (amount < 0) return -1;

			int res = std::numeric_limits<int>::max();
			for (int i = 0; i < len; i++) {
				// ����������Ľ��
				int subProblem = CoinChange(coins, len, amount - coins[i]);
				// �������޽�������
				if (subProblem == -1)continue;
				// ����������ѡ�����Ž⣬Ȼ���һ
				res = std::fmin(res, subProblem + 1);
			}
			
			return res == INT_MAX ? -1 : res;
		}

		// ---------------------------------------
		// 2���Զ����µݹ鷨
		// ---------------------------------------
		// ʱ�临�Ӷȣ��ݹ麯������ĸ��Ӷ� * �ݹ麯�����õĴ��� = o(k) * o(n) = o(kn)
		int * memo = nullptr;

		int CoinChange_TopDown(int* coins, int len, int amount) {
			memo = new int[amount + 1];
			// ����ȫ����ʼ��Ϊ����ֵ
			//memset(memo, -666, amount + 1);
			for (int i = 0; i < amount + 1; i++)memo[i] = -666;
			return dp(coins, len, amount);
		}

		int dp(int* coins, int len, int amount) {
			// base case
			if (amount == 0)return 0;
			if (amount < 0) return -1;
			// �鱸��¼����ֹ�ظ�����
			if (memo[amount] != -666) return memo[amount];

			int res = std::numeric_limits<int>::max();
			for (int i = 0; i < len; i++) {
				// ����������Ľ��
				int subProblem = dp(coins, len, amount - coins[i]);
				// �������޽�������
				if (subProblem == -1)continue;
				// ����������ѡ�����Ž⣬Ȼ���һ
				res = std::fmin(res, subProblem + 1);
			}
			// ��������뱸��¼
			memo[amount] = (res == INT_MAX) ? -1 : res;
			return memo[amount];
		}

		// ---------------------------------------
		// 3���Ե����ϵ����㷨
		// ---------------------------------------
		// dp���鶨�壺�ճ���� i��������Ҫ��Ҫdp[i]öӲ��
		int CoinChange_DownTop(int* coins, int len, int amount) {
			int* dp = new int[amount + 1];
			// ����ȫ����ʼ��Ϊ����ֵ
			for (int i = 0; i < amount + 1; i++)dp[i] = amount + 1;

			// base case
			dp[0] = 0;
			// ��� for ѭ����������״̬������ȡֵ
			for (int i = 0; i < amount + 1; i++) {
				// �ڲ� for ѭ��������ѡ�����Сֵ
				for (int coin = 0; coin < len; coin++) {
					// �������޽�������
					if (i - coins[coin] < 0)continue;
					// ״̬ת��
					dp[i] = std::fmin(dp[i], 1 + dp[i - coins[coin]]);
				}
			}
			// ������� amount �ܲ��ܴճ���
			return (dp[amount] == amount + 1) ? -1 : dp[amount];
		}
	public:
		~Coins();
	};


	//************
	//* ����Ǯ���� *
	//************
	class EditDistance
	{
	public:
		EditDistance(std::string & _s1, std::string & _s2);
	public:
		// ��Ŀ��
		// ������������word1��word2������������word1ת����word2��ʹ�õ����ٲ�������
		// ����Զ�һ�����ʽ����������ֲ���
		// 1. ����һ���ַ�
		// 2. ɾ��һ���ַ�
		// 3. �滻һ���ַ�
		// ����
		// ���룺word1 = "horse" word2 = "ros"
		// �����3
		// ���ͣ�
		// horse -> rorse����'h'�滻Ϊ'r'��
		// rorse -> rose��ɾ��'r'��
		// rose	 -> ros ��ɾ��'e'��

		std::string s1, s2;

		// ---------------------------------------
		// 1�������ݹ�
		// ---------------------------------------
		int MinDistance()
		{
			auto len1 = s1.size();
			auto len2 = s2.size();
			// ���壺dp(i, j) ����s1[0..i]��s2[0..j]����С�༭����
			// len1-1��s1�����һ���ַ���λ��
			// len2-1��s2�����һ���ַ���λ��
			return dp(len1-1, len2-1);
		}

		int dp(int i, int j)
		{
			// base case ������
			if (i == -1) return j + 1;
			if (j == -1) return i + 1;

			if (s1[i] == s2[j])
				return dp(i - 1, j - 1);		// ɶ������
			else
				return std::min({
					dp(i, j - 1) + 1,			// ��ʾ����
					dp(i - 1, j) + 1,			// ��ʾɾ��
					dp(i - 1, j - 1) + 1		// ��ʾ�滻
					});
		}

		// ---------------------------------------
		// 2��������¼�ĵݹ�
		// ---------------------------------------
		std::map<std::pair<int, int>, int> dict;

		int MinDistance_Memo()
		{
			auto len1 = s1.size();
			auto len2 = s2.size();
			// ���壺dp(i, j) ����s1[0..i]��s2[0..j]����С�༭����
			// len1-1��s1�����һ���ַ���λ��
			// len2-1��s2�����һ���ַ���λ��
			return dp_memo(len1 - 1, len2 - 1);
		}

		int dp_memo(int i, int j)
		{
			// base case ������
			if (i == -1) return j + 1;
			if (j == -1) return i + 1;

			auto& ite = dict.find(std::pair<int, int>(i, j));
			if (ite != dict.end())
				return ite->second;
			if (s1[i] == s2[j])
				dict[std::pair<int, int>(i, j)] = dp(i - 1, j - 1);		// ɶ������
			else
				dict[std::pair<int, int>(i, j)] = std::min({
					dp(i, j - 1) + 1,									// ��ʾ����
					dp(i - 1, j) + 1,									// ��ʾɾ��
					dp(i - 1, j - 1) + 1								// ��ʾ�滻
					});
			return dict[std::pair<int, int>(i, j)];
		}

		// ---------------------------------------
		// 3���Ե����ϵ����ⷨ
		// ---------------------------------------
		int MinDistance_DownTop()
		{
			int m = s1.size();
			int n = s2.size();

			// ���壺dp[i][j] ����s1[0..i-1]��s2[0..j-1]����С�༭����
			int **dp = (int **)malloc(sizeof(int *) * (m+1));
			for (int i = 0; i <= m; ++i)
			{
				dp[i] = (int *)malloc(sizeof(int) * (n+1));
			}

			// base case
			for (int i = 1; i <= m; i++) dp[i][0] = i;
			for (int j = 1; j <= n; j++) dp[0][j] = j;
			dp[0][0] = 0;

			// �Ե��������
			for (int i = 1; i <= m; i++) {
				for (int j = 1; j <= n; j++) {
					if (s1[i - 1] == s2[j - 1])
						dp[i][j] = dp[i - 1][j - 1];		// ɶ������
					else 
						dp[i][j] = std::min({
							dp[i][j - 1] + 1,				// ��ʾ����
							dp[i - 1][j] + 1,				// ��ʾɾ��
							dp[i - 1][j - 1] + 1			// ��ʾ�滻
							});
				}
			}

			int result = dp[m][n];
			for (int i = 0; i <= m; ++i)
				free(dp[i]);
			free(dp);

			return result;
		}

	};
}