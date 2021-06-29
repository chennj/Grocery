#include "Solution.h"

namespace DP
{
	Coins::~Coins()
	{
		if (memo) delete[] memo;
	}
}