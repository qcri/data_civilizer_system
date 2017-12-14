//
// Created by Wenbo Tao on 12/27/16.
//

#ifndef XCLEAN_BRUTEFORCEJOINER_H
#define XCLEAN_BRUTEFORCEJOINER_H
#include "Joiner.h"
using namespace std;


class BruteForceJoiner : public Joiner
{
public:
	//constructors
	BruteForceJoiner() {}
	BruteForceJoiner(vector<t_rule>, vector<string>);

	//overwrite the virtual method
	vector<pair<double, pair<string, string>>> getJoinedStringPairs();
};


#endif //XCLEAN_BRUTEFORCEJOINER_H
