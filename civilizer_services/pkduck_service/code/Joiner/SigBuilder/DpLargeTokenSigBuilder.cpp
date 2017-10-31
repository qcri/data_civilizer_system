//
// Created by Wenbo Tao on 1/12/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

unordered_set<string> PolynomialJoiner::buildDpLargeTokenSigs(vector<string> tokens, vector<t_rule> applicable_rules)
{
	//generate for each starting point, an inverted list of applicable rules
	vector<unordered_map<int, int>> rule_inv(tokens.size());
	for (t_rule rule : applicable_rules)
		for (int st = 0; st + (int) rule.first.size() - 1 < (int) tokens.size(); st ++)
		{
			bool ok = true;
			for (int i = 0; i < (int) rule.first.size(); i ++)
				if (rule.first[i] != tokens[st + i])
				{
					ok = false;
					break;
				}
			if (! ok)
				continue;

			int lhs_size = (int) rule.first.size();
			string rhs = "";
			for (int i = 0; i < (int) rule.second.size(); i ++)
				rhs += rule.second[i] + (i + 1 == (int) rule.second.size() ? "" :  " ");

			if (! rule_inv[st].count(lhs_size))
				rule_inv[st][lhs_size] = -1;

			if (token_rankings[rhs] > rule_inv[st][lhs_size])
				rule_inv[st][lhs_size] = token_rankings[rhs];
		}

	//enumerate all tokens in the full expansion set
	unordered_set<string> signatures;
	for (t_rule rule : applicable_rules)
		for (int st = 0; st + (int) rule.first.size() - 1 < (int) tokens.size(); st ++)
		{
			bool ok = true;
			for (int i = 0; i < (int) rule.first.size(); i ++)
				if (rule.first[i] != tokens[st + i])
				{
					ok = false;
					break;
				}
			if (! ok)
				continue;

			string rhs = "";
			for (int i = 0; i < (int) rule.second.size(); i ++)
				rhs += rule.second[i] + (i + 1 == (int) rule.second.size() ? "" :  " ");
			int cur_ranking = token_rankings[rhs];
			int tot_len = (int) tokens.size();

			//dp
			vector<vector<int>> opt(tot_len + 1, vector<int>(tot_len + 1, tot_len));
			opt[0][0] = 0;
			for (int cur = 0; cur < tot_len; cur ++)
				for (int len = 0; len <= tot_len; len ++)
				{
					if (opt[cur][len] >= (int) tot_len - ceil(tot_len * Common::JAC_THRESHOLD) + 1)
						continue;

					int wt;
					//using transformations starting at cur
					for (auto it : rule_inv[cur])
					{
						int lhs_size = it.first;
						int next_cur = cur + lhs_size;
						int next_len = len + 1;

						//check if this rule application overlaps with the enumerated rule
						ok = false;
						if (next_cur <= st)
							ok = true;
						if (cur >= st + (int) rule.first.size())
							ok = true;
						if (cur == st && lhs_size == (int) rule.first.size())
							ok = true;
						if (! ok)
							continue;

						//calculate weight
						if (cur == st)
							wt = 0;
						else
							wt = (it.second >= cur_ranking ? 0 : 1);

						if (next_len <= tot_len && opt[cur][len] + wt < opt[next_cur][next_len])
							opt[next_cur][next_len] = opt[cur][len] + wt;
					}
				}

			//check
			bool in_prefix = false;
			for (int len = 1; len <= tot_len; len ++)
				if (opt[tot_len][len] + 1 <= len - ceil(len * Common::JAC_THRESHOLD) + 1)
				{
					in_prefix = true;
					break;
				}
			if (in_prefix)
				signatures.insert(rhs);
		}
	return signatures;
}
