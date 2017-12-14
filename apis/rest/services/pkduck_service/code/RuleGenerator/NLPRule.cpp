//
// Created by Wenbo Tao on 1/28/17.
//

#include "NLPRule.h"
#include "FastLCS.h"
using namespace std;

NLPRule::NLPRule(vector<string> s) :
	RuleGenerator(s)
{
}

vector<t_rule> NLPRule::gen_rules()
{
	RuleGenerator *wildLCS = new FastLCS(cells);
	vector<t_rule> initial_rule_set = wildLCS->gen_rules();
	delete wildLCS;

	vector<t_rule> ans;
	for (t_rule rule : initial_rule_set)
	{
		string lhs = "", rhs = "";
		for (string t : rule.first)
			lhs += t;
		for (string t : rule.second)
			rhs += t;
		int len_l = (int) lhs.size();

		//|A| + 5, |A| * 2
		if (rule.second.size() > min(len_l + 5, len_l * 2))
			continue;

		//stop words
		if (stop_words.count(rule.first[0]))
			continue;

		//first character match
		if (rule.first[0][0] != rule.second[0][0])
			continue;

		//remove rules with digits
		bool has_digit = false;
		for (int i = 0; i < (int) rule.first[0].size(); i ++)
			if (isdigit(rule.first[0][i]))
				has_digit = true;
		for (int i = 0; i < (int) rule.second[0].size(); i ++)
			if (isdigit(rule.second[0][i]))
				has_digit = true;
		if (has_digit)
			continue;

		//remove rules with lhs size greater than 1
		if (rule.first.size() > 1)
			continue;

		ans.push_back(rule);
	}
	return ans;
}
