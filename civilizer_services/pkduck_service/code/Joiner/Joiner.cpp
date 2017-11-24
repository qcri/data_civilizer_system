//
// Created by Wenbo Tao on 12/27/16.
//

#include "Joiner.h"
using namespace std;

Joiner::Joiner(vector<t_rule> r, vector<string> s)
		: rules(r), cells(s)
{
	this->r = (int) r.size();
	this->n = (int) s.size();

	//build token sets and token maps
	tokens.clear();
	token_maps.clear();
	for (int i = 0; i < n; i ++)
	{
		tokens.push_back(Common::get_tokens(cells[i]));
		token_maps.push_back(umpsi());
		for (string s : tokens[i])
			token_maps[i][s] ++;
	}

	//build rule hash table
	rule_hash_table.clear();
	for (t_rule rule : rules)
		rule_hash_table.insert(rule);

	//generate applicable rules for each string
	gen_applicable_rules();

	//build matchable tokens
	matchable_tokens.clear();
	matchable_tokens.resize(n);
	for (int i = 0; i < n; i ++)
	{
		unordered_set<string> lhs_set;
		for (int rule_id : applicable_rule_ids[i])
		{
			t_rule rule = rules[rule_id];
			string lhs = "";
			for (string t : rule.first)
				lhs += t + " ";
			lhs.erase((int) lhs.size() - 1, 1);
			lhs_set.insert(lhs);
		}
		for (int st = 0; st < (int) tokens[i].size(); st ++)
		{
			string cur = "";
			for (int en = st; en < (int) tokens[i].size(); en ++)
			{
				if (en > st)
					cur += " ";
				cur += tokens[i][en];
				if (st == en || lhs_set.count(cur))
					matchable_tokens[i].emplace_back(st, en);
			}
		}
	}

	//generate expansion set
	gen_expansion_set();

	//generate global ranking list
	gen_global_ranking();

	//generate original signatures
	gen_original_signatures();
}

void Joiner::gen_applicable_rules()
{
	//build rule inverted lists
	int max_lhs_size = 0;
	unordered_map<string, vector<int>> rule_invl;
	for (int i = 0; i < (int) rules.size(); i ++)
	{
		string s = "";
		for (int j = 0; j < (int) rules[i].first.size(); j ++)
		{
			s += rules[i].first[j];
			if (j + 1 < (int) rules[i].first.size())
				s += " ";
		}
		rule_invl[s].push_back(i);
		max_lhs_size = max(max_lhs_size, (int) rules[i].first.size());
	}

	//generate applicable rule ids
	int sum_r = 0, max_r = 0;
	for (int i = 0; i < n; i ++)
	{
		unordered_set<int> app_set;
		for (int j = 0; j < (int) tokens[i].size(); j ++)
		{
			string cur_substr = "";
			for (int l = 1; l <= max_lhs_size; l ++)
			{
				if (j + l - 1 >= (int) tokens[i].size())
					break;
				if (l > 1)
					cur_substr += " ";
				cur_substr += tokens[i][j + l - 1];
				for (int rule_id : rule_invl[cur_substr])
					app_set.insert(rule_id);
			}
		}
		applicable_rule_ids.push_back(vector<int>(app_set.begin(), app_set.end()));
		max_r = max(max_r, (int) applicable_rule_ids.back().size());
		sum_r += (int) applicable_rule_ids.back().size();
	}

	cout << "MAX #applicable rule: " << max_r << endl;
	cout << "AVG #applicable rule: " << sum_r / (double) n << endl;
}

void Joiner::gen_expansion_set()
{
	expansion_set.clear();
	for (int i = 0; i < n; i ++)
	{
		unordered_set<string> cur_expansion_set;
		for (auto cp : token_maps[i])
			cur_expansion_set.insert(cp.first);
		for (int rule_id : applicable_rule_ids[i])
		{
			t_rule rule = rules[rule_id];
			for (string t : rule.second)
				cur_expansion_set.insert(t);
		}
		expansion_set.push_back(cur_expansion_set);
	}
}

void Joiner::gen_global_ranking()
{
	//make global token ranked list
	umpsi g_token_map;
	for (int i = 0; i < n; i ++)
		for (int st = 0; st < (int) tokens[i].size(); st ++)
			for (int en = st; en < (int) tokens[i].size(); en ++)
			{
				string t = "";
				for (int k = st; k <= en; k ++)
					t += tokens[i][k] + (k == en ? "" :  " ");
				g_token_map[t] ++;
			}

	vector<pair<int, string>> sort_array;
	for (auto cp : g_token_map)
		sort_array.emplace_back(cp.second, cp.first);
	sort(sort_array.begin(), sort_array.end());

	for (int i = 0; i < (int) sort_array.size(); i ++)
	{
		global_list.push_back(sort_array[i].second);
		token_rankings[sort_array[i].second] = i;
	}
}

void Joiner::gen_original_signatures()
{
	//build o_signatures
	o_sigs.clear();
	o_large_sigs.clear();
	for (int i = 0; i < n; i ++)
	{
		o_sigs.push_back(buildOriginalSigs(i));
		o_large_sigs.push_back(buildOriginalLargeTokenSigs(i));
	}
}

vector<t_rule> Joiner::get_applicable_rules(int x)
{
	vector<t_rule> app_rules;
	for (auto id : applicable_rule_ids[x])
		app_rules.push_back(rules[id]);
	return app_rules;
}

unordered_set<string> Joiner::get_o_sigs(int x)
{
	return o_sigs[x];
}