//
// Created by Wenbo Tao on 12/27/16.
//

#include "BruteForceJoiner.h"
using namespace std;

BruteForceJoiner::BruteForceJoiner(vector<t_rule> r, vector<string> s)
	: Joiner(r, s)
{
}

vector<pair<double, pair<string, string>>> BruteForceJoiner::getJoinedStringPairs()
{
	vector<pair<double, pair<string, string>>> ans_pairs;

	//check
	for (int i = 0; i < n; i ++)
		for (int j = i + 1; j < n; j ++)
		{
			vector<int> cur_applicable;
			for (int k = 0; k < (int) applicable_rule_ids[i].size(); k ++)
			{
				t_rule rule = rules[applicable_rule_ids[i][k]];
				bool applicable = true;
				umpsi tmp_map = token_maps[i];
				for (string t : rule.first)
					if (! tmp_map.count(t) || tmp_map[t] == 0)
					{
						applicable = false;
						break;
					}
					else
						tmp_map[t] --;
				for (string t : rule.second)
					if (! expansion_set[j].count(t))
					{
						applicable = false;
						break;
					}
				if (! applicable)
					continue;
				cur_applicable.push_back(applicable_rule_ids[i][k]);
			}
			if (cur_applicable.size() > Common::APPLICABLE_THRESHOLD)
				continue;

			//brute-force
			int M = 1 << (cur_applicable.size());
			double max_sim = 0;
			for (int mask = 0; mask < M; mask ++)
			{
				umpsi cur_map = token_maps[i];
				bool applicable = true;
				for (int k = 0; k < (int) cur_applicable.size(); k ++)
				{
					if (! (mask & (1 << k)))
						continue;
					t_rule cur_rule = rules[cur_applicable[k]];
					for (string s : cur_rule.first)
						if (! cur_map.count(s) || cur_map[s] == 0)
							applicable = false;
						else
							cur_map[s] --;
					if (! applicable)
						break;
				}
				if (! applicable)
					continue;
				for (int k = 0; k < (int) cur_applicable.size(); k ++)
				{
					if (!(mask & (1 << k)))
						continue;
					t_rule cur_rule = rules[cur_applicable[k]];
					for (string s : cur_rule.second)
						cur_map[s] ++;
				}
				max_sim = max(max_sim, Common::jaccard(cur_map, token_maps[j]));
			}
			if (max_sim >= Common::JAC_THRESHOLD)
				ans_pairs.emplace_back(max_sim, make_pair(cells[i], cells[j]));
		}

	//return
	return ans_pairs;
}
