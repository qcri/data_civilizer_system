//
// Created by Wenbo Tao on 12/27/16.
//

#ifndef XCLEAN_JOINER_H
#define XCLEAN_JOINER_H
#include "../Common.h"
using namespace std;

class Joiner
{
public:
	//constructors
	Joiner() {}
	Joiner(vector<t_rule>, vector<string>);

	//join
	virtual vector<pair<double, pair<string, string>>> getJoinedStringPairs() = 0;

	//virtual destructor
	virtual ~Joiner() = default;

	//get applicable rules
	vector<t_rule> get_applicable_rules(int);

	//get original signatures
	unordered_set<string> get_o_sigs(int);

protected:
	//set of rules
	vector<t_rule> rules;
	unordered_set<t_rule, rule_hash> rule_hash_table;

	//cells
	vector<string> cells;

	//number of cells
	int n;

	//number of rules
	int r;

	//token sets and maps
	vector<vector<string>> tokens;
	vector<umpsi> token_maps;
	vector<vector<pair<int, int>>> matchable_tokens;

	//applicable rules
	vector<vector<int>> applicable_rule_ids;

	//expansion sets
	vector<unordered_set<string>> expansion_set;

	//global token rank list
	vector<string> global_list;
	umpsi token_rankings;

	//original signatures
	vector<unordered_set<string>> o_sigs, o_large_sigs;

	//functions
	unordered_set<string> buildOriginalSigs(int);
	unordered_set<string> buildOriginalLargeTokenSigs(int);
	void gen_applicable_rules();
	void gen_expansion_set();
	void gen_global_ranking();
	void gen_original_signatures();
};


#endif //XCLEAN_JOINER_H
