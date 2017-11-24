//
// Created by Wenbo Tao on 12/26/16.
//

#include "Common.h"
using namespace std;

double Common::JAC_THRESHOLD = 0.7;
double Common::VLDB09_JAC_THRESHOLD = 0.6;
bool Common::ENABLE_DELTA = false;
bool Common::DO_JOIN = true;
bool Common::FAST_SIG = true;
int Common::DICTIONARY = 0;
int Common::MEASURE = 0;
int Common::DELTA_ABBR_LEN = 1;
int Common::LHS_SIZE_LIM = 1;
int Common::BAD_THRESHOLD = 10;
int Common::LHS_FREQ_THRESHOLD = 100;
int Common::APPLICABLE_THRESHOLD = 15;
int Common::SIG_LEN_LIM = 30;

vector<string> Common::get_tokens(string s)
{
	s += ' ';
	vector<string> ans;
	string ss = "";
	for (int i = 0; i < (int) s.size(); i ++)
		if (! isalpha(s[i]) && ! isdigit(s[i]))
		{
			if (ss.size())
				ans.push_back(ss);
			ss = "";
		}
		else
			ss += s[i];
	return ans;
}

bool Common::is_subseq_greedy(string lhs, string rhs)
{
	int last_pos = -1;
	for (int j = 0; j < (int) lhs.size(); j ++)
		for (last_pos ++; last_pos < (int) rhs.size(); last_pos ++)
			if (rhs[last_pos] == lhs[j])
				break;

	return last_pos < (int) rhs.size();
}

bool Common::is_subseq_dp(string lhs, string rhs)
{
	vector<string> lhs_token_set = Common::get_tokens(lhs);
	vector<string> rhs_token_set = Common::get_tokens(rhs);

	vector<int> start_token;
	string lhs_concat = "";
	for (string s : lhs_token_set)
	{
		lhs_concat += s;
		start_token.push_back((int) s.size());
		for (int i = 0; i < (int) s.size() - 1; i ++)
			start_token.push_back(0);
	}

	int n = (int) lhs_concat.size();
	int m = (int) rhs_token_set.size();
	vector<vector<bool>> dp(n + 1, vector<bool>(m + 1, false));

	dp[0][0] = true;
	for (int i = 1; i <= n; i ++)
		for (int j = 1; j <= m; j ++)
		{
			for (int k = 0; k < i; k ++)
				if (dp[k][j - 1])
				{
					string cur_substr = lhs_concat.substr(k, i - k);
					if (cur_substr[0] != rhs_token_set[j - 1][0])
						continue;
					if (is_subseq_greedy(cur_substr.substr(1), rhs_token_set[j - 1].substr(1)))
					{
						dp[i][j] = true;
						break;
					}
				}
		}

	return dp[n][m] && lhs != rhs;
}

void Common::print_rule(t_rule rule)
{
	for (string s : rule.first)
		cout << s << " ";
	cout << "==> ";
	for (string s : rule.second)
		cout << s << " ";
	cout << endl;
}

double Common::jaccard(umpsi a, umpsi b)
{
	double itsct = 0, unn = 0;
	for (auto cp : a)
		if (b.count(cp.first))
		{
			itsct += min(a[cp.first], b[cp.first]);
			unn += max(a[cp.first], b[cp.first]);
		}
		else
			unn += cp.second;
	for (auto cp : b)
		if (! a.count(cp.first))
			unn += cp.second;

	return itsct / unn;
}

void Common::set_default()
{
	JAC_THRESHOLD = 0.8;
	VLDB09_JAC_THRESHOLD = 0.6;
	ENABLE_DELTA = false;
	DO_JOIN = true;
	FAST_SIG = true;
	DICTIONARY = 0;
	MEASURE = 0;
	DELTA_ABBR_LEN = 1;
	LHS_SIZE_LIM = 1;
	BAD_THRESHOLD = 10;
	LHS_FREQ_THRESHOLD = 100;
	APPLICABLE_THRESHOLD = 15;
	SIG_LEN_LIM = 30;
}
