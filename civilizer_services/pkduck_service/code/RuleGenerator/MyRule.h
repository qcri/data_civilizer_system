//
// Created by Wenbo Tao on 1/6/17.
//

#ifndef XCLEAN_SIMPLENLPRULEGENERATOR_H
#define XCLEAN_SIMPLENLPRULEGENERATOR_H


#include "RuleGenerator.h"
using namespace std;


class MyRule: public RuleGenerator
{
public:
	//constructor
	MyRule(vector<string>);

	//overwrite virtual methods
	vector<t_rule> gen_rules();

private:
	//vowels
	const unordered_set<char> vowels =
			unordered_set<char>({'a', 'e', 'i', 'o', 'u'});
};


#endif //XCLEAN_SIMPLENLPRULEGENERATOR_H
