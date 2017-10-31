//
// Created by Wenbo Tao on 2/11/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

double PolynomialJoiner::icde08_get_similarity(int x, int y)
{
	//calculate st point of applicable rules
	umpsi x_map = token_maps[x];
	umpsi y_map = token_maps[y];

	//some boolean arrays
	vector<bool> t_usedx(tokens[x].size(), false), t_usedy(tokens[y].size(), false);
	vector<bool> r_usedx(app_rule_w_st[x].size(), false), r_usedy(app_rule_w_st[y].size(), false);

	//repeatedly select rules
	while (1)
	{
		int rule_idx = -1, rule_idy = -1;
		double max_inc_sim = 0;
		double cur_sim = Common::jaccard(x_map, y_map);
		for (auto i = 0; i <= app_rule_w_st[x].size(); i ++)
			for (auto j = 0; j <= app_rule_w_st[y].size(); j ++)
			{
				if (i < app_rule_w_st[x].size() && r_usedx[i])
					continue;
				if (j < app_rule_w_st[y].size() && r_usedy[j])
					continue;

				//check if these two rules are both applicable
				if (i < app_rule_w_st[x].size())
				{
					bool all_empty = true;
					auto cp = app_rule_w_st[x][i];
					for (auto k = 0; k < cp.first.first.size(); k ++)
						if (t_usedx[k + cp.second])
						{
							all_empty = false;
							break;
						}
					if (! all_empty)
						continue;
				}
				if (j < app_rule_w_st[y].size())
				{
					bool all_empty = true;
					auto cp = app_rule_w_st[y][j];
					for (auto k = 0; k < cp.first.first.size(); k ++)
						if (t_usedy[k + cp.second])
						{
							all_empty = false;
							break;
						}
					if (! all_empty)
						continue;
				}

				//build new map
				umpsi new_x_map = x_map;
				if (i < app_rule_w_st[x].size())
					for (string t : app_rule_w_st[x][i].first.first)
						new_x_map[t] --;
				if (i < app_rule_w_st[x].size())
					for (string t : app_rule_w_st[x][i].first.second)
						new_x_map[t] ++;

				umpsi new_y_map = y_map;
				if (j < app_rule_w_st[y].size())
					for (string t : app_rule_w_st[y][j].first.first)
						new_y_map[t] --;
				if (j < app_rule_w_st[y].size())
					for (string t : app_rule_w_st[y][j].first.second)
						new_y_map[t] ++;

				//calculate gain
				double cur_inc_sim = Common::jaccard(new_x_map, new_y_map) - cur_sim;
				if (cur_inc_sim > max_inc_sim)
				{
					max_inc_sim = cur_inc_sim;
					rule_idx = i;
					rule_idy = j;
				}
			}

		if (max_inc_sim <= 0)
			break;

		if (rule_idx < app_rule_w_st[x].size())
		{
			r_usedx[rule_idx] = true;
			auto cp = app_rule_w_st[x][rule_idx];
			for (auto j = 0; j < cp.first.first.size(); j ++)
				t_usedx[j + cp.second] = true;
			for (string t : cp.first.first)
				x_map[t] --;
			for (string t : cp.first.second)
				x_map[t] ++;
		}
		if (rule_idy < app_rule_w_st[y].size())
		{
			r_usedy[rule_idy] = true;
			auto cp = app_rule_w_st[y][rule_idy];
			for (auto j = 0; j < cp.first.first.size(); j ++)
				t_usedy[j + cp.second] = true;
			for (string t : cp.first.first)
				y_map[t] --;
			for (string t : cp.first.second)
				y_map[t] ++;
		}
	}

	return Common::jaccard(x_map, y_map);
}
