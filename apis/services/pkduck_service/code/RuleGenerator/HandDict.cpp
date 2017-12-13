//
// Created by Wenbo Tao on 6/9/17.
//

#include "HandDict.h"
using namespace std;

HandDict::HandDict(string s)
{
	s = s.substr(0, s.rfind("/")) + "/hand_dict.txt";
	ifstream fin(s.c_str());

	string s1, s2;
	unordered_set<t_rule, rule_hash> rule_set;
	while (getline(fin, s1))
	{
		getline(fin, s2);
		getline(fin, s);
		rule_set.insert(make_pair(Common::get_tokens(s1), Common::get_tokens(s2)));
	}
	for (auto rule : rule_set)
		rules.push_back(rule);
}

vector<t_rule> HandDict::gen_rules()
{
	return rules;
}
