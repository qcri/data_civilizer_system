//
// Created by dongdeng on 6/12/17.
//

#ifndef ENUM_RULE_RULES_H
#define ENUM_RULE_RULES_H


#include "Table.h"
#include "Synthesize.h"

#include <tuple>


class Locator {
public:
  int id;
  int beg;
  int end;
  Locator(int v, int b, int e)
    : id(v), beg(b), end(e) { }
  bool operator<(const Locator &e) const
  {
    if (id < e.id)
      return true;
    if (id > e.id)
      return false;
    if (beg < e.beg)
      return true;
    if (beg > e.beg)
      return false;
    return end < e.end;
  }
  bool operator>(const Locator &e) const
  {
    if (id > e.id)
      return true;
    if (id < e.id)
      return false;
    if (beg > e.beg)
      return true;
    if (beg < e.beg)
      return false;
    return end > e.end;
  }
  inline bool operator==(const Locator &e) const
  {
    return (id == e.id && beg == e.beg && end == e.end);
  }
};

struct LocatorHash
{
  size_t operator()(const Locator &o) const
  {
    size_t res = 1009;
    res = res * 9176 + o.id;
    res = res * 9176 + o.beg;
    res = res * 9176 + o.end;
    return res;
  }
};



struct PairLocatorHash
{
  size_t operator()(const pair<Locator, Locator> &o) const
  {
    size_t res = 1009;
    res = res * 9176 + o.first.id;
    res = res * 9176 + o.first.beg;
    res = res * 9176 + o.first.end;
    res = res * 9176 + o.second.id;
    res = res * 9176 + o.second.beg;
    res = res * 9176 + o.second.end;
    return res;
  }
};


typedef tuple<int, int, int> triple;
struct TripleHash
{
  size_t operator()(const triple &k) const
  {
    size_t res = 1009;
    res = res * 9176 + get<0>(k);
    res = res * 9176 + get<1>(k);
    res = res * 9176 + get<2>(k);
    return res;
  }
};


class Rules {

public:
  Table &table;
  int col_id;

  vector<string> values;
  vector<vector<int>> clusters;  // cluster to rows in clusters
  vector<vector<int>> id_mappings;
  vector<int> counts;

  vector<int> row_to_cluster;

  int rule_types;
  const bool enable_auto_confirm_rules;
  const string token_delim = " ";  // make sure there is only 1 character
  string op_log = "mc,";

  unordered_map<pair<string, string>, int, pair_hash> termRules;  // larger string first
  // unordered_map<string, int> delTermRules;

  // a rule (first > second) -----> places to apply the rules ------>  timestamp
  unordered_map<pair<string, string>, unordered_map<pair<Locator, Locator>, pair<int, int>, PairLocatorHash>, pair_hash> rule_locations;
  unordered_map<pair<string, string>, vector<pair<tuple<int, int, int>, tuple<int, int, int>>>, pair_hash> termRulesApplicable;

  vector<vector<string>> valTokens;

  unordered_map<string, vector<pair<int, int>>> invIndex;  // row id and position
  // unordered_map<string, unordered_map<int, int>> ruleInvIndex;  // row id and position // TODO

  Rules(Table &t, int cid, const vector<string> &v, const vector<vector<int>> &c, const vector<vector<int>> &m, const vector<int> &f, int rtype, bool confirm)
          : table(t), col_id(cid), values(v), clusters(c), id_mappings(m), counts(f), rule_types(rtype), enable_auto_confirm_rules(confirm)
  {
    row_to_cluster.resize(values.size());
    for (auto i = 0; i < clusters.size(); i++)
      for (auto j : clusters[i])
        row_to_cluster[j] = i;
  }


  void GenerateRules();

  void RankRules(unordered_map<pair<string, string>, int, pair_hash> &rules, vector<pair<pair<string, string>, int>> &tops, int limit = 10);

  bool ApplyGroupRuleComplex(const vector<pair<pair<string, string>, int>> &rules,
                                    const vector<bool> &ruleDir,
                                    const vector<int> &ruleGroup,
                                    vector<int> &ruleStatus,
                                    const vector<vector<int>> &groupRules,
                                    vector<int> &groupStatus,
                                    const vector<Path> &transformations,
                                    vector<vector<tuple<int, int, int>>> &modification, int max_group_id, string tmp); // stpos, len_before, len_after


  string ShowNextCluster(vector<pair<pair<string, string>, int>> &rules, 
      vector<bool> &ruleDir,
      vector<int> &ruleGroup, 
      vector<int> &ruleStatus, vector<vector<int>> &groupRules,
      vector<int> &groupStatus,
      vector<Path> &transformations, 
      vector<pair<string, string>> &structures,
      vector<vector<tuple<int, int, int>>> &modification);

  bool ApplyRule(vector<pair<pair<string, string>, int>> &rules, vector<bool> &ruleDir,
                        vector<int> &ruleGroup, vector<int> &ruleStatus, vector<vector<int>> &groupRules, vector<int> &groupStatus,
                        vector<Path> &transformations, vector<pair<string, string>> &structures,
                        vector<vector<tuple<int, int, int>>> &modification, int max_group_id, string tmp);

private:

  int DetectChanges(const unordered_set<triple, TripleHash> &uniq_row_ids,
                    const vector<vector<tuple<int, int, int>>> &modification) const;

  pair<int, int> GreedilyPickNextGroup(const vector<pair<pair<string, string>, int>> &rules,
                                              const vector<int> &ruleGroup,
                                              const vector<int> &ruleStatus) const;

  string ShowGroups(const vector<pair<pair<string, string>, int>> &rules,
                         const vector<bool> &ruleDir,
                         const vector<vector<int>> &groupRules,
                         const vector<Path> &transformations,
                         const vector<vector<tuple<int, int, int>>> &modification,
                         int max_count, int max_group_id) const;

  int AutoChoice(const vector<pair<pair<string, string>, int>> &rules,
                 const vector<bool> &ruleDir,
                 const vector<int> &ruleGroup,
                 const vector<vector<int>> &groupRules,
                 const vector<vector<tuple<int, int, int>>> &modification,
                 const int max_group_id,
                 const string &tmp) const;

  void GenTermRules();

  void BuildInvIndex();

  inline string getPiece(const vector<string> &tokens, int st, int en);

  void ReplaceStringInPlace(string &subject, const string &search, const string &replace);

};



#endif //ENUM_RULE_RULES_H
