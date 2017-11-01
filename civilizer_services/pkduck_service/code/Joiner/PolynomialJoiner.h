//
// Created by Wenbo Tao on 12/30/16.
//

#ifndef XCLEAN_POLYNOMIALJOINER_H
#define XCLEAN_POLYNOMIALJOINER_H
#include "Joiner.h"
using namespace std;


class PolynomialJoiner : public Joiner
{
public:
	//constructors
	PolynomialJoiner() {}
	PolynomialJoiner(vector<t_rule>, vector<string>);

	//overwrite the virtual method
	vector<pair<double, pair<string, string>>> getJoinedStringPairs();

	//get sigs
	unordered_set<string> get_t_sigs(int x);

private:

	//the maximum number of tokens a string could have
	int k;

	//signatures
	vector<unordered_set<string>> t_sigs, e_sigs, t_large_sigs;

	//inverted list for applicable rules, speed up verification
	vector<unordered_map<string, unordered_set<int>>> a_rule_inv_list;

	//pair of applicable rules and starting points
	vector<vector<pair<t_rule, int>>> app_rule_w_st;

	//Sig builders
	unordered_set<string> buildDpSigs(vector<string>, vector<t_rule>);
	unordered_set<string> buildDpSigsSlow(vector<string>, vector<t_rule>);
	unordered_set<string> buildJacctSigs(vector<string>, vector<t_rule>);
	unordered_set<string> buildExpansionSigs(vector<string>, vector<t_rule>);
	unordered_set<string> buildDpLargeTokenSigs(vector<string>, vector<t_rule>);

	//SIGMOD 13 verifier
	double sigmod13_get_similarity(int, int);
	double rule_gain(t_rule, umpsi, int);

	//icde 08 verifier
	double icde08_get_similarity(int, int);

	//large token verifier
	double large_token_get_similarity(int, int);

	//normal greedy verifier
	double greedy_get_similarity(int, int);
	double greedy_directed_get_similarity(int, int);
};


#endif //XCLEAN_POLYNOMIALJOINER_H
