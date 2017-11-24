//
// Created by Wenbo Tao on 1/18/17.
//

#ifndef XCLEAN_BADBOY_H
#define XCLEAN_BADBOY_H

#include "RuleGenerator.h"
using namespace std;


class BadBoy : public RuleGenerator
{
public:
	//Constructor
	BadBoy(vector<string>);

	//overwrite virtual methods
	vector<t_rule> gen_rules();
};


#endif //XCLEAN_BADBOY_H
