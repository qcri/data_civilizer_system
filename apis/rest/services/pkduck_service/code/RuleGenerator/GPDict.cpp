//
// Created by Wenbo Tao on 6/18/17.
//

#include "GPDict.h"
using namespace std;

GPDict::GPDict()
{
	string s = "data/gp_dict.txt";
	ifstream fin(s.c_str());

	string s1, s2;
	unordered_set<t_rule, rule_hash> rule_set;
	while (getline(fin, s1))
	{
		getline(fin, s2);
		getline(fin, s);
		vector<string> tokens_s1 = Common::get_tokens(s1);
		vector<string> tokens_s2 = Common::get_tokens(s2);
		if (tokens_s1.size() == 0 || tokens_s2.size() == 0)
			continue;
		rule_set.insert(make_pair(tokens_s1, tokens_s2));
	}
	for (auto rule : rule_set)
		rules.push_back(rule);
}

vector<t_rule> GPDict::gen_rules()
{
	return rules;
}