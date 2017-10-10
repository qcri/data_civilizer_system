//
// Created by dongdeng on 6/12/17.
//

#include "Rules.h"
#include "Synthesize.h"

inline string Rules::getPiece(const vector<string> &toks, int st, int en)
{
  if (st >= en)
    return "";

  string piece = toks[st];
  for (auto pos = st + 1; pos < en; pos++)
    piece += (" " + toks[pos]);
  return piece;
}


void Rules::RankRules(unordered_map<pair<string, string>, int, pair_hash> &rules, vector<pair<pair<string, string>, int>> &tops, int limit) {

#ifdef PRUNE_BY_STRUCTURE_ENABLE
  unordered_map<pair<string, string>, vector<pair<pair<string, string>, int>>, pair_hash> clusters;
  for (auto &entry : rules)
  {
    string input = entry.first.first;
    string output = entry.first.second;
    for (int j = 0; j < AGG_REGEX_NUM; j++)
    {
      input = Wrapper::FindReplace(input, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
      output = Wrapper::FindReplace(output, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
    }
    if (input == output)
      clusters[make_pair(input, output)].push_back(entry);
    else if (input > output)
      clusters[make_pair(input, output)].push_back(entry);
    else
      clusters[make_pair(output, input)].push_back(entry);
  }

  vector<pair<pair<string, string>, int>> ordered;
  for (auto &entry : clusters)
  {
    int cnt = 0;
    for (auto &r : entry.second)
      cnt += r.second;
    ordered.emplace_back(entry.first, cnt);
  }
  sort(ordered.begin(), ordered.end(), [](const pair<pair<string, string>, int> &v1, const pair<pair<string, string>, int> &v2){
    return v1.second > v2.second; });

  int count = 0;
  for (auto id = 0; id < ordered.size(); id++)
    if (ordered[id].second < PRUNE_BY_STRUCTURE_THRESHOLD || id >= PRUNE_BY_STRUCTURE_GROUP_NUM)
      for (auto &r : clusters[ordered[id].first])
        rules.erase(r.first);

#endif
  if (limit > rules.size())
    limit = rules.size();

  tops.clear();
  tops.resize(limit);
  partial_sort_copy(rules.begin(), rules.end(), tops.begin(), tops.end(),
                    [](pair<pair<string, string>, int> const& l,
                       pair<pair<string, string>, int> const& r)
                    {
                      return l.second > r.second;
                    }
  );
}

void Rules::ReplaceStringInPlace(string& subject, const string& search, const string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != string::npos)
  {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

void Rules::BuildInvIndex()
{
  valTokens.resize(values.size());
  for (auto i = 0; i < values.size(); i++)
  {
    Wrapper::strToTokens(values[i], valTokens[i], token_delim);
    for (auto j = 0; j < valTokens[i].size(); j++)
      invIndex[valTokens[i][j]].emplace_back(i, j);
  }
}


void Rules::GenTermRules()
{
  for (auto &cluster : clusters)
  {
    for (auto m = 0; m < cluster.size(); m++)
    {
#ifdef SIMILARITY_CHECK_ENABLE
      unordered_set<string> all_tokens;
      for (auto &tmp_str : valTokens[cluster[m]])
        all_tokens.insert(tmp_str);
#endif
      for (auto n = m + 1; n < cluster.size(); n++)
      {
        int row1 = cluster[m];
        int row2 = cluster[n];

        auto &v1 = values[row1];
        auto &v2 = values[row2];

        if (v1 == v2) continue;
        if (v1.empty() || v2.empty()) continue;

        // build term rules
        // rule_type == 1: replace rule only
        // rule_type == 2: replace rule + full rule
        // rule_type == 3: replace rule + delete rule
        // rule_type == 4: replace rule + delete rule + full rule
        // rule_type == 5: full rule

        vector<string> &tok1 = valTokens[row1];
        vector<string> &tok2 = valTokens[row2];

#ifdef LCS_CHECK_ENABLE
        LCSLength(v1, v2, v1.length(), v2.length());
        int lcs_len = matrix[v1.length()][v2.length()];
        int min_len = v1.length();
        if (v1.length() > v2.length())
          min_len = v2.length();
        if (lcs_len * 1.0 / min_len < LCS_THRESHOLD)
        {
          // cout << "sim: " << lcs_len * 1.0 / min_len << "skipped" << endl;
          // cout << v1 << endl << v2 << endl << lcs_len << endl;
          continue;
        } else {
          double size_filter = tok1.size() * 1.0 / tok2.size();
          if (tok1.size() > tok2.size())
            size_filter = tok2.size() * 1.0 / tok1.size();
          if (size_filter < SIZE_FILTER_THRESHOLD)
            continue;
          cout << "sim: " << lcs_len * 1.0 / min_len << " looking for rules" << endl;
          cout << v1 << endl << v2 << endl << lcs_len << " " << lcs_len * 1.0 / min_len << " " << size_filter << endl;
        }
#endif

#ifdef SIMILARITY_CHECK_ENABLE
        int overlap = 0;
        for (auto &tmp_str : tok2)
          if (all_tokens.count(tmp_str))
            ++overlap;
        double sim = overlap * 1.0 / (all_tokens.size() + tok2.size() - overlap);
        if (sim < SIMILARITY_THRESHOLD) {
          cout << "sim: " << sim << " skipped" << endl;
          cout << v1 << endl << v2 << endl << endl;
          continue;
        } else {
          cout << "sim: " << sim << " looking for rules" << endl;
          cout << v1 << endl << v2 << endl << endl;
        }
#endif

        // for value-value rule
        if (rule_types == 2 || rule_types == 5 || rule_types == 4) {
          if (v1 > v2) {

#ifdef BOOST_DUP_COUNTS_ENABLE
            termRules[make_pair(v1, v2)] += (counts[row1] * counts[row2]);
#else
            termRules[make_pair(v1, v2)] += 1;
#endif

#ifdef RULE_UPDATE_ENABLE
            rule_locations[make_pair(v1, v2)].insert(make_pair(make_pair(
                    Locator(row1, 0, tok1.size()),
                    Locator(row2, 0, tok2.size())), make_pair(0, 0)));
#else
            termRulesApplicable[make_pair(v1, v2)].emplace_back(
                    make_tuple(row1, 0, tok1.size()),
                    make_tuple(row2, 0, tok2.size()));
#endif
          } else if (v1 < v2) {

#ifdef BOOST_DUP_COUNTS_ENABLE
            termRules[make_pair(v2, v1)] += (counts[row2] * counts[row1]);
#else
            termRules[make_pair(v2, v1)] += 1;
#endif

#ifdef RULE_UPDATE_ENABLE
            rule_locations[make_pair(v2, v1)].insert(make_pair(make_pair(
                    Locator(row2, 0, tok2.size()),
                    Locator(row1, 0, tok1.size())), make_pair(0, 0)));
#else
            termRulesApplicable[make_pair(v2, v1)].emplace_back(
                    make_tuple(row2, 0, tok2.size()),
                    make_tuple(row1, 0, tok1.size()));
#endif


          }
        }

        // for delete rule and replace rule
        if (rule_types == 5)
          continue;

        vector<pair<pair<int, int>, pair<int, int>>> diffs;
        Wrapper::Alignment<vector<string>>(tok1, tok2, tok1.size(), tok2.size(), diffs, false);

        for (auto &entry : diffs) {
          auto &stPos = entry.first;
          auto &enPos = entry.second;

          string piece1 = getPiece(tok1, stPos.first, enPos.first);
          string piece2 = getPiece(tok2, stPos.second, enPos.second);

          if (piece1.empty() || piece2.empty())
            if (rule_types != 3 && rule_types != 4)  // no deletion rule
              continue;

          if (piece1 > piece2) {

            termRules[make_pair(piece1, piece2)] += 1;

#ifdef RULE_UPDATE_ENABLE
            rule_locations[make_pair(piece1, piece2)].insert(make_pair(make_pair(
                    Locator(row1, stPos.first, enPos.first),
                    Locator(row2, stPos.second, enPos.second)), make_pair(0, 0)));
#else
            termRulesApplicable[make_pair(piece1, piece2)].emplace_back(
                    make_tuple(row1, stPos.first, enPos.first),
                    make_tuple(row2, stPos.second, enPos.second));
#endif
          } else if (piece1 < piece2) {

            termRules[make_pair(piece2, piece1)] += 1;

#ifdef RULE_UPDATE_ENABLE
            rule_locations[make_pair(piece2, piece1)].insert(make_pair(make_pair(
                    Locator(row2, stPos.second, enPos.second),
                    Locator(row1, stPos.first, enPos.first)), make_pair(0, 0)));
#else
            termRulesApplicable[make_pair(piece2, piece1)].emplace_back(
                    make_tuple(row2, stPos.second, enPos.second),
                    make_tuple(row1, stPos.first, enPos.first));
#endif
          }
        }
      }
    }
  }
}


void Rules::GenerateRules()
{
  timeval t1, t2, t3;
  gettimeofday(&t1, NULL);

  BuildInvIndex();

  Wrapper::logTime(t1, t2, "   --> Building Rule Index Time: ");

  GenTermRules();

  Wrapper::logTime(t2, t3, "   --> Producing Term Rules Time: ");
}


/*********** Applying Rules ***************/
pair<int, int> Rules::GreedilyPickNextGroup(const vector<pair<pair<string, string>, int>> &rules,
                          const vector<int> &ruleGroup,
                          const vector<int> &ruleStatus) const
{
  // pick a group to apply
  unordered_map<int, int> groupCount;
  for (int rid = 0; rid < rules.size(); rid++)
  {
    int apply_num = rules[rid].second;
    int group_id = ruleGroup[rid];
    int status = ruleStatus[rid];

    if (status == 0)  // 0: not determined yet;    1: deleted rules    2: applied rules     3: do not apply    4: postpone
      groupCount[group_id] += apply_num;
    if (status == 4)
      groupCount[group_id] += (apply_num / 100); // TODO: deal with option 4, either remove it or do it.
  }

  // find the one to ask and apply
  int max_count = 0;
  int max_group_id = -1;
  for (auto &entry : groupCount)
    if (entry.second > max_count)
    {
      max_group_id = entry.first;
      max_count = entry.second;
    }

  return make_pair(max_count, max_group_id);
}


int Rules::DetectChanges(const unordered_set<triple, TripleHash> &uniq_row_ids,
                         const vector<vector<tuple<int, int, int>>> &modification) const
{
  int counter = 0;
  unordered_map<int, vector<triple>> clusters_to_print;
  for (auto &entry : uniq_row_ids)
  {
    int cluster_id = row_to_cluster[get<0>(entry)];
    clusters_to_print[cluster_id].push_back(entry);
  }

  for (auto &cluster_to_print : clusters_to_print)
  {
    unordered_map<string, int> going_to_apply;
    unordered_set<int> going_to_apply_rids;
    for (auto &entry : cluster_to_print.second)
    {
      int row_id = get<0>(entry);
      int st_pos = get<1>(entry);
      int en_pos = get<2>(entry);
      int offset = 0;
      bool flag = true;
      for (auto &log : modification[row_id])
      {
        int from_pos = get<0>(log);
        int prev_len = get<1>(log);
        int next_len = get<2>(log);
        if (from_pos >= en_pos + offset)
          continue;
        else if (from_pos + prev_len <= st_pos + offset)
          offset += (next_len - prev_len);
        else
        {
          flag = false;
          break;
        }
      }
      if (!flag)
        continue;
      else
      {
        going_to_apply_rids.insert(row_id);
        going_to_apply[values[row_id]] += 1;
        counter++;
      }
    }

    unordered_map<string, int> not_going_to_apply;
    for (auto row_id : clusters[cluster_to_print.first])
      if (!going_to_apply_rids.count(row_id))
        not_going_to_apply[values[row_id]] += 1;

    cout << "going to apply within a cluster: " << endl;
    for (auto &entry : not_going_to_apply)
      cout << "|" << entry.first << "| x" << entry.second << endl;

    cout << "print the rests in the cluster: " << endl;
    for (auto &entry : going_to_apply)
      cout << "|" << entry.first << "| x" << entry.second << endl;
    cout << endl;
  }
  return counter;
}

string Rules::ShowGroups(const vector<pair<pair<string, string>, int>> &rules,
                const vector<bool> &ruleDir,
                const vector<vector<int>> &groupRules,
                const vector<Path> &transformations,
                const vector<vector<tuple<int, int, int>>> &modification,
                int max_count, int max_group_id) const
{
  ostringstream buffer;
  // show the transformation
  buffer << endl << "group: ------" << endl;
  for (auto &e : transformations[max_group_id].path)
    buffer << e.first << " --- " << e.second << endl;
  buffer << endl << endl;

  // show the rules
  for (auto &rid : groupRules[max_group_id])
  {
    if (rules[rid].second > 0)
    {
      // false is first --> second while true is second -->first
      if (!ruleDir[rid])
        buffer << rid << ": |" << rules[rid].first.first << "|-->|" << rules[rid].first.second << "| x"
             << rules[rid].second << endl;
      else
        buffer << rid << ": |" << rules[rid].first.second << "|-->|" << rules[rid].first.first << "| x"
             << rules[rid].second << endl;
    }
  }

  // show sample values
  buffer << "Sample Applications:" << endl;
  int sample_num = 10;
  for (auto &rid : groupRules[max_group_id])
  {
    pair<string, string> mapping = rules[rid].first;
    if (rules[rid].first.first < rules[rid].first.second)
      mapping = make_pair(rules[rid].first.second, rules[rid].first.first);

#ifdef RULE_UPDATE_ENABLE
    auto trit = rule_locations.find(mapping);
    if (trit == rule_locations.end())
      continue;
    for (auto &p : trit->second)
    {
      int row_id1 = p.first.first.id;
      int row_id2 = p.first.second.id;
      buffer << values[row_id1] << " --- ";
      buffer << values[row_id2] << endl;
      break;
    }
#else
    auto trit = termRulesApplicable.find(mapping);
    if (trit == termRulesApplicable.end())
      continue;

    for (auto &p : trit->second)
    {
      int row_id1 = get<0>(p.first);
      int row_id2 = get<0>(p.second);
      buffer << values[row_id1] << " --- ";
      buffer << values[row_id2] << endl;
      break;
    }
#endif

    if (--sample_num == 0)  break;
  }

  buffer << endl << "number of rules in this group: " << groupRules[max_group_id].size() <<  endl << "max count: " << max_count << endl;
  buffer << "------------" << endl << endl;
  buffer << "0: do not apply all of them;  1: apply them to all;  2: apply them reversely; 3: postpone;  4; exit" << endl;
  return buffer.str();
}


int Rules::AutoChoice(const vector<pair<pair<string, string>, int>> &rules,
                      const vector<bool> &ruleDir,
                      const vector<int> &ruleGroup,
                      const vector<vector<int>> &groupRules,
                      const vector<vector<tuple<int, int, int>>> &modification,
                      const int max_group_id,
                      const string &tmp) const
{
  int counter = 0;
  for (auto &rid : groupRules[max_group_id])
  {
    bool isRev = false;
    auto rule = rules[rid].first;
    auto dir = ruleDir[rid];
    if (dir == true)
      isRev = !isRev;

    if (tmp == "2")
      isRev = !isRev;

    string from_str, to_str;
    if (isRev == false) {
      from_str = rule.first;
      to_str = rule.second;
    } else {
      from_str = rule.second;
      to_str = rule.first;
    }

    if (rule.first < rule.first)
      rule = make_pair(rule.second, rule.first);

    unordered_set<triple, TripleHash> uniq_row_ids;
    auto trit = termRulesApplicable.find(rule);
    if (trit == termRulesApplicable.end())
      continue;

    for (auto p : trit->second) {
      auto &entry = p.first;
      if (from_str != rule.first) {
        entry = p.second;
      }
      uniq_row_ids.insert(entry);
    }

    for (auto &entry : uniq_row_ids) {
      int row_id = get<0>(entry);
      int st_pos = get<1>(entry);
      int en_pos = get<2>(entry);
      int offset = 0;
      bool flag = true;
      for (auto &log : modification[row_id]) {
        int from_pos = get<0>(log);
        int prev_len = get<1>(log);
        int next_len = get<2>(log);
        if (from_pos >= en_pos + offset)
          continue;
        else if (from_pos + prev_len <= st_pos + offset)
          offset += (next_len - prev_len);
        else {
          flag = false;
          break;
        }
      }
      if (!flag)
        continue;
      else
        counter++;
    }
  }
  return counter;
}


bool Rules::ApplyGroupRuleComplex(const vector<pair<pair<string, string>, int>> &rules,
                                  const vector<bool> &ruleDir,
                                  const vector<int> &ruleGroup,
                                  vector<int> &ruleStatus,
                                  const vector<vector<int>> &groupRules,
                                  vector<int> &groupStatus,
                                  const vector<Path> &transformations,
                                  vector<vector<tuple<int, int, int>>> &modification,
                                  int max_group_id, string tmp)
{
  if (tmp == "0")
  {
    for (auto &rid : groupRules[max_group_id])
      ruleStatus[rid] = 3;
  }
  else if (tmp == "3")
  {
    for (auto &rid : groupRules[max_group_id])
      ruleStatus[rid] = 4;
  }
  else if (tmp == "1" || tmp == "2")
  {
    if (tmp == "1")
      groupStatus[max_group_id] = 1;
    else
      groupStatus[max_group_id] = 2;

    for (auto &rid : groupRules[max_group_id])
    {
      ruleStatus[rid] = 2;

      bool isRev = false;
      auto rule = rules[rid].first;

      auto dir = ruleDir[rid];
      if (dir == true)
        isRev = !isRev;

      if (tmp == "2")
        isRev = !isRev;

      string from_str, to_str;
      if (isRev == false)
      {
        from_str = rule.first;
        to_str = rule.second;
      } else {
        from_str = rule.second;
        to_str = rule.first;
      }

      if (rule.first < rule.second)
        rule = make_pair(rule.second, rule.first);

      int from_size, to_size;
      unordered_set<triple, TripleHash> uniq_row_ids;
      for (auto &p : termRulesApplicable[rule])
      {
        auto &entry = p.first;
        from_size = get<2>(p.first) - get<1>(p.first);
        to_size = get<2>(p.second) - get<1>(p.second);
        if (from_str != rule.first)
        {
          entry = p.second;
          int tmp_size = from_size;
          from_size = to_size;
          to_size = tmp_size;
        }
        uniq_row_ids.insert(entry);
      }

      for (auto &entry : uniq_row_ids)
      {
        int row_id = get<0>(entry);
        int st_pos = get<1>(entry);
        int en_pos = get<2>(entry);
        auto &val = values[row_id];

#ifdef OUTPUT_INFO
        cout << endl << endl << val << "-------->" << endl;
#endif

        int offset = 0;
        bool flag = true;
        vector<string> &toks = valTokens[row_id];
        for (auto &log : modification[row_id])
        {
          int from_pos = get<0>(log);
          int prev_len = get<1>(log);
          int next_len = get<2>(log);
          if (from_pos >= en_pos + offset)
            continue;
          else if (from_pos + prev_len <= st_pos + offset)
            offset += (next_len - prev_len);
          else
          {
            flag = false;
            break;
          }
        }
        if (!flag)
          continue;

        string new_value = "";
        for (auto x = 0; x < st_pos + offset; x++)
          new_value += (toks[x] + " ");
        new_value += to_str;
        if (!new_value.empty() && new_value.back() == ' ')
          new_value.pop_back();
        for (auto x = en_pos + offset; x < toks.size(); x++)
          new_value += (" " + toks[x]);
        modification[row_id].push_back(make_tuple(st_pos + offset, from_size, to_size));
        values[row_id] = new_value;

        // update the table!!
        for (auto origin_row_id : id_mappings[row_id])
          table.rows[origin_row_id][col_id] = new_value;

#ifdef OUTPUT_INFO
        cout << new_value << endl;
#endif

        toks.clear();
        Wrapper::strToTokens(new_value, toks, token_delim);
      }
    }
  } else if (tmp == "4") {
    cout << "exit" << endl;
    return false;
  } else {
    cout << "invalid option" << endl;
  }
  return true;
}


string Rules::ShowNextCluster(vector<pair<pair<string, string>, int>> &rules, 
                               vector<bool> &ruleDir,
                               vector<int> &ruleGroup, 
                               vector<int> &ruleStatus, vector<vector<int>> &groupRules,
                               vector<int> &groupStatus,
                               vector<Path> &transformations, 
                               vector<pair<string, string>> &structures,
                               vector<vector<tuple<int, int, int>>> &modification) 
{
  ostringstream buffer;
  auto res_pair = GreedilyPickNextGroup(rules, ruleGroup, ruleStatus);
  int max_count = res_pair.first;
  int max_group_id = res_pair.second;
  buffer << max_group_id << "\t" << max_count << "\t" << endl;

  if (max_count == 0) 
    return buffer.str();

  buffer << ShowGroups(rules, ruleDir, groupRules, transformations, modification, max_count, max_group_id);
  return buffer.str();
}

// with update
bool Rules::ApplyRule(vector<pair<pair<string, string>, int>> &rules, vector<bool> &ruleDir,
                      vector<int> &ruleGroup, vector<int> &ruleStatus, vector<vector<int>> &groupRules,
                      vector<int> &groupStatus,
                      vector<Path> &transformations, vector<pair<string, string>> &structures,
                      vector<vector<tuple<int, int, int>>> &modification, int max_group_id, string tmp)
{
  if (tmp == "0") {
    for (auto &rid : groupRules[max_group_id])
      ruleStatus[rid] = 3;
  } else if (tmp == "3") {
    for (auto &rid : groupRules[max_group_id])
      ruleStatus[rid] = 4;
  } else if (tmp == "1" || tmp == "2") {
    if (tmp == "1")
      groupStatus[max_group_id] = 1;
    else
      groupStatus[max_group_id] = 2;

    int current_rid = 0;
    for (auto &rid : groupRules[max_group_id])
    {
      cout << "Current RID:" << current_rid++ << "/" << groupRules[max_group_id].size() << endl;
      if (rid < 0 || rid > rules.size())
      {
        cout << "NOTICE: ASSERT FAILED!!!" << rid << " " << max_group_id << endl;
        continue;
      }

      ruleStatus[rid] = 2;

      bool isRev = false;
      auto rule = rules[rid].first;

      auto dir = ruleDir[rid];
      if (dir == true)
        isRev = !isRev;

      if (tmp == "2")
        isRev = !isRev;

      string from_str, to_str;
      if (isRev == false) {
        from_str = rule.first;
        to_str = rule.second;
      } else {
        from_str = rule.second;
        to_str = rule.first;
      }

      if (rule.first < rule.second)
        rule = make_pair(rule.second, rule.first);

      int from_size, to_size;
      unordered_map<Locator, int, LocatorHash> uniq_row_ids;

      if (from_str == rule.first)
      {
        for (auto &p : rule_locations[rule])
        {
#ifdef OUTPUT_INFO
          // if (uniq_row_ids.count(p.first.first))
          //  assert(uniq_row_ids[p.first.first] == p.second.first);
#endif
          from_size = p.first.first.end - p.first.first.beg;
          to_size = p.first.second.end - p.first.second.beg;
          uniq_row_ids.insert(make_pair(p.first.first, p.second.first));
        }
      }
      else
      {
        for (auto &p : rule_locations[rule])
        {
#ifdef OUTPUT_INFO
          // if (uniq_row_ids.count(p.first.second))
          //  assert(uniq_row_ids[p.first.second] == p.second.second);
#endif
          from_size = p.first.second.end - p.first.second.beg;
          to_size = p.first.first.end - p.first.first.beg;
          uniq_row_ids.insert(make_pair(p.first.second, p.second.second));
        }
      }


      int current_unique_row = 0;
      for (auto &entry : uniq_row_ids)
      {
        int row_id = entry.first.id;
        int st_pos = entry.first.beg;
        int en_pos = entry.first.end;
        int log_from = entry.second;
        auto &val = values[row_id];

#ifdef OUTPUT_INFO
        cout << endl << endl << val << "-------->" << endl;
#endif

        int offset = 0;
        bool flag = true; // to test if there is any overlap
        vector<string> &toks = valTokens[row_id];

        for (auto idx = log_from; idx < modification[row_id].size(); idx++)
        {
          auto &log = modification[row_id][idx];
          int from_pos = get<0>(log);
          int prev_len = get<1>(log);
          int next_len = get<2>(log);
          if (from_pos >= en_pos + offset)
            continue;
          else if (from_pos + prev_len <= st_pos + offset)
            offset += (next_len - prev_len);
          else
          {
            flag = false;
            break;
          }
        }
        if (!flag)
          continue;

        string new_value = "";
        for (auto x = 0; x < st_pos + offset; x++)
          new_value += (toks[x] + " ");
        new_value += to_str;
        if (!new_value.empty() && new_value.back() == ' ')
          new_value.pop_back();
        for (auto x = en_pos + offset; x < toks.size(); x++)
          new_value += (" " + toks[x]);

#ifdef OUTPUT_INFO
        vector<string> from_str_tok, to_str_tok;
        Wrapper::strToTokens(from_str, from_str_tok, token_delim);
        Wrapper::strToTokens(to_str, to_str_tok, token_delim);
        cout << new_value << endl;
#endif

        modification[row_id].push_back(make_tuple(st_pos + offset, from_size, to_size));
        // update the table!!
        for (auto origin_row_id : id_mappings[row_id])
          table.rows[origin_row_id][col_id] = new_value;

        /*** update ***/
        int new_row_id = row_id;
        const auto &new_val = new_value;
        vector<string> new_tok;
        Wrapper::strToTokens(new_value, new_tok, token_delim);

        int cluster_id = row_to_cluster[row_id];
        auto &cluster = clusters[cluster_id];

        unordered_set<pair<string, string>, pair_hash> new_rules;

#ifdef SIMILARITY_CHECK_ENABLE
        unordered_set<string> all_tokens;
        for (auto &tmp_str : new_tok)
          all_tokens.insert(tmp_str);
#endif

        for (auto n = 0; n < cluster.size(); n++)
        {
          if (cluster[n] == row_id)
            continue;

          int old_row_id = row_id;
          int dup_row_id = cluster[n];

          const auto &old_val = values[old_row_id];
          const auto &dup_val = values[dup_row_id];

          const vector<string> &old_tok = valTokens[old_row_id];
          const vector<string> &dup_tok = valTokens[dup_row_id];
#ifdef OUTPUT_INFO
          cout << endl << "NEW: " << new_val << endl << "OLD: " << old_val << endl << "DUP: " << dup_val << endl;
#endif

#ifdef LCS_CHECK_ENABLE
          LCSLength(new_val, dup_val, new_val.length(), dup_val.length());
          int lcs_len = matrix[new_val.length()][dup_val.length()];
          int min_len = new_val.length();
          if (new_val.length() > dup_val.length())
            min_len = dup_val.length();
          if (lcs_len * 1.0 / min_len < LCS_THRESHOLD)
          {
            // cout << "sim: " << lcs_len * 1.0 / min_len << "skipped" << endl;
            // cout << new_val << endl << dup_val << endl << lcs_len << endl;
            continue;
          } else {
            double size_filter = new_tok.size() * 1.0 / dup_tok.size();
            if (new_tok.size() > dup_tok.size())
              size_filter = dup_tok.size() * 1.0 / new_tok.size();
            if (size_filter < SIZE_FILTER_THRESHOLD)
              continue;
            // cout << "sim: " << lcs_len * 1.0 / min_len << " looking for rules" << endl;
            // cout << new_val << endl << dup_val << endl << lcs_len << " " << lcs_len * 1.0 / min_len << " " << size_filter << endl;
          }
#endif

#ifdef SIMILARITY_CHECK_ENABLE
          int overlap = 0;
          for (auto &tmp_str : dup_tok)
            if (all_tokens.count(tmp_str))
              ++overlap;
          double sim = overlap * 1.0 / (all_tokens.size() + dup_tok.size() - overlap);
          if (sim < SIMILARITY_THRESHOLD) {
            cout << "sim: " << sim << " skipped" << endl;
            cout << dup_val << endl << new_val << endl << endl;
            continue;
          } else {
            cout << "sim: " << sim << " looking for rules" << endl;
            cout << dup_val << endl << new_val << endl << endl;
          }
#endif

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
          unordered_map<pair<string, string>, unordered_set<pair<Locator, Locator>, PairLocatorHash>, pair_hash> to_delete_rules;
#endif

          if (old_val != dup_val && !old_val.empty() && !dup_val.empty())
          {

            // build term rules
            // rule_type == 1: replace rule only
            // rule_type == 2: replace rule + full rule
            // rule_type == 3: replace rule + delete rule
            // rule_type == 4: replace rule + delete rule + full rule
            // rule_type == 5: full rule

            if (rule_types == 2 || rule_types == 5 || rule_types == 4)
            {
              if (old_val > dup_val) {
                if (termRules[make_pair(old_val, dup_val)] > 0) {

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                  to_delete_rules[make_pair(old_val, dup_val)].insert(make_pair(
                          Locator(old_row_id, 0, old_tok.size()),
                          Locator(dup_row_id, 0, dup_tok.size())));
#else

                  termRules[make_pair(old_val, dup_val)] -= 1;
                  rule_locations[make_pair(old_val, dup_val)].erase(make_pair(
                          Locator(old_row_id, 0, old_tok.size()),
                          Locator(dup_row_id, 0, dup_tok.size())));
#endif
                }
              } else {
                if (termRules[make_pair(dup_val, old_val)] > 0) {

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                  to_delete_rules[make_pair(dup_val, old_val)].insert(make_pair(
                          Locator(dup_row_id, 0, dup_tok.size()),
                          Locator(old_row_id, 0, old_tok.size())));
#else
                  termRules[make_pair(dup_val, old_val)] -= 1;
                  rule_locations[make_pair(dup_val, old_val)].erase(make_pair(
                          Locator(dup_row_id, 0, dup_tok.size()),
                          Locator(old_row_id, 0, old_tok.size())));
#endif
                }
              }
            }

            if (rule_types != 5) {

              vector<pair<pair<int, int>, pair<int, int>>> old_diffs;
              Wrapper::Alignment<vector<string>>(old_tok, dup_tok, old_tok.size(), dup_tok.size(), old_diffs, false);

              for (auto &entry : old_diffs) {
                auto &stPos = entry.first;
                auto &enPos = entry.second;

                const string &old_piece = getPiece(old_tok, stPos.first, enPos.first);
                const string &dup_piece = getPiece(dup_tok, stPos.second, enPos.second);

                if (old_piece.empty() || dup_piece.empty())
                  if (rule_types != 3 && rule_types != 4)  // no deletion rule
                    continue;

                if (old_piece > dup_piece) {
#ifdef OUTPUT_INFO
                  cout << termRules[make_pair(old_piece, dup_piece)] << "|||";
                  if (termRules[make_pair(old_piece, dup_piece)] == 0)
                    cout << "1 didn't find" << endl
                         << old_row_id << ": " << old_piece << endl
                         << dup_row_id << ": " << dup_piece << endl;
#endif

                  if (termRules[make_pair(old_piece, dup_piece)] > 0) {

                    #ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                    to_delete_rules[make_pair(old_piece, dup_piece)].insert(make_pair(
                            Locator(old_row_id, stPos.first, enPos.first),
                            Locator(dup_row_id, stPos.second, enPos.second)));
#else
                    termRules[make_pair(old_piece, dup_piece)] -= 1;
                    rule_locations[make_pair(old_piece, dup_piece)].erase(make_pair(
                            Locator(old_row_id, stPos.first, enPos.first),
                            Locator(dup_row_id, stPos.second, enPos.second)));
#endif
                  }

#ifdef OUTPUT_INFO
                  cout << "DELETED: " << old_piece << " --- " << dup_piece << " ";
                  cout << "|||" << termRules[make_pair(old_piece, dup_piece)] << endl;
#endif
                } else if (old_piece < dup_piece) {

#ifdef OUTPUT_INFO
                  cout << termRules[make_pair(dup_piece, old_piece)] << "|||";
                  if (termRules[make_pair(dup_piece, old_piece)] == 0)
                    cout << "2 didn't find" << endl
                         << dup_row_id << ": " << dup_piece << endl
                         << old_row_id << ": " << old_piece << endl;
#endif

                  if (termRules[make_pair(dup_piece, old_piece)] > 0) {

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                    to_delete_rules[make_pair(dup_piece, old_piece)].insert(make_pair(
                            Locator(dup_row_id, stPos.second, enPos.second),
                            Locator(old_row_id, stPos.first, enPos.first)));
#else
                    termRules[make_pair(dup_piece, old_piece)] -= 1;
                    rule_locations[make_pair(dup_piece, old_piece)].erase(make_pair(
                            Locator(dup_row_id, stPos.second, enPos.second),
                            Locator(old_row_id, stPos.first, enPos.first)));
#endif
                  }

#ifdef OUTPUT_INFO
                  cout << "DELETED: " << dup_piece << " --- " << old_piece << "|||";
                  cout << "|||" << termRules[make_pair(dup_piece, old_piece)] << " " << endl;
#endif
                }
              }
            }
          }

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
          unordered_map<pair<string, string>, unordered_map<pair<Locator, Locator>, pair<int, int>, PairLocatorHash>, pair_hash> to_add_rules;
#endif

          if (new_val != dup_val && !new_val.empty() && !dup_val.empty())
          {

            int new_timestamp = modification[new_row_id].size();
            int dup_timestamp = modification[dup_row_id].size();

            if (rule_types == 2 || rule_types == 5 || rule_types == 4)
            {
              if (new_val > dup_val)
              {
#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                to_add_rules[make_pair(new_val, dup_val)].insert(make_pair(make_pair(
                        Locator(new_row_id, 0, new_tok.size()),
                        Locator(dup_row_id, 0, dup_tok.size())), make_pair(new_timestamp, dup_timestamp)));

#else
                if (termRules[make_pair(new_val, dup_val)] == 0)
                  new_rules.insert(make_pair(new_val, dup_val));

                termRules[make_pair(new_val, dup_val)] += 1;
                rule_locations[make_pair(new_val, dup_val)].insert(make_pair(make_pair(
                      Locator(new_row_id, 0, new_tok.size()),
                      Locator(dup_row_id, 0, dup_tok.size())), make_pair(new_timestamp, dup_timestamp)));
#endif
              }
              else if (new_val < dup_val)
              {
#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                to_add_rules[make_pair(dup_val, new_val)].insert(make_pair(make_pair(
                        Locator(dup_row_id, 0, dup_tok.size()),
                        Locator(new_row_id, 0, new_tok.size())), make_pair(dup_timestamp, new_timestamp)));

#else
                if (termRules[make_pair(dup_val, new_val)] == 0)
                  new_rules.insert(make_pair(dup_val, new_val));

                termRules[make_pair(dup_val, new_val)] += 1;
                rule_locations[make_pair(dup_val, new_val)].insert(make_pair(make_pair(
                      Locator(dup_row_id, 0, dup_tok.size()),
                      Locator(new_row_id, 0, new_tok.size())), make_pair(dup_timestamp, new_timestamp)));
#endif
              }
            }

            if (rule_types != 5)
            {
              vector<pair<pair<int, int>, pair<int, int>>> new_diffs;
              Wrapper::Alignment<vector<string>>(new_tok, dup_tok, new_tok.size(), dup_tok.size(), new_diffs, false);

              for (auto &entry : new_diffs)
              {
                auto &stPos = entry.first;
                auto &enPos = entry.second;

                const string & new_piece = getPiece(new_tok, stPos.first, enPos.first);
                const string & dup_piece = getPiece(dup_tok, stPos.second, enPos.second);

                if (new_piece.empty() || dup_piece.empty())
                  if (rule_types != 3 && rule_types != 4)  // no deletion rule
                    continue;

                if (new_piece > dup_piece)
                {
#ifdef OUTPUT_INFO
                  cout << termRules[make_pair(new_piece, dup_piece)] << "|||";
#endif
#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                  to_add_rules[make_pair(new_piece, dup_piece)].insert(make_pair(make_pair(
                          Locator(new_row_id, stPos.first, enPos.first),
                          Locator(dup_row_id, stPos.second, enPos.second)), make_pair(new_timestamp, dup_timestamp)));
#else
                  if (termRules[make_pair(new_piece, dup_piece)] == 0)
                    new_rules.insert(make_pair(new_piece, dup_piece));

                  termRules[make_pair(new_piece, dup_piece)] += 1;
                  rule_locations[make_pair(new_piece, dup_piece)].insert(make_pair(make_pair(
                        Locator(new_row_id, stPos.first, enPos.first),
                        Locator(dup_row_id, stPos.second, enPos.second)), make_pair(new_timestamp, dup_timestamp)));
#endif
#ifdef OUTPUT_INFO
                  cout << "ADDED: " << new_piece << " --- " << dup_piece << " ";
                  cout << "|||" << termRules[make_pair(new_piece, dup_piece)] << " " << endl;
#endif
                }
                else if (new_piece < dup_piece)
                {
#ifdef OUTPUT_INFO
                  cout << termRules[make_pair(dup_piece, new_piece)] << "|||";
#endif
#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
                  to_add_rules[make_pair(dup_piece, new_piece)].insert(make_pair(make_pair(
                          Locator(dup_row_id, stPos.second, enPos.second),
                          Locator(new_row_id, stPos.first, enPos.first)), make_pair(dup_timestamp, new_timestamp)));
#else
                  if (termRules[make_pair(dup_piece, new_piece)] == 0)
                    new_rules.insert(make_pair(dup_piece, new_piece));

                  termRules[make_pair(dup_piece, new_piece)] += 1;
                  rule_locations[make_pair(dup_piece, new_piece)].insert(make_pair(make_pair(
                        Locator(dup_row_id, stPos.second, enPos.second),
                        Locator(new_row_id, stPos.first, enPos.first)), make_pair(dup_timestamp, new_timestamp)));
#endif
#ifdef OUTPUT_INFO
                  cout << "ADDED: " << dup_piece << " --- " << new_piece << " ";
                  cout << "|||" << termRules[make_pair(dup_piece, new_piece)] << " " << endl;
#endif
                }
              }
            }
          }

#ifdef INCREMENTAL_UPDATE_RULE_ENABLE
//          cout << "BEFORE TO ADD: " << to_add_rules.size() << endl;
//          cout << "BEFORE TO DEL: " << to_delete_rules.size() << endl;
          vector<pair<string, string>> common_update_rules;
          for (auto &to_delete : to_delete_rules)
          {
            auto iter = to_add_rules.find(to_delete.first);
            if (iter != to_add_rules.end() && iter->second.size() == to_delete.second.size())
              common_update_rules.push_back(to_delete.first);
          }
          for (auto &to_remove : common_update_rules)
          {
            to_delete_rules.erase(to_remove);
            to_add_rules.erase(to_remove);
          }
//          cout << "after TO ADD: " << to_add_rules.size() << endl;
//          cout << "after TO DEL: " << to_delete_rules.size() << endl;
          for (auto &to_delete : to_delete_rules)
          {
            for (auto &to_delete_loc : to_delete.second)
            {
              termRules[to_delete.first] -= 1;
              rule_locations[to_delete.first].erase(to_delete_loc);
            }
          }
          for (auto &to_add : to_add_rules)
          {
            for (auto to_add_loc : to_add.second)
            {
              if (termRules[to_add.first] == 0)
                new_rules.insert(to_add.first);

              termRules[to_add.first] += 1;
              rule_locations[to_add.first].insert(to_add_loc);
            }
          }
//          cout << "new RULES: " << new_rules.size() << endl;
#endif
        }



        unordered_map<pair<string, string>, vector<int>, pair_hash> structure_groups;
        for (auto group_id = 0; group_id < structures.size(); group_id++)
          structure_groups[structures[group_id]].push_back(group_id);


        for (auto &entry : new_rules)
        {
          int latest_rule_id = rules.size();
          rules.push_back(make_pair(entry, 0));

          string input = entry.first;
          string output = entry.second;
          for (int j = 0; j < AGG_REGEX_NUM; j++)
          {
            input = Wrapper::FindReplace(input, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
            output = Wrapper::FindReplace(output, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
          }

          bool isReverse = false;
          string lhs = input;
          string rhs = output;

          if (input == output)
          {
            if (entry.first.length() < entry.second.length())
            {
              string tmp = lhs;
              lhs = rhs;
              rhs = tmp;
              isReverse = true;
            }
            else if (entry.first.length() == entry.second.length())
            {
              if (entry.first < entry.second)
              {
                string tmp = lhs;
                lhs = rhs;
                rhs = tmp;
                isReverse = true;
              }
            }
          }
          else
          {
            if (structure_groups.count(make_pair(lhs, rhs)))
            {
              // cout << "do nothing" << endl;
            }
            else if (structure_groups.count(make_pair(rhs, lhs)))
            {
              string tmp = lhs;
              lhs = rhs;
              rhs = tmp;
              isReverse = true;
            }
            else if (entry.first.length() < entry.second.length())
            {
              string tmp = lhs;
              lhs = rhs;
              rhs = tmp;
              isReverse = true;
            }
            else if (entry.first.length() == entry.second.length())
            {
              if (entry.first < entry.second)
              {
                string tmp = lhs;
                lhs = rhs;
                rhs = tmp;
                isReverse = true;
              }
            }
          }


          if (!structure_groups.count(make_pair(lhs, rhs)))
          {
            // self to be a group with no transformation
            int latest_group_id = groupRules.size();
            ruleDir.push_back(isReverse);
            ruleGroup.push_back(latest_group_id);
            ruleStatus.push_back(0);
            groupRules.emplace_back(1, latest_rule_id);
            groupStatus.push_back(0);
            structures.emplace_back(lhs, rhs);
            transformations.push_back(Path());
          }
          else
          {
            // test transformation
            bool has_founded = false;

            string source = entry.first;
            string target = entry.second;
            if (isReverse)
            {
              source = entry.second;
              target = entry.first;
            }
            Graph source_graph;
            source_graph.GenGraphStr(source, -1);

            for (auto gid : structure_groups[make_pair(lhs, rhs)])
            {
              auto &trans = transformations[gid];
              bool isContain = source_graph.TestContainment(trans.path, target, source);
              if (isContain)
              {
                has_founded = true;
                ruleDir.push_back(isReverse);
                ruleGroup.push_back(gid);
                ruleStatus.push_back(0);
                groupRules[gid].push_back(latest_rule_id);
                break;
              }
            }
            if (!has_founded)
            {
              int latest_group_id = groupRules.size();
              ruleDir.push_back(isReverse);
              ruleGroup.push_back(latest_group_id);
              ruleStatus.push_back(0);
              groupRules.emplace_back(1, latest_rule_id);
              groupStatus.push_back(0);
              structures.emplace_back(lhs, rhs);
              transformations.push_back(Path());
            }
          }
        }

        int num_rules = 0;
        for (auto &entry : rules)
        {
          if (termRules.count(entry.first))
            entry.second = termRules[entry.first];

          if (entry.second > 0)
            num_rules++;

#ifdef OUTPUT_INFO
          if (entry.second < 0)
            cout << "this is impossible!!!!" << endl;
          assert(entry.second >= 0);
#endif
        }
#ifdef OUTPUT_INFO
        cout << "number of rules after update: " << num_rules << endl;
#endif
        values[row_id] = new_value;
        toks = new_tok;
      }
    }
  } else if (tmp == "4") {
    cout << "exit" << endl;
    return false;
  } else {
    cout << "invalid option" << endl;
  }
  return true;
}
