#include "Solution.h"

namespace DP
{
	Coins::~Coins()
	{
		if (memo) delete[] memo;
	}
	EditDistance::EditDistance(std::string & _s1, std::string & _s2)
		:s1(_s1),s2(_s2)
	{
	}
}