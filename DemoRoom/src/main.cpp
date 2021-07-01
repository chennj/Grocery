#include "common/Base.h"

#include "algorithm/DP/Solution.h"

int main()
{
	Scope<DP::Fibonacci> fib = CreateScope< DP::Fibonacci>();
	int result = fib->fib_Optimal(10);
	std::cout << "fib 10 = " << result << std::endl;

	Scope<DP::Coins> coins = CreateScope< DP::Coins>();
	int c[] = { 1,2,5 };
	result = coins->CoinChange_DownTop(c, sizeof(c)/sizeof(int), 11);
	std::cout << "coins 11 = " << result << std::endl;

	Scope<DP::EditDistance> distance = CreateScope< DP::EditDistance>(std::string("intention"), std::string("execution"));
	result = distance->MinDistance_DownTop();
	std::cout << "min distance = " << result << std::endl;

}