//
// Created by Wenbo Tao on 1/6/17.
//

#include "MyRule.h"
#include "FastLCS.h"

using namespace std;

MyRule::MyRule(vector<string> s)
	: RuleGenerator(s)
{
}

vector<t_rule> MyRule::gen_rules()
{
	RuleGenerator *fastLCS = new FastLCS(cells);
	vector<t_rule> initial_rule_set = fastLCS->gen_rules();
	cout << "LCS rule set size : " << initial_rule_set.size() << endl;
	delete fastLCS;

	vector<pair<double, t_rule>> sort_array;
	for (t_rule rule : initial_rule_set)
	{
		//remove rules like science ==> science center
		if (rule.first.size() == 1 && rule.second.size() > 1)
			if (rule.second[0].find(rule.first[0]) == 0 ||
					rule.first[0].find(rule.second[0]) == 0)
				if (fabs((int) rule.first[0].size() - (int) rule.second[0].size()) <= 1)
					continue;

		//remove rules with lhs size greater than 1
		if (rule.first.size() > 1)
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

		//string representation of left & right hand side
		string lhs = "", rhs = "";
		for (string t : rule.first)
			lhs += t;
		for (string t : rule.second)
			rhs += t;
		int len_l = (int) lhs.size();

		//directly approve initialism
		if (rule.second.size() == 1)
			if (rhs.find(lhs) == 0)
			{
				sort_array.emplace_back(0, rule);
				continue;
			}

		//specially treat acronym
		if (rule.second.size() > 1)
		{
			int n_rhs_token = (int) rule.second.size();
			vector<vector<bool>> ok(len_l + 1, vector<bool>(n_rhs_token + 1, false));
			ok[0][0] = true;
			for (int i = 0; i <= len_l; i ++)
				for (int j = 1; j <= n_rhs_token; j ++)
				{
					if (i)
						if (lhs[i - 1] == rule.second[j - 1][0])
							if (ok[i - 1][j - 1])
								ok[i][j] = true;

					if (stop_words.count(rule.second[j - 1]))
						if (ok[i][j - 1])
							ok[i][j] = true;
				}

			if (ok[len_l][n_rhs_token])
				sort_array.emplace_back(0, rule);

			continue;
		}

		//greedily find a matching
		vector<int> m_inds;
		for (int i = 0; i < len_l; i ++)
		{
			int st = (m_inds.size() ? m_inds.back() + 1 : 0);
			while (st < rhs.size() && rhs[st] != lhs[i])
				st ++;
			if (st < rhs.size())
				m_inds.push_back(st);
		}

		//check vowels
		bool ok = true;
		for (int i = 1; i < (int) m_inds.size(); i ++)
			if (vowels.count(rhs[m_inds[i]]))
				if (m_inds[i - 1] + 1 != m_inds[i])
					ok = false;
		if (! ok)
			continue;

		//number of consonants
		int lhs_n_cons = 0, rhs_n_cons = 0;
		for (auto i = 0; i < m_inds.size(); i ++)
			if (! vowels.count(rhs[m_inds[i]]))
				lhs_n_cons ++;

		for (auto i = 0; i < rhs.size(); i ++)
			if (! vowels.count(rhs[i]))
				rhs_n_cons ++;

		if (rhs_n_cons == 0 || lhs_n_cons / (double) rhs_n_cons >= 0.6)
			sort_array.emplace_back(0, rule);
	}

	vector<t_rule> rules;
	for (auto cp : sort_array)
		rules.push_back(cp.second);

	return rules;
}
