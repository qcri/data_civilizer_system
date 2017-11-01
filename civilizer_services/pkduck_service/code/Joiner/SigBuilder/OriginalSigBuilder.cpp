//
// Created by Wenbo Tao on 1/17/17.
//

#include "../Joiner.h"
using namespace std;

unordered_set<string> Joiner::buildOriginalSigs(int x)
{
	int len = (int) tokens[x].size();
	vector<pair<int, string>> sort_array;
	for (int i = 0; i < len; i ++)
		sort_array.emplace_back(token_rankings[tokens[x][i]], tokens[x][i]);
	sort(sort_array.begin(), sort_array.end());

	unordered_set<string> signatures;
	for (int i = 0; i < len; i ++)
		if (i + 1 <= len - ceil(len * Common::JAC_THRESHOLD) + 1)
			signatures.insert(sort_array[i].second);
		else
			break;

	return signatures;
}
