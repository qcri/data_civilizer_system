//
// Created by Wenbo Tao on 12/30/16.
//

#include "PolynomialJoiner.h"

PolynomialJoiner::PolynomialJoiner(vector<t_rule> r, vector<string> s)
	: Joiner(r, s)
{
	//calculate k
	k = 0;
	for (int i = 0; i < n; i ++)
		k = max(k, (int) tokens[i].size());

	//build t_signatures
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	t_sigs.clear();
	e_sigs.clear();
	for (int i = 0; i < n; i ++)
	{
		//construct a different set of applicable rules
		vector<t_rule> applicable_rules;
		for (int rule_id : applicable_rule_ids[i])
			applicable_rules.push_back(rules[rule_id]);

		for (string t : tokens[i])
			applicable_rules.emplace_back(vector<string>(1, t), vector<string>(1, t));
		if (Common::MEASURE == 0)
			t_sigs.push_back(Common::FAST_SIG ? buildDpSigs(tokens[i], applicable_rules) : buildDpSigsSlow(tokens[i], applicable_rules));
		else if (Common::MEASURE == 1)
			e_sigs.push_back(buildExpansionSigs(tokens[i], applicable_rules));
		else if (Common::MEASURE == 2)
			t_sigs.push_back(buildJacctSigs(tokens[i], applicable_rules));
	}
	gettimeofday(&t2, NULL);
	double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
	cout << endl << "Signature generation took : " << elapsedTime << "s." << endl << endl;

	//build a_rule_inv_list
	a_rule_inv_list.clear();
	for (int i = 0; i < n; i ++)
	{
		unordered_map<string, unordered_set<int>> inv_list;
		inv_list.clear();
		for (int rule_id : applicable_rule_ids[i])
		{
			t_rule rule = rules[rule_id];
			for (string t : rule.second)
			{
				if (! inv_list.count(t))
					inv_list[t].clear();
				inv_list[t].insert(rule_id);
			}
		}
		a_rule_inv_list.push_back(inv_list);
	}

	//build app_rule_w_st
	app_rule_w_st.clear();
	for (int i = 0; i < n; i ++)
	{
		vector<pair<t_rule, int>> rule_w_st;
		for (int rule_id : applicable_rule_ids[i])
		{
			t_rule rule = rules[rule_id];
			for (auto st = 0; st < tokens[i].size(); st ++)
			{
				if (st + rule.first.size() > tokens[i].size())
					break;
				bool good = true;
				for (auto j = 0; j < rule.first.size(); j ++)
					if (tokens[i][st + j] != rule.first[j])
					{
						good = false;
						break;
					}
				if (! good)
					continue;
				rule_w_st.emplace_back(rule, st);
			}
		}
		app_rule_w_st.push_back(rule_w_st);
	}

	cerr << "Signature built." << endl;
}

vector<pair<double, pair<string, string>>> PolynomialJoiner::getJoinedStringPairs()
{
	vector<pair<double, pair<string, string>>> ans;
	unordered_set<pair<int, int>, pairii_hash> candidates;

	//build inverted lists
	unordered_map<string, vector<int>> inv_list;
	for (int i = 0; i < n; i ++)
	{
		unordered_set<string> &sig_set (Common::MEASURE == 0 ? t_sigs[i] : (Common::MEASURE == 1 ? e_sigs[i] : t_sigs[i]));
		for (string t : sig_set)
			inv_list[t].push_back(i);
	}

	//generate candidates
	for (int i = 0; i < n; i ++)
	{
		unordered_set<int> cur_set;
		unordered_set<string> &sig_set = (Common::MEASURE == 0 ? o_sigs[i] : (Common::MEASURE == 1 ? e_sigs[i] : t_sigs[i]));
		for (string t : sig_set)
			for (int v : inv_list[t])
				if (v != i)
					cur_set.insert(v);
		for (int v : cur_set)
		{
			pair<int, int> cur_cp = make_pair(min(i, v), max(i, v));
			candidates.insert(cur_cp);
		}
	}
	cout << "Number of candidate string pairs : " << candidates.size() << endl;

	if (! Common::DO_JOIN)
		return ans;

	//calculate similarity for candidate pairs
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	for (auto cp : candidates)
	{
		double sim;
		if (Common::MEASURE == 0)
			sim = greedy_get_similarity(cp.first, cp.second);
		else
			sim = sigmod13_get_similarity(cp.first, cp.second);
		//double sim = icde08_get_similarity(cp.first, cp.second);
		//double sim = large_token_get_similarity(cp.first, cp.second);
		if (sim >= Common::JAC_THRESHOLD)
			ans.emplace_back(sim, make_pair(cells[cp.first], cells[cp.second]));
	}
	gettimeofday(&t2, NULL);
	double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
	cout << endl << "Candidate verification took : " << elapsedTime << "s." << endl << endl;

	return ans;
}

unordered_set<string> PolynomialJoiner::get_t_sigs(int x)
{
	return t_sigs[x];
}
