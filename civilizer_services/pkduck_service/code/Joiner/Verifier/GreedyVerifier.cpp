//
// Created by Wenbo Tao on 2/6/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

double PolynomialJoiner::greedy_get_similarity(int x, int y)
{
	double sim = max(greedy_directed_get_similarity(x, y),
				greedy_directed_get_similarity(y, x));
	if (cells[x] == "electrical eng & computer sci" && cells[y] == "dept of electrical engineering & computer science")
		cout << "fuckyou!! " << sim << endl;
	return sim;
}

double PolynomialJoiner::greedy_directed_get_similarity(int x, int y)
{
	vector<pair<t_rule, int>> good_rules;
	//identity
	for (auto i = 0; i < tokens[x].size(); i ++)
		if (token_maps[y].count(tokens[x][i]))
		{
			vector<string> lhs = vector<string>(1, tokens[x][i]);
			vector<string> rhs = lhs;
			good_rules.emplace_back(make_pair(lhs, rhs), i);
		}

	//use inv list to build a candidate rule list
	unordered_set<int> good_rule_ids;
	for (string t : tokens[y])
		if (a_rule_inv_list[x].count(t))
			for (int rule_id : a_rule_inv_list[x][t])
				good_rule_ids.insert(rule_id);

	//transformation
	for (int rule_id : good_rule_ids)
	{
		t_rule rule = rules[rule_id];
		for (auto i = 0; i < tokens[x].size(); i ++)
		{
			if (i + rule.first.size() > tokens[x].size())
				break;
			bool good = true;
			for (auto j = 0; j < rule.first.size(); j ++)
				if (tokens[x][i + j] != rule.first[j])
				{
					good = false;
					break;
				}
			if (! good)
				continue;
			good_rules.emplace_back(rule, i);
		}
	}

	//greedy algorithm
	vector<bool> rule_used(good_rules.size(), false);
	vector<bool> x_used(tokens[x].size(), false);
	vector<bool> y_used(tokens[y].size(), false);

	int n_uncovered = 0, n_covered = 0;
	umpsi x_map = token_maps[x];
	double max_sim = Common::jaccard(x_map, token_maps[y]);
	while (true)
	{
		double max_rule_gain = -1;
		int best_n_good = -1;
		int best_rule_id = -1;
		for (auto i = 0; i < good_rules.size(); i ++)
			if (! rule_used[i])
			{
				t_rule rule = good_rules[i].first;
				auto st = good_rules[i].second;
				bool good = true;
				for (auto j = 0; j < rule.first.size(); j ++)
					if (x_used[st + j])
					{
						good = false;
						break;
					}
				if (! good)
					continue;

				//calculate rule gain
				double n_good = 0, n_bad = 0;
				vector<bool> y_used_copy = y_used;
				for (string t : rule.second)
				{
					bool covered = false;
					for (auto j = 0; j < tokens[y].size(); j ++)
						if (! y_used_copy[j] && tokens[y][j] == t)
						{
							y_used_copy[j] = true;
							covered = true;
							break;
						}
					if (covered)
						n_good ++;
					else
						n_bad ++;
				}
				double rule_gain = n_good / (n_good + n_bad);
				if (rule_gain > max_rule_gain ||
						(rule_gain == max_rule_gain && n_good > best_n_good))
					max_rule_gain = rule_gain, best_n_good = n_good, best_rule_id = i;
			}

		//no applicable rules
		if (best_rule_id == -1)
			break;

		rule_used[best_rule_id] = true;
		t_rule rule = good_rules[best_rule_id].first;
		auto st = good_rules[best_rule_id].second;
		for (auto j = 0; j < rule.first.size(); j ++)
			x_used[st + j] = true;
		for (string t : rule.second)
		{
			bool covered = false;
			for (auto j = 0; j < tokens[y].size(); j++)
				if (! y_used[j] && tokens[y][j] == t)
				{
					y_used[j] = true;
					covered = true;
					break;
				}
			if (covered)
				n_covered ++;
			else
				n_uncovered ++;
		}

		//update x_map and max_sim
		for (auto j = 0; j < rule.first.size(); j ++)
			x_map[tokens[x][st + j]] --;
		for (string t : rule.second)
			x_map[t] ++;
		max_sim = max(max_sim, Common::jaccard(x_map, token_maps[y]));
	}

	return max_sim;
}
