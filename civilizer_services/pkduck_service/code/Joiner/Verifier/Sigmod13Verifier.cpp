//
// Created by Wenbo Tao on 1/14/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

double PolynomialJoiner::sigmod13_get_similarity(int x, int y)
{
	//findCandidateRuleSet
	vector<pair<t_rule, double>> cx, cy;
	for (int rule_id : applicable_rule_ids[x])
	{
		double rg = rule_gain(rules[rule_id], token_maps[x], y);
		if (rg > 0)
			cx.emplace_back(rules[rule_id], rg);
	}
	for (int rule_id : applicable_rule_ids[y])
	{
		double rg = rule_gain(rules[rule_id], token_maps[y], x);
		if (rg > 0)
			cy.emplace_back(rules[rule_id], rg);
	}

	while (1)
	{
		umpsi sx = token_maps[x];
		umpsi sy = token_maps[y];
		for (auto cp : cx)
			for (string t : cp.first.second)
				sx[t] ++;
		for (auto cp : cy)
			for (string t : cp.first.second)
				sy[t] ++;

		//calculate jaccard
		double theta = Common::jaccard(sx, sy);

		//remove rules
		vector<pair<t_rule, double>> cx_old = cx, cy_old = cy;
		cx.clear(), cy.clear();
		bool removed = false;
		for (auto cp : cx_old)
			if (cp.second < theta / (theta + 1))
				removed = true;
			else
				cx.push_back(cp);
		for (auto cp : cy_old)
			if (cp.second < theta / (theta + 1))
				removed = true;
			else
				cy.push_back(cp);

		if (! removed)
			break;
	}

	//expand
	umpsi sx = token_maps[x], sy = token_maps[y];
	unordered_set<t_rule, rule_hash> dx, dy;
	for (auto cp : cx)
		dx.insert(cp.first);
	for (auto cp : cy)
		dy.insert(cp.first);

	while (dx.size() || dy.size())
	{
		double max_rg = 0;
		t_rule best_rule;
		int i = -1;

		for (t_rule rule : dx)
		{
			double rg = rule_gain(rule, sx, y);
			if (rg > max_rg)
				max_rg = rg, best_rule = rule, i = 1;
		}
		for (t_rule rule : dy)
		{
			double rg = rule_gain(rule, sy, x);
			if (rg > max_rg)
				max_rg = rg, best_rule = rule, i = 2;
		}

		//no rules with positive rule gain
		if (max_rg <= 0)
			break;

		//expand s[i]
		for (string t : best_rule.second)
			if (i == 1)
				sx[t] ++;
			else
				sy[t] ++;

		//remove best rule
		if (i == 1)
			dx.erase(best_rule);
		else
			dy.erase(best_rule);
	}

	return max(Common::jaccard(sx, sy), Common::jaccard(token_maps[x], token_maps[y]));
}

double PolynomialJoiner::rule_gain(t_rule rule, umpsi token_map, int y)
{
	//must make sure rule is an applicable rule of cell[x]
	double U = 0, G = 0;

	//calculate U
	for (string t : rule.second)
		if (! token_map.count(t))
			U ++;
	if (U == 0)
		return 0;

	//calculate G
	for (string t : rule.second)
		if (expansion_set[y].count(t) && ! token_map.count(t))
			G ++;

	return G / U;
}
