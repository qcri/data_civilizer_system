//
// Created by Wenbo Tao on 3/2/17.
//

#include "../PolynomialJoiner.h"
using namespace std;

unordered_set<string> PolynomialJoiner::buildDpSigsSlow(vector<string> tokens, vector<t_rule> applicable_rules)
{
	//limit on the number of tokens in the transformed string
	int LIM = (int) ((double) k / Common::JAC_THRESHOLD);

	//generate for each starting point, an inverted list of applicable rules
	vector<vector<t_rule>> rule_inv(tokens.size());
	for (t_rule rule : applicable_rules)
		for (int st = 0; st + (int) rule.first.size() - 1 < (int) tokens.size(); st ++)
		{
			bool ok = true;
			for (int j = 0; j < (int) rule.first.size(); j ++)
				if (rule.first[j] != tokens[st + j])
				{
					ok = false;
					break;
				}
			if (ok)
				rule_inv[st].push_back(rule);
		}
	unordered_set<string> all_tokens(tokens.begin(), tokens.end());
	for (t_rule rule : applicable_rules)
		for (string t : rule.second)
			all_tokens.insert(t);

	//enumerate all tokens in the full expansion set
	unordered_set<string> signatures;
	for (string t : all_tokens)
	{
		int cur_ranking = token_rankings[t];
		int tot_len = (int) tokens.size();

		//dp
		vector<vector<vector<int>>> opt(tot_len + 1, vector<vector<int>>(LIM + 1, vector<int>(2, LIM)));
		opt[0][0][0] = 0;
		for (int cur = 0; cur < tot_len; cur ++)
			for (int len = 0; len <= LIM; len ++)
				for (int e = 0; e < 2; e ++)
				{
					if (opt[cur][len][e] >= (int) LIM - ceil(LIM * Common::JAC_THRESHOLD) + 1)
						continue;

					int wt, new_e;
					//no transformation
					wt = (token_rankings[tokens[cur]] < cur_ranking ? 1 : 0);
					new_e = (tokens[cur] == t ? 1 : e);
					if (len + 1 <= LIM)
						if (opt[cur][len][e] + wt < opt[cur + 1][len + 1][new_e])
							opt[cur + 1][len + 1][new_e] = opt[cur][len][e] + wt;

					//using transformations starting at cur
					for (t_rule rule : rule_inv[cur])
					{
						wt = 0;
						new_e = e;
						for (string tt : rule.second)
							if (token_rankings[tt] < cur_ranking)
								wt ++;
							else if (tt == t)
								new_e = 1;
						int next_cur = cur + (int) rule.first.size();
						int next_len = len + (int) rule.second.size();
						if (next_len <= LIM)
							if (opt[cur][len][e] + wt < opt[next_cur][next_len][new_e])
								opt[next_cur][next_len][new_e] = opt[cur][len][e] + wt;
					}
				}

		//check
		bool in_prefix = false;
		for (int len = 1; len <= LIM; len ++)
			if (opt[tot_len][len][1] + 1 <= (int) len - ceil(len * Common::JAC_THRESHOLD) + 1)
			{
				in_prefix = true;
				break;
			}
		if (in_prefix)
			signatures.insert(t);
	}
	return signatures;
}
