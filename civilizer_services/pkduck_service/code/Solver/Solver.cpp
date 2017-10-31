//
// Created by Wenbo Tao on 12/26/16.
//

#include "Solver.h"
#include "../RuleGenerator/FastLCS.h"
#include "../Joiner/BruteForceJoiner.h"
#include "../Joiner/PolynomialJoiner.h"
#include "../RuleGenerator/MyRule.h"
#include "../RuleGenerator/BadBoy.h"
#include "../RuleGenerator/WildLCS.h"
#include "../RuleGenerator/NLPRule.h"
#include "../RuleGenerator/Vldb09.h"
#include "../RuleGenerator/HandDict.h"
#include "../RuleGenerator/GPDict.h"
#include <fstream>
using namespace std;

Solver::Solver(string string_file_name)
{
	ifstream fin1(string_file_name.c_str());
	for (string cell; getline(fin1, cell);)
		cells.push_back(cell);
	fin1.close();
}

Solver::Solver(vector<string> _cells)
{
	cells = _cells;
}

vector<pair<string, string>> Solver::solve()
{
	n = cells.size();

	//freq
	umpsi freq;
	int sum_tokens = 0;
	for (string s : cells)
	{
		vector<string> tokens = Common::get_tokens(s);
		for (string t : tokens)
			freq[t] ++;
		sum_tokens += tokens.size();
	}

	//generate rules
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	cerr << "Generating rules......" << endl;
	RuleGenerator *ruleGenerator;
	if (Common::DICTIONARY == 0)
		ruleGenerator = new MyRule(cells);
	else if (Common::DICTIONARY == 1)
		ruleGenerator = new Vldb09(cells);
	else
		ruleGenerator = new GPDict();

	vector<t_rule> rules = ruleGenerator->gen_rules();
//	for (auto r : rules)
//		Common::print_rule(r);
//	cout << "# Rule: " << rules.size() * 2 << endl;
	gettimeofday(&t2, NULL);
	double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
	cout << endl << "Dictionary generation took : " << elapsedTime << "s." << endl << endl;

	//add reverse rules
	for (int i = 0, n = (int) rules.size(); i < n; i ++)
		rules.emplace_back(make_pair(rules[i].second, rules[i].first));

	//calculate number of distinct lhs
	umpsi lhs_set;
	for (t_rule rule : rules)
		if ((rule.first.size() < rule.second.size()) ||
				(rule.first.size() == 1 && rule.first[0].size() < rule.second[0].size()))
			lhs_set[rule.first[0]] ++;
	cout << "# Distinct lhs: " << lhs_set.size() << endl;

	//output top 10 bad boys
	vector<pair<int, string>> sort_array;
	for (auto cp : lhs_set)
		sort_array.emplace_back(cp.second, cp.first);
	sort(sort_array.begin(), sort_array.end(), greater<pair<int, string>>());
	for (auto i = 0; i < min((int) sort_array.size(), 10); i ++)
		cout << sort_array[i].second<< " : " << sort_array[i].first << endl;

	//frequency of abbreviations having multiple full forms
	double sum_freq = 0;
	for (auto cp : lhs_set)
		if (cp.second > 5)
			sum_freq += (double) freq[cp.first] / (double) sum_tokens;
	cout << "Frequency of naughty abbreviations: " << sum_freq << endl;

	//joins
	cerr << "Joining......" << endl;
	Joiner *joiner = new PolynomialJoiner(rules, cells);
	vector<pair<double, pair<string, string>>> joinedStringPairs = joiner->getJoinedStringPairs();

	//output
	cout << joinedStringPairs.size() << endl;
	sort(joinedStringPairs.begin(), joinedStringPairs.end());
//	for (auto cp : joinedStringPairs)
//		cout << cp.second.first << endl << cp.second.second << endl << cp.first << endl << endl;

	//return vector pair
	vector<pair<string, string>> results;
	for (auto cp : joinedStringPairs)
		results.emplace_back(cp.second.first, cp.second.second);

	delete ruleGenerator;
	delete joiner;

	return results;
}
