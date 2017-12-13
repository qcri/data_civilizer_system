//
// Created by Wenbo Tao on 1/3/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

unordered_set<string> PolynomialJoiner::buildDpSigs(vector<string> tokens, vector<t_rule> applicable_rules)
{
	//limit on the number of tokens in the transformed string
	int LIM = (int) ((double) k / Common::JAC_THRESHOLD);

	//generate for each starting point, an inverted list of applicable rules
	vector<unordered_map<pair<int, int>, vector<int>, pairii_hash>> rule_inv(tokens.size());
	for (t_rule rule : applicable_rules)
		for (int st = 0; st + (int) rule.first.size() - 1 < (int) tokens.size(); st ++)
		{
			//test if this rule is applicable at this starting point
			bool ok = true;
			for (int i = 0; i < (int) rule.first.size(); i ++)
				if (rule.first[i] != tokens[st + i])
				{
					ok = false;
					break;
				}
			if (! ok)
				continue;

			for (int x = 0; x < rule.second.size(); x ++)
				for (int y = x + 1; y < rule.second.size(); y ++)
					if (token_rankings[rule.second[x]] > token_rankings[rule.second[y]])
						swap(rule.second[x], rule.second[y]);
			int lhs_size = (int) rule.first.size();
			int rhs_size = (int) rule.second.size();
			auto cp = make_pair(lhs_size, rhs_size);
			if (! rule_inv[st].count(cp))
				rule_inv[st][cp] = vector<int>(rhs_size, -1);

			for (int i = 0; i < rhs_size; i ++)
			{
				int cur = (int) token_rankings.size();
				if (i < (int) rule.second.size())
					cur = token_rankings[rule.second[i]];
				if (cur > rule_inv[st][cp][i])
					rule_inv[st][cp][i] = cur;
			}
		}

	//enumerate all tokens
	unordered_set<string> signatures;
	for (t_rule rule : applicable_rules)
		for (int st = 0; st + (int) rule.first.size() - 1 < (int) tokens.size(); st ++)
		{
			//test if this rule is applicable at this starting point
			bool ok = true;
			for (int i = 0; i < (int) rule.first.size(); i ++)
				if (rule.first[i] != tokens[st + i])
				{
					ok = false;
					break;
				}
			if (! ok)
				continue;

			//enumerate tokens in the right hand side
			for (string t : rule.second)
			{
				int cur_ranking = token_rankings[t];
				int tot_len = (int) tokens.size();
				int rhs_wt = 0;
				for (string ot : rule.second)
					if (token_rankings[ot] < cur_ranking)
						rhs_wt ++;

				//dp
				vector<vector<int>> opt(tot_len + 1, vector<int>(LIM + 1, LIM));
				opt[0][0] = 0;
				for (int cur = 0; cur < tot_len; cur ++)
					for (int len = 0; len <= LIM; len ++)
					{
						if (opt[cur][len] >= (int) LIM - ceil(LIM * Common::JAC_THRESHOLD) + 1)
							continue;

						//using transformations starting at cur
						for (auto it : rule_inv[cur])
						{
							int lhs_size = it.first.first;
							int rhs_size = it.first.second;
							int next_cur = cur + lhs_size;
							int next_len = len + rhs_size;

							//check if this rule application overlaps with the enumerated rule
							ok = false;
							if (next_cur <= st)
								ok = true;
							if (cur >= st + (int) rule.first.size())
								ok = true;
							if (cur == st && lhs_size == (int) rule.first.size()
									&& rhs_size == (int) rule.second.size())
								ok = true;
							if (! ok)
								continue;

							//calculate weight
							int wt;
							if (cur == st)
								wt = rhs_wt;
							else
								for (wt = 0; wt < rhs_size; wt ++)
									if (it.second[wt] >= cur_ranking)
										break;

							if (next_len <= LIM && opt[cur][len] + wt < opt[next_cur][next_len])
								opt[next_cur][next_len] = opt[cur][len] + wt;
						}
					}

				//check
				bool in_prefix = false;
				for (int len = 1; len <= LIM; len ++)
					if (opt[tot_len][len] + 1 <= len - ceil(len * Common::JAC_THRESHOLD) + 1)
					{
						in_prefix = true;
						break;
					}
				if (in_prefix)
					signatures.insert(t);
			}
		}

	return signatures;
}
