#pragma once

#include <vector>
#include <limits>

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
}