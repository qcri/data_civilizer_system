//
// Created by Wenbo Tao on 12/27/16.
//

#include "FastLCS.h"

FastLCS::FastLCS(vector<string> s)
		: RuleGenerator(s)
{
	//build trie
	trie.clear(), trie.resize(1);
	contain_word.clear(), contain_word.push_back(unordered_set<string>());

	for (int i = 0; i < n; i ++)
		for (string s : tokens[i])
		{
			if (s.size() < Common::LHS_SIZE_LIM)
				continue;

			//insert s into trie
			int cur_node = 0;
			for (int j = 0; j < (int) s.size(); j ++)
			{
				if (! trie[cur_node].count(s[j]))
				{
					trie[cur_node][s[j]] = (int) trie.size();
					trie.push_back(unordered_map<char, int>());
					contain_word.push_back(unordered_set<string>());
				}
				cur_node = trie[cur_node][s[j]];
			}
			contain_word[cur_node].insert(s);

			if (! Common::ENABLE_DELTA)
				continue;

			for (auto d = 0; d < s.size(); d ++)
			{
				string t = s; t.erase(d, 1);
				if (t.size() < Common::DELTA_ABBR_LEN)
					break;

				cur_node = 0;
				for (auto j = 0; j < t.size(); j ++)
				{
					if (! trie[cur_node].count(t[j]))
					{
						trie[cur_node][t[j]] = (int) trie.size();
						trie.push_back(unordered_map<char, int>());
						contain_word.push_back(unordered_set<string>());
					}
					cur_node = trie[cur_node][t[j]];
				}
				//if (contain_word[cur_node].empty())
				contain_word[cur_node].insert(s);
			}
		}

	//calculate D
	unordered_map<char, int> b_factor;
	for (auto mp : trie)
		for (auto cp : mp)
			b_factor[cp.first] ++;

	double sum_b = 0;
	for (auto cp : b_factor)
		sum_b += cp.second;

	double max_b = 0;
	for (auto cp : b_factor)
		max_b = max(max_b, (double) cp.second);
}

vector<t_rule> FastLCS::gen_rules()
{
	unordered_set<t_rule, rule_hash> rules;
	for (int i = 0; i < n; i ++)
		for (int st = 0; st < (int) tokens[i].size(); st ++)
		{
			string cur_string = "";
			vector<int> start_token;
			for (int j = st; j < (int) tokens[i].size(); j ++)
				for (int k = 0; k < (int) tokens[i][j].size(); k ++)
				{
					cur_string += tokens[i][j][k];
					if (k == 0)
						start_token.push_back(j);
					else
						start_token.push_back(-1);
				}
			//dp
			unordered_set<int> nodes[2];
			int cur_iter = 0;
			nodes[0].clear(), nodes[0].insert(0);

			for (int j = 0; j < cur_string.size(); j ++)
			{
				//current iteration
				cur_iter = 1 - cur_iter;
				nodes[cur_iter].clear();

				for (int node : nodes[cur_iter ^ 1])
				{
					//not include current character
					if (start_token[j] == -1 || stop_words.count(tokens[i][start_token[j]]))
						nodes[cur_iter].insert(node);

					//include current character
					if (trie[node].count(cur_string[j]))
						nodes[cur_iter].insert(trie[node][cur_string[j]]);
				}

				//make rules
				if (j + 1 == (int) cur_string.size() || start_token[j + 1] != -1)
				{
					//make rhs
					vector<string> rhs;
					int en = (int) tokens[i].size() - 1;
					if (j + 1 < (int) cur_string.size())
						en = start_token[j + 1] - 1;
					for (int k = st; k <= en; k ++)
						rhs.push_back(tokens[i][k]);
					if (st < en && (stop_words.count(tokens[i][st]) || stop_words.count(tokens[i][en])))
						continue;

					//for every node in the previous iteration
					for (int node : nodes[cur_iter])
						for (string abbr : contain_word[node])
						{
							vector<string> lhs;
							lhs.push_back(abbr);
							if (stop_words.count(lhs[0]))
								continue;

							//get rid of identity rules
							if (rhs.size() == 1 && lhs[0] == rhs[0])
								continue;
							rules.insert(make_pair(lhs, rhs));
						}
				}
			}
		}

	return vector<t_rule>(rules.begin(), rules.end());
}
