//
// Created by Wenbo Tao on 1/17/17.
//

#include "../Joiner.h"
using namespace std;

unordered_set<string> Joiner::buildOriginalLargeTokenSigs(int x)
{
	int tot_len = (int) tokens[x].size();

	//make is_large_token
	vector<vector<bool>> is_large_token(tot_len, vector<bool>(tot_len, false));
	for (auto cp : matchable_tokens[x])
		is_large_token[cp.first][cp.second] = true;

	//dp
	unordered_set<string> signatures;
	for (auto cp : matchable_tokens[x])
	{
		int st = cp.first + 1;
		int en = cp.second + 1;

		//make string
		string cur_string = "";
		for (int i = st; i <= en; i ++)
			cur_string += (i == st ? "" : " ") + tokens[x][i - 1];
		int cur_ranking = token_rankings[cur_string];

		//dp
		vector<vector<int>> opt(tot_len + 1, vector<int>(tot_len + 1, tot_len + 1));
		opt[0][0] = 0;
		for (int i = 1; i <= tot_len; i ++)
			for (int j = 1; j <= i; j ++)
				for (int k = 1; k <= i; k ++)
				{
					//check if this transfer is ok
					if (! is_large_token[k - 1][i - 1])
						continue;
					bool ok = false;
					if (i < st)
						ok = true;
					if (k > en)
						ok = true;
					if (k == st && i == en)
						ok = true;
					if (! ok)
						continue;

					//calculate weight
					string cur_token = "";
					for (int p = k; p <= i; p ++)
						cur_token += (p == k ? "" : " ") + tokens[x][p - 1];
					int wt = (token_rankings[cur_token] < cur_ranking ? 1 : 0);

					//update
					if (opt[k - 1][j - 1] + wt < opt[i][j])
						opt[i][j] = opt[k - 1][j - 1] + wt;
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
			signatures.insert(cur_string);
	}
	return signatures;
}
