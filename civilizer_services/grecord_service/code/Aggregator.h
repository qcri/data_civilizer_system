//
// Created by dongdeng on 6/26/17.
//

#ifndef ENUM_RULE_AGGREGATOR_H
#define ENUM_RULE_AGGREGATOR_H

#include "Wrapper.h"
#include <map>
#include <regex>
#include <algorithm>

class Path;

class Aggregator {
public:

  const vector<pair<pair<string, string>, int>> &rules;


  vector<bool> ruleDir;
  vector<int> ruleGroup;
  vector<int> ruleStatus;
  vector<Path> transformations;
  vector<pair<string, string>> structures;
  vector<vector<int>> groupRules;

  vector<int> format_group_cluster_ids;
  vector<vector<int>> format_group_rule_ids;
  vector<vector<pair<pair<string, string>, int>>> format_group_rules; // multiple lists of rules with the same format

  unordered_map<string, pair<double, int>> global_const_terms;
  vector<unordered_map<string, pair<double, int>>> local_const_terms;

  vector<int> cluster_sizes;
  vector<pair<int, int>> forward_list; // rule id to cluster id in ordered (->, <-)
  vector<pair<pair<string, string>, int>> ordered;

  Aggregator(const vector<pair<pair<string, string>, int>> &allRules) : rules(allRules) { }

  void Aggregate();
  void GroupAggregate();
  void AggregateStructure();
  void NoAggregatation();

  void Group();
  void RandomGroup();
  void CalConstantTerms();

private:
  // string FindReplace(const string &str, const regex &exp, const string &replace);
};


#endif //ENUM_RULE_AGGREGATOR_H
