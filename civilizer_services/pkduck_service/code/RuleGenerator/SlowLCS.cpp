//
// Created by Wenbo Tao on 12/27/16.
//

#include "SlowLCS.h"

SlowLCS::SlowLCS(vector<string> s)
	: RuleGenerator(s)
{
	//count occurrences
	occurrence.clear();
	for (int i = 0; i < n; i ++)
		for (int x = 0; x < (int) tokens[i].size(); x ++)
			for (int y = x; y < (int) tokens[i].size(); y ++)
			{
				string cur_string = "";
				for (int j = x; j < y; j ++)
					cur_string += tokens[i][j] + " ";
				cur_string += tokens[i][y];
				occurrence[cur_string] ++;
			}
}

vector<t_rule> SlowLCS::gen_rules()
{
	vector<t_rule> rules;
	unordered_set<string> lhs_set;
	for (int i = 0; i < n; i ++)
		for (int x = 0; x < (int) tokens[i].size(); x ++)
		{
			string cur_lhs = tokens[i][x];
			if (cur_lhs.size() < Common::LHS_SIZE_LIM)
				continue;
			if (lhs_set.count(cur_lhs))
				continue;
			lhs_set.insert(cur_lhs);
			for (auto cp : occurrence)
			{
				string cur_string = cp.first;

				//check whether cur_lhs is a constrainted-subsequence of cur_string
				if (Common::is_subseq_dp(cur_lhs, cur_string))
					rules.emplace_back(Common::get_tokens(cur_lhs), Common::get_tokens(cur_string));
			}
		}

	//add reverse rules
	for (int i = 0, n = (int) rules.size(); i < n; i ++)
		rules.push_back(make_pair(rules[i].second, rules[i].first));

	return rules;
}
