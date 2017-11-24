//
// Created by Wenbo Tao on 12/26/16.
//

#include "RuleGenerator.h"

RuleGenerator::RuleGenerator(vector<string> s)
{
	cells = s;
	n = cells.size();

	//build token sets and token maps
	tokens.clear();
	token_maps.clear();
	for (int i = 0; i < n; i ++)
	{
		tokens.push_back(Common::get_tokens(cells[i]));
		token_maps.push_back(umpsi());
		for (string s : tokens[i])
			token_maps[i][s] ++;
	}
}
