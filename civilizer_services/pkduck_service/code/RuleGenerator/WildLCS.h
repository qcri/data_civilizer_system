//
// Created by Wenbo Tao on 1/25/17.
//

#ifndef XCLEAN_WILDLCS_H
#define XCLEAN_WILDLCS_H
#include "RuleGenerator.h"
using namespace std;


class WildLCS : public RuleGenerator
{
public:
	//constructor
	WildLCS(vector<string>);

	//overwrite the virtual method
	vector<t_rule> gen_rules();

private:
	//trie
	vector<unordered_map<char, int>> trie;
	vector<string> contain_word;
};

#endif //XCLEAN_WILDLCS_H
