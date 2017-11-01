//
// Created by Wenbo Tao on 2/11/17.
//
#include "../PolynomialJoiner.h"
using namespace std;

unordered_set<string> PolynomialJoiner::buildExpansionSigs(vector<string> tokens, vector<t_rule> rules)
{
	int LIM = Common::SIG_LEN_LIM;

	//expansion set
	unordered_set<string> exp_set;
	for (string t : tokens)
		exp_set.insert(t);
	for (t_rule rule : rules)
		for (string t : rule.second)
			exp_set.insert(t);

	//PTIME signature generation
	unordered_set<string> signatures;
	for (string t : exp_set)
	{
		//whether this token is in the original string
		int original = 0;
		for (string o : tokens)
			if (t == o)
				original = 1;

		//dp
		vector<vector<vector<int>>> opt(rules.size() + 1);
		for (auto i = 0; i < opt.size(); i ++)
		{
			opt[i].clear();
			opt[i].resize(LIM + 1);
			for (auto j = 0; j < opt[i].size(); j ++)
			{
				opt[i][j].push_back(LIM * LIM);
				opt[i][j].push_back(LIM * LIM);
			}
		}
		opt[0][tokens.size()][original] = 0;
		for (string o : tokens)
			if (token_rankings[o] < token_rankings[t])
				opt[0][tokens.size()][original] ++;
		for (auto i = 0; i < rules.size(); i ++)
			for (auto j = 0; j <= LIM; j ++)
				for (auto c = 0; c < 2; c ++)
					if (opt[i][j][c] != LIM * LIM)
					{
						//not apply next rule
						opt[i + 1][j][c] = min(opt[i + 1][j][c], opt[i][j][c]);

						//apply next rule
						auto len = rules[i].second.size();
						if (j + len <= LIM)
						{
							int wt = 0;
							int contain = 0;
							for (string o : rules[i].second)
								if (token_rankings[o] < token_rankings[t])
									wt ++;
								else if (o == t)
									contain = 1;
							int &r = opt[i + 1][j + len][c | contain];
							r = min(r, opt[i][j][c] + wt);
						}
					}
		bool in_prefix = false;
		for (int len = 0; len <= LIM; len ++)
			if (opt[rules.size()][len][1] + 1 <= len - ceil(len * Common::JAC_THRESHOLD) + 1)
			{
				in_prefix = true;
				break;
			}
		if (in_prefix)
			signatures.insert(t);
	}
	return signatures;
}
