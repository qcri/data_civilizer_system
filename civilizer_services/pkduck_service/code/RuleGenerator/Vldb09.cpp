//
// Created by Wenbo Tao on 2/11/17.
//

#include "Vldb09.h"
#include "../Joiner/JaccardJoiner.h"
using namespace std;

Vldb09::Vldb09(vector<string> s)
	: RuleGenerator(s)
{
}

vector<t_rule> Vldb09::gen_rules()
{
	vector<t_rule> rules;

	//frequency count
	umpsi freq;
	for (auto i = 0; i < n; i ++)
		for (string t : tokens[i])
			freq[t] ++;

	//build example pairs
	JaccardJoiner *jacJoiner = new JaccardJoiner(vector<t_rule>(), cells, Common::VLDB09_JAC_THRESHOLD);
	vector<pair<double, pair<string, string>>> examples = jacJoiner->getJoinedStringPairs();
	cout << "example size: " << examples.size() << endl;
	delete jacJoiner;

	//algorithm
	unordered_map<t_rule, int, rule_hash> rule_freq;
	for (auto cp : examples)
	{
		vector<string> tks1 = Common::get_tokens(cp.second.first);
		vector<string> tks2 = Common::get_tokens(cp.second.second);
		vector<bool> u1(tks1.size()), u2(tks2.size());
		while (1)
		{
			bool find_identical_pair = false;
			for (auto i = 0; i < tks1.size(); i ++)
				if (! u1[i])
					for (auto j = 0; j < tks2.size(); j ++)
						if (! u2[j] && tks1[i] == tks2[j])
						{
							find_identical_pair = true;
							u1[i] = u2[j] = true;
						}
			if (! find_identical_pair)
				break;
		}
		for (auto i = 0; i < tks1.size(); i ++)
		{
			if (u1[i] || freq[tks1[i]] > Common::LHS_FREQ_THRESHOLD)
				continue;
			for (auto x = 0; x < tks2.size(); x ++)
				for (auto y = x; y < tks2.size(); y ++)
				{
					if (u2[y] || (x == y && freq[tks2[y]] > Common::LHS_FREQ_THRESHOLD))
						continue;
					vector<string> lhs, rhs;
					lhs.push_back(tks1[i]);
					for (auto j = x; j <= y; j ++)
						rhs.push_back(tks2[j]);

					rule_freq[make_pair(lhs, rhs)] ++;
				}
		}
		for (auto i = 0; i < tks2.size(); i ++)
		{
			if (u2[i] || freq[tks2[i]] > Common::LHS_FREQ_THRESHOLD)
				continue;
			for (auto x = 0; x < tks1.size(); x ++)
				for (auto y = x; y < tks1.size(); y ++)
				{
					if (u1[y] || (x == y && freq[tks1[y]] > Common::LHS_FREQ_THRESHOLD))
						continue;
					vector<string> lhs, rhs;
					lhs.push_back(tks2[i]);
					for (auto j = x; j <= y; j ++)
						rhs.push_back(tks1[j]);

					rule_freq[make_pair(lhs, rhs)] ++;
				}
		}
	}

	vector<pair<int, t_rule>> sort_array;
	for (auto cp : rule_freq)
		sort_array.push_back(make_pair(cp.second, cp.first));
	sort(sort_array.begin(), sort_array.end(), std::greater<pair<int, t_rule>>());

	cout << "--------" << endl;
	cout << sort_array.size() << endl;
	cout << "--------" << endl;

	for (auto i = 0; i < sort_array.size() && i < 10000; i ++)
		rules.push_back(sort_array[i].second);

	return rules;
}
