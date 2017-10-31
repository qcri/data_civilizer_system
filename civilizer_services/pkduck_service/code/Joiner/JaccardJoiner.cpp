//
// Created by Wenbo Tao on 2/11/17.
//

#include "JaccardJoiner.h"
using namespace std;

JaccardJoiner::JaccardJoiner(vector<t_rule> r, vector<string> s, double threshold)
	: Joiner(r, s)
{
	this->threshold = threshold;
}

vector<pair<double, pair<string, string>>> JaccardJoiner::getJoinedStringPairs()
{
	//build inverted lists
	unordered_map<string, vector<int>> inv_list;
	for (int i = 0; i < n; i ++)
		for (string t : o_sigs[i])
			inv_list[t].push_back(i);

	//generate candidates
	unordered_set<pair<int, int>, pairii_hash> candidates;
	for (int i = 0; i < n; i ++)
	{
		unordered_set<int> cur_set;
		for (string t : o_sigs[i])
			for (int v : inv_list[t])
				if (v != i)
					cur_set.insert(v);
		for (int v : cur_set)
		{
			pair<int, int> cur_cp = make_pair(min(i, v), max(i, v));
			candidates.insert(cur_cp);
		}
	}

	//verify
	vector<pair<double, pair<string, string>>> ans;
	for (auto cp : candidates)
	{
		int x = cp.first;
		int y = cp.second;
		double sim = Common::jaccard(token_maps[x], token_maps[y]);
		if (sim >= threshold)
			ans.emplace_back(sim, make_pair(cells[x], cells[y]));
	}
	return ans;
}
