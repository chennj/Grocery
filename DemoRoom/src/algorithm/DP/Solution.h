#pragma once

#include <vector>
#include <limits>

namespace DP
{
	//*****************
	//* 斐波那契数列问题 *
	//*****************
	class Fibonacci
	{
	public:
		// ---------------------------------------
		// 1、暴力递归
		// ---------------------------------------
		// 算法复杂度：o(2^n) * o(1) = o(2^n) 即2的n次方
		int Fib(int n)
		{
			if (n == 1 || n == 2) return 1;
			return Fib(n - 1) + Fib(n - 2);
		}
		// ---------------------------------------

		// ---------------------------------------
		// 2.带备忘录的递归算法
		// ---------------------------------------
		// 递归算法的时间复杂度怎么计算？就是用子问题个数乘以解决一个子问题需要的时间。
		// 算法复杂度：递归函数本身的复杂度 * 递归函数调用的次数 = o(1) * o(n) = o(n)
		// 实际上，带「备忘录」的递归算法，把一棵存在巨量冗余的递归树通过「剪枝」，
		// 改造成了一幅不存在冗余的递归图，极大减少了子问题（即递归图中节点）的个数。
		int fib(int N) {
			if (N < 1) return 0;
			// 备忘录全初始化为 0
			std::vector<int> memo(N + 1, 0);
			// 进行带备忘录的递归
			return helper(memo, N);
		}

		int helper(std::vector<int>& memo, int n) {
			// base case
			if (n == 1 || n == 2) return 1;
			// 已经计算过
			if (memo[n] != 0) return memo[n];
			memo[n] = helper(memo, n - 1) + helper(memo, n - 2);
			return memo[n];
		}
		// ---------------------------------------

		// ---------------------------------------
		// 3、dp 数组的迭代解法
		// ---------------------------------------
		// 算法复杂度：o(n)
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
		// 4、最优解
		// ---------------------------------------
		// 算法复杂度：o(n) 空间复杂度o(1)
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
	//* 凑零钱问题 *
	//************
	class Coins
	{
	public:
		// 什么是状态？
		// 状态就是在状态转移过程中变化的量
		// dp数组中，状态是索引；递归函数中，状态是参数.

		// 状态：目标金额 amount
		// 选择：coins 数组中列出的所有硬币面额
		// 函数的定义：凑出总金额amount，至少需要 CoinChange(coins, amount)枚硬币
		// base case：amount == 0 时，需要 0 枚硬币。amount < 0 时，不可能凑出。
		// 思路： 
		// CoinChange({1,2,5}, 3, 11)
		// = min( CoinChange({1,2,5}, 3, 10), CoinChange({1,2,5}, 3, 9), CoinChange({1,2,5}, 3, 6) )

		// ---------------------------------------
		// 1、暴力递归
		// ---------------------------------------
		// 时间复杂度：指数级
		// 函数定义：凑出金额 amount， 至少需要 CoinChange(amount)枚硬币
		int CoinChange(int* coins, int len, int amount) {
			// base case
			if (amount == 0)return 0;
			if (amount < 0) return -1;

			int res = std::numeric_limits<int>::max();
			for (int i = 0; i < len; i++) {
				// 计算子问题的结果
				int subProblem = CoinChange(coins, len, amount - coins[i]);
				// 子问题无解则跳过
				if (subProblem == -1)continue;
				// 在子问题中选择最优解，然后加一
				res = std::fmin(res, subProblem + 1);
			}
			
			return res == INT_MAX ? -1 : res;
		}

		// ---------------------------------------
		// 2、自顶向下递归法
		// ---------------------------------------
		// 时间复杂度：递归函数本身的复杂度 * 递归函数调用的次数 = o(k) * o(n) = o(kn)
		int * memo = nullptr;

		int CoinChange_TopDown(int* coins, int len, int amount) {
			memo = new int[amount + 1];
			// 数组全部初始化为特殊值
			//memset(memo, -666, amount + 1);
			for (int i = 0; i < amount + 1; i++)memo[i] = -666;
			return dp(coins, len, amount);
		}

		int dp(int* coins, int len, int amount) {
			// base case
			if (amount == 0)return 0;
			if (amount < 0) return -1;
			// 查备忘录，防止重复计算
			if (memo[amount] != -666) return memo[amount];

			int res = std::numeric_limits<int>::max();
			for (int i = 0; i < len; i++) {
				// 计算子问题的结果
				int subProblem = dp(coins, len, amount - coins[i]);
				// 子问题无解则跳过
				if (subProblem == -1)continue;
				// 在子问题中选择最优解，然后加一
				res = std::fmin(res, subProblem + 1);
			}
			// 将结果存入备忘录
			memo[amount] = (res == INT_MAX) ? -1 : res;
			return memo[amount];
		}

		// ---------------------------------------
		// 3、自底向上迭代算法
		// ---------------------------------------
		// dp数组定义：凑出金额 i，至少需要需要dp[i]枚硬币
		int CoinChange_DownTop(int* coins, int len, int amount) {
			int* dp = new int[amount + 1];
			// 数组全部初始化为特殊值
			for (int i = 0; i < amount + 1; i++)dp[i] = amount + 1;

			// base case
			dp[0] = 0;
			// 外层 for 循环遍历所有状态的所有取值
			for (int i = 0; i < amount + 1; i++) {
				// 内层 for 循环求所有选择的最小值
				for (int coin = 0; coin < len; coin++) {
					// 子问题无解则跳过
					if (i - coins[coin] < 0)continue;
					// 状态转移
					dp[i] = std::fmin(dp[i], 1 + dp[i - coins[coin]]);
				}
			}
			// 看看金额 amount 能不能凑出来
			return (dp[amount] == amount + 1) ? -1 : dp[amount];
		}
	public:
		~Coins();
	};
}