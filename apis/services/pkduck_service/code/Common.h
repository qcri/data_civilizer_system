//
// Created by Wenbo Tao on 12/26/16.
//

#ifndef XCLEAN_COMMON_H
#define XCLEAN_COMMON_H
#include <map>
#include <set>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>
using namespace std;
typedef unordered_map<string, int> umpsi;
typedef pair<vector<string>, vector<string>> t_rule;


struct vectorstr_hash
{
	std::size_t operator () (const std::vector<string> &p) const
	{
		long long hash_value = 1;
		for (auto i = 0; i < p.size(); i ++)
			for (auto j = 0; j < p[i].size(); j ++)
				hash_value = (long long) hash_value * (long long) p[i][j] % 99999997LL;

		return (size_t) hash_value;
	}
};

struct pairii_hash
{
	std::size_t operator () (const std::pair<int, int> &p) const
	{
		return (size_t) ((long long) p.first * (long long) p.second % 9999997LL);
	}
};


//hash function for rules
struct rule_hash
{
	std::size_t operator () (const std::pair<vector<string>, vector<string>> &p) const
	{
		size_t a = 1;
		for (string s : p.first)
			a = (size_t) ((long long) a * (long long) std::hash<string>{}(s) % 9999997LL);
		for (string s : p.second)
			a = (size_t) ((long long) a * (long long) std::hash<string>{}(s) % 9999997LL);

		return a;
	}
};

class Common
{
public:

	//split a string into tokens
	static vector<string> get_tokens(string);

	//check whether lhs is a subsequece of rhs
	static bool is_subseq_greedy(string lhs, string rhs);

	//use dp to check whether lhs is a constrained-subsequence of rhs
	static bool is_subseq_dp(string lhs, string rhs);

	//print a rule
	static void print_rule(t_rule rule);

	//the jaccard similarity between to unordered maps
	static double jaccard(umpsi a, umpsi b);

	//set parameters to default values
	static void set_default();

	//parameters
	static double JAC_THRESHOLD;
	static double VLDB09_JAC_THRESHOLD;
	static bool ENABLE_DELTA;
	static bool DO_JOIN;
	static int DICTIONARY;		//0 -- lcs, 1 -- vldb09, 2 -- handcrafted, 3 -- general purpose
	static int MEASURE;			//0 -- sim, 1 -- sigmod13, 2 -- jaccard
	static bool FAST_SIG;

	static int DELTA_ABBR_LEN;
	static int LHS_SIZE_LIM;
	static int BAD_THRESHOLD;
	static int LHS_FREQ_THRESHOLD;
	static int APPLICABLE_THRESHOLD;
	static int SIG_LEN_LIM;
};


#endif //XCLEAN_COMMON_H
