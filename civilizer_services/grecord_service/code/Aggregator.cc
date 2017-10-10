//
// Created by dongdeng on 6/26/17.
//

#include "Aggregator.h"
#include "Synthesize.h"




void Aggregator::RandomGroup()
{
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  srand(time(NULL));

  // replacing tokens to marks
  int i = 1;
  unordered_map<pair<string, string>, vector<int>, pair_hash> clusters;
  for (auto &entry : rules)
  {
    string input = entry.first.first;
    string output = entry.first.second;
    for (int j = 0; j < AGG_REGEX_NUM; j++)
    {
      input = Wrapper::FindReplace(input, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
      output = Wrapper::FindReplace(output, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
    }

#ifdef LONG_FIRST_MAPPING_ENABLE
    if (entry.first.first.length() > entry.first.second.length())
      clusters[make_pair(input, output)].push_back(i);
    else if (entry.first.first.length() < entry.first.second.length())
      clusters[make_pair(output, input)].push_back(0 - i);
    else if (entry.first.first > entry.first.second)
      clusters[make_pair(input, output)].push_back(i);
    else if (entry.first.first < entry.first.second)
      clusters[make_pair(output, input)].push_back(0 - i);
#else
    clusters[make_pair(input, output)].push_back(i);
#endif
    /* int isRev = rand() % 2; // 0 for not reverse, 1 for reverse
    if (isRev == 0)
      clusters[make_pair(input, output)].push_back(i);
    else
      clusters[make_pair(output, input)].push_back(0 - i); */


    ++i;
  }

#ifdef OUTPUT_INFO
  Wrapper::logTime(t1, t2, "Building Clusters Finished.");
  cout << "no of clusters: " << clusters.size() << endl;
#endif

  // fill in and sort ordered ---- clusters in order
  // ordered stored the final clusters based on structures which are sorted in freq order
  ordered.clear();
  for (auto &entry : clusters)
  {
    int cnt = 0;
    for (auto id : entry.second)
    {
      if (id > 0)
        cnt += rules[id - 1].second;
      else
        cnt += rules[0 - 1 - id].second;
    }
    ordered.emplace_back(entry.first, cnt);
  }
  sort(ordered.begin(), ordered.end(), [](const pair<pair<string, string>, int> &v1, const pair<pair<string, string>, int> &v2){
    return v1.second > v2.second; });

  cluster_sizes.clear();
  forward_list.assign(rules.size(), make_pair(-1, -1));
  for (auto id = 0; id < ordered.size(); id++)
  {
    cluster_sizes.push_back(clusters[ordered[id].first].size());
    for (auto rid : clusters[ordered[id].first])
    {
      if (rid > 0)
        forward_list[rid - 1].first = id;
      else
        forward_list[0 - 1 - rid].second = id;
    }
  }

  int cluster_id = 0;
  format_group_rule_ids.resize(ordered.size());
  for (auto p : ordered)
  {
    vector<pair<pair<string, string>, int>> current_group;

#ifdef OUTPUT_INFO
    cout << "|" << p.first.first << "|---->|" << p.first.second << "|  " << p.second << endl;
#endif

    for (auto s : clusters[p.first])
    {
      if (s > 0)
      {
        current_group.push_back(rules[s - 1]);
        format_group_rule_ids[cluster_id].push_back(s - 1);
#ifdef OUTPUT_INFO
        cout << s << ": [" << rules[s - 1].first.first << "]--->[" << rules[s - 1].first.second << "]" << endl;
#endif
      }
      else
      {
        current_group.emplace_back(make_pair(rules[0 - 1 - s].first.second, rules[0 - 1 - s].first.first), rules[0 - 1 - s].second);
        format_group_rule_ids[cluster_id].push_back(0 - 1 - s);
#ifdef OUTPUT_INFO
        cout << s << ": [" << rules[0 - 1 - s].first.second << "]--->[" << rules[0 - 1 - s].first.first << "]" << endl;
#endif
      }
    }
#ifdef OUTPUT_INFO
    cout << "--------------------------------------" << endl;
#endif
    format_group_cluster_ids.push_back(cluster_id++);
    format_group_rules.push_back(current_group);
  }
  cout << "Number of Rules: " << forward_list.size() << endl;
  cout << "Number of Structure Groups: " << ordered.size() << endl;

#ifdef OUTPUT_INFO
  Wrapper::logTime(t2, t3, " Filling Groups etc.");
#endif
}


void Aggregator::Group()
{

#ifdef RANDOM_MAPPING_DIR_ENABLE
  RandomGroup();
  return;
#endif

  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  // replacing tokens to marks
  int i = 1;
  unordered_map<pair<string, string>, vector<int>, pair_hash> clusters;
  for (auto &entry : rules)
  {
    string input = entry.first.first;
    string output = entry.first.second;
    for (int j = 0; j < AGG_REGEX_NUM; j++)
    {
      input = Wrapper::FindReplace(input, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
      output = Wrapper::FindReplace(output, Wrapper::agg_regexes[j], Wrapper::agg_replace_str[j]);
    }
    
    // if the lhs and rhs have the same structure, then we use the longer one as the left hand side
    // an id smaller than 0 means it is reversed
#ifdef REVERSE_MAPPING_DIR_ENABLE
    if (input == output)
    {
      if (entry.first.first.length() < entry.first.second.length())
        clusters[make_pair(input, output)].push_back(i);
      else if (entry.first.first.length() > entry.first.second.length())
        clusters[make_pair(output, input)].push_back(0 - i);
      else if (entry.first.first < entry.first.second)
        clusters[make_pair(input, output)].push_back(i);
      else if (entry.first.first > entry.first.second)
        clusters[make_pair(output, input)].push_back(0 - i);
      else
      {
        clusters[make_pair(input, output)].push_back(i);
        cout << "WARNING: equavelent rules: " << entry.first.first << endl << entry.first.second << endl;
      }
    }
#else
    if (input == output)
    {
      if (entry.first.first.length() > entry.first.second.length())
        clusters[make_pair(input, output)].push_back(i);
      else if (entry.first.first.length() < entry.first.second.length())
        clusters[make_pair(output, input)].push_back(0 - i);
      else if (entry.first.first > entry.first.second)
        clusters[make_pair(input, output)].push_back(i);
      else if (entry.first.first < entry.first.second)
        clusters[make_pair(output, input)].push_back(0 - i);
      else
      {
        clusters[make_pair(input, output)].push_back(i);
        cout << "WARNING: equavelent rules: " << entry.first.first << endl << entry.first.second << endl;
      }
    }
#endif
    else
    {
      clusters[make_pair(input, output)].push_back(i);
      clusters[make_pair(output, input)].push_back(0 - i);
    }
    ++i;
  }

#ifdef OUTPUT_INFO
  Wrapper::logTime(t1, t2, "Building Clusters Finished.");
  cout << "no of clusters before deduplicate: " << clusters.size() << endl;
#endif

  // mirrors
  // get the list of duplicate (mirror) structure pairs
  vector<pair<string, string>> duplicate_rules;
  for (auto &entry : clusters)
    if (entry.first.first > entry.first.second)
      duplicate_rules.emplace_back(entry.first);

#ifdef OUTPUT_INFO
  cout << "no of duplicates: " << duplicate_rules.size() << endl;
#endif

  // remove duplicate clusters
  for (auto &entry : duplicate_rules)
  {
    long l_avglen = 0;
    long r_avglen = 0;
    for (auto id : clusters[entry])
    {
      if (id > 0)
      {
        int rid = id - 1;
        l_avglen += rules[rid].first.first.length();
        r_avglen += rules[rid].first.second.length();
      } else {
        int rid = 0 - 1 - id;
        l_avglen += rules[rid].first.second.length();
        r_avglen += rules[rid].first.first.length();
      }
    }
    // if lhs is shorter, then remove current cluster; otherwise, remove the mirrored one

#ifdef REVERSE_MAPPING_DIR_ENABLE
    if (l_avglen >= r_avglen)
      clusters.erase(entry);
    else
      clusters.erase(make_pair(entry.second, entry.first));
#else
    if (l_avglen < r_avglen)
      clusters.erase(entry);
    else
      clusters.erase(make_pair(entry.second, entry.first));
#endif
  }

#ifdef OUTPUT_INFO
  cout << "No of Structural Groups After Deduplicate: " << clusters.size() << endl;
#endif

  // fill in and sort ordered ---- clusters in order 
  // ordered stored the final clusters based on structures which are sorted in freq order
  ordered.clear();
  for (auto &entry : clusters)
  {
    int cnt = 0;
    for (auto id : entry.second)
    {
      if (id > 0)
        cnt += rules[id - 1].second;
      else
        cnt += rules[0 - 1 - id].second;
    }
    ordered.emplace_back(entry.first, cnt);
  }
  sort(ordered.begin(), ordered.end(), [](const pair<pair<string, string>, int> &v1, const pair<pair<string, string>, int> &v2){
    return v1.second > v2.second; });

  // fill in forward list
  // forward list stores a map from rule id to cluster id in ordered, if the rule is reversed, the cluster id is stored in the second slot
  // cluster_sizes stores the sizes of the clusters in ordered
  cluster_sizes.clear();
  forward_list.assign(rules.size(), make_pair(-1, -1));
  for (auto id = 0; id < ordered.size(); id++)
  {
    cluster_sizes.push_back(clusters[ordered[id].first].size());
    for (auto rid : clusters[ordered[id].first])
    {
      if (rid > 0)
        forward_list[rid - 1].first = id;
      else
        forward_list[0 - 1 - rid].second = id;
    }
  }

#ifdef OUTPUT_INFO
  for (auto &entry : forward_list)
  {
    assert(entry.first != -1 || entry.second != -1);
    assert(entry.first == -1 || entry.second == -1);
  }
#endif

  // fill in groups
  // group the rules based on their structures for later regroup by transformation
  int cluster_id = 0;
  format_group_rule_ids.resize(ordered.size());
  for (auto p : ordered)
  {
    vector<pair<pair<string, string>, int>> current_group;

#ifdef OUTPUT_INFO
    cout << "|" << p.first.first << "|---->|" << p.first.second << "|  " << p.second << endl;
#endif

    for (auto s : clusters[p.first])
    {
      if (s > 0)
      {
        current_group.push_back(rules[s - 1]);
        format_group_rule_ids[cluster_id].push_back(s - 1);
#ifdef OUTPUT_INFO
        cout << s << ": [" << rules[s - 1].first.first << "]--->[" << rules[s - 1].first.second << "]" << endl;
#endif
      }
      else
      {
        current_group.emplace_back(make_pair(rules[0 - 1 - s].first.second, rules[0 - 1 - s].first.first), rules[0 - 1 - s].second);
        format_group_rule_ids[cluster_id].push_back(0 - 1 - s);
#ifdef OUTPUT_INFO
        cout << s << ": [" << rules[0 - 1 - s].first.second << "]--->[" << rules[0 - 1 - s].first.first << "]" << endl;
#endif
      }
    }
#ifdef OUTPUT_INFO
    cout << "--------------------------------------" << endl;
#endif
    format_group_cluster_ids.push_back(cluster_id++);
    format_group_rules.push_back(current_group);
  }
  cout << "Number of Rules: " << forward_list.size() << endl;
  cout << "Number of Structure Groups: " << ordered.size() << endl;

#ifdef OUTPUT_INFO
  Wrapper::logTime(t2, t3, " Filling Groups etc.");
#endif
}

void Aggregator::CalConstantTerms()
{
#ifdef OUTPUT_INFO
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);
#endif

  int rid = 0;
  // string, cluster_id, count/freq
  unordered_map<string, unordered_map<int, int>> freq_list;
  for (auto &entry : rules)
  {
    const string &lhs = entry.first.first;
    const string &rhs = entry.first.second;

    int cluster_id = forward_list[rid].first;
    if (cluster_id == -1)
      cluster_id = forward_list[rid].second;

    unordered_set<string> distinct_substrings;
    for (auto i = 0; i < lhs.length(); i++)
      for (auto j = i; j < lhs.length(); j++)
        if (j - i + 1 < MAX_CONSTANT_LENGTH)
          distinct_substrings.insert(lhs.substr(i, j - i + 1));

    for (auto i = 0; i < rhs.length(); i++)
      for (auto j = i; j < rhs.length(); j++)
        if (j - i + 1 < MAX_CONSTANT_LENGTH)
          distinct_substrings.insert(rhs.substr(i, j - i + 1));

    for (auto &str : distinct_substrings)
      freq_list[str][cluster_id] += 1;

    rid++;
  }

#ifdef OUTPUT_INFO
  Wrapper::logTime(t1, t2, "finished building frequency lists");
#endif

  local_const_terms.clear();
  global_const_terms.clear();

  unordered_map<string, int> all_total_num;
  for (auto &entry : freq_list)
  {
    int total_num = 0;
    int max_num = 0;
    for (auto &cid_cnt : entry.second)
    {
      total_num += cid_cnt.second;
      if (cid_cnt.second > max_num)
        max_num = cid_cnt.second;
    }
    all_total_num[entry.first] = total_num;

    if (total_num < GLOBAL_FREQUENCY_THRESHOLD)  continue;

    double ratio = max_num * 1.0 / sqrt(total_num);
    // double ratio = max_num * 1.0 / total_num;

    global_const_terms[entry.first] = make_pair(ratio, total_num);
  }

#ifdef OUTPUT_INFO
  Wrapper::logTime(t2, t3, "finished building global terms.");
#endif

  int cluster_num = ordered.size();
  local_const_terms.resize(cluster_num);
  for (auto &entry : freq_list)
  {
    int total_freq = all_total_num[entry.first];
    for (auto &cid_cnt : entry.second)
    {
      // cid_cnt.second === frequency within the cluster cid_cnt.first
      // clusetr_sizes[cid_cnt.first] ==== cluster size
      // thus below is the ratio of freq within the cluster
      if (cid_cnt.second * 1.0 / cluster_sizes[cid_cnt.first] >= LOCAL_FREQUENCY_THRESHOLD)
      {
        // a substring within a cluster ===== assigned score, which is freq within the cluster divided by total_freq
        // local_const_terms[cid_cnt.first][entry.first] = make_pair(cid_cnt.second * 1.0 / cluster_sizes[cid_cnt.first], total_freq);
        local_const_terms[cid_cnt.first][entry.first] = make_pair(cid_cnt.second * 1.0 / sqrt(total_freq), total_freq);
      }
    }
  }

#ifdef OUTPUT_INFO
  Wrapper::logTime(t3, t4, "finished building local terms.");
#endif
}

void Aggregator::GroupAggregate()
{
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  Group();

  Wrapper::logTime(t1, t2, "   ==> Grouping by Structure Time etc: ");

  CalConstantTerms();

  Wrapper::logTime(t2, t3, "   ==> Calculating Constant Terms Time: ");


  int idx = 0;
  ruleGroup.assign(rules.size(), -1);
  ruleStatus.assign(rules.size(), 0);
  ruleDir.assign(rules.size(), false);

  for (auto &current_group : format_group_rules)
  {
    unordered_map<Path, vector<int>, PathHash> transform_group_rules;  // transformation, rules
    int cluster_id = format_group_cluster_ids[idx++];

    Synthesizer synsizer(current_group, local_const_terms[cluster_id]);
    synsizer.SynAggregating(transform_group_rules, format_group_rule_ids[cluster_id]);

    int num_groups = transform_group_rules.size();
    for (auto &entry : transform_group_rules)
    {
      int group_id = transformations.size();
      transformations.push_back(entry.first);
      groupRules.emplace_back(entry.second);
      int structure_id = -1;
      for (auto rid : entry.second)
      {
        ruleGroup[rid] = group_id;
        structure_id = forward_list[rid].first;
        if (forward_list[rid].first == -1)
        {
          ruleDir[rid] = true;
          structure_id = forward_list[rid].second;
        }
      }
      structures.push_back(ordered[structure_id].first);
    }
  }
  Wrapper::logTime(t3, t4, "   ==> Grouping by Transformation Time: ");
  cout << "Number of Groups: " << groupRules.size() << endl;
}


void Aggregator::Aggregate()
{
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  Group();

  Wrapper::logTime(t1, t2, "   ==> Grouping by Structure Time etc: ");

  CalConstantTerms();

  Wrapper::logTime(t2, t3, "   ==> Calculating Constant Terms Time: ");

  vector<int> rule_ids;
  vector<pair<pair<string, string>, int>> current_group;
  for (auto i = 0; i < format_group_rules.size(); i++)
    for (auto j = 0; j < format_group_rules[i].size(); j++)
    {
      current_group.push_back(format_group_rules[i][j]);
      rule_ids.push_back(format_group_rule_ids[i][j]);
    }

#ifdef OUTPUT_INFO
  assert(rule_ids.size() == rules.size());
#endif

  ruleGroup.assign(rules.size(), -1);
  ruleStatus.assign(rules.size(), 0);
  ruleDir.assign(rules.size(), false);

  unordered_map<Path, vector<int>, PathHash> transform_group_rules;  // transformation, rules

  Synthesizer synsizer(current_group, global_const_terms);
  synsizer.SynAggregating(transform_group_rules, rule_ids);


  int num_groups = transform_group_rules.size();
  for (auto &entry : transform_group_rules)
  {

#ifdef OUTPUT_INFO
    cout << "group: ------" << endl;
    for (auto &e : entry.first.path)
      cout << e.first << " --- " << e.second << endl;
    cout << endl << endl;
#endif

    int group_id = transformations.size();
    transformations.push_back(entry.first);
    groupRules.emplace_back(entry.second);
    // int structure_id = -1;
    for (auto rid : entry.second)
    {
      ruleGroup[rid] = group_id;
      // structure_id = forward_list[rid].first;
      if (forward_list[rid].first == -1)
      {
        ruleDir[rid] = true;
        // structure_id = forward_list[rid].second;
#ifdef OUTPUT_INFO
        cout << rid << ": " << rules[rid].first.second << "-->" << rules[rid].first.first << "|  " << rules[rid].second << endl;
#endif
      }
#ifdef OUTPUT_INFO
      else
        cout << rid << ": " << rules[rid].first.first << "-->" << rules[rid].first.second << "|" << rules[rid].second << endl;
#endif
    }
    // structures.push_back(ordered[structure_id].first);

#ifdef OUTPUT_INFO
    cout << "------------" << endl;
#endif
  }

  Wrapper::logTime(t3, t4, "   ==> Grouping by Transformation Time: ");
  cout << "Number of Groups: " << groupRules.size() << endl;
}




void Aggregator::AggregateStructure()
{
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  Group();

  Wrapper::logTime(t1, t2, "   ==> Grouping by Structure Time etc: ");

  // Do not need to calculate constant terms

  Wrapper::logTime(t2, t3, "   ==> Calculating Constant Terms Time: ");


  ruleGroup.assign(rules.size(), -1);
  ruleStatus.assign(rules.size(), 0);
  ruleDir.assign(rules.size(), false);

  Path uniqu_path;
  uniqu_path.path.emplace_back(Pos(Regex::CAPS, 10000, "NO_GROUPING_BY_TRANSFORMATION", false), Pos(Regex::CAPS, 10000, "NO_GROUPING_BY_TRANSFORMATION", false));

  for (auto i = 0; i < format_group_rules.size(); i++)
  {
    uniqu_path.path.front().first.label.index += 1;

    vector<pair<Path, vector<int>>> transform_group_rules;
    transform_group_rules.emplace_back(uniqu_path, vector<int>());
    for (auto j = 0; j < format_group_rules[i].size(); j++)
      transform_group_rules.back().second.push_back(format_group_rule_ids[i][j]);

    for (auto &entry : transform_group_rules)
    {
      int group_id = transformations.size();
      transformations.push_back(entry.first);
      groupRules.emplace_back(entry.second);
      int structure_id = -1;
      for (auto rid : entry.second)
      {
        ruleGroup[rid] = group_id;
        structure_id = forward_list[rid].first;
        if (forward_list[rid].first == -1)
        {
          ruleDir[rid] = true;
          structure_id = forward_list[rid].second;
        }
      }
      structures.push_back(ordered[structure_id].first);
    }
  }
  Wrapper::logTime(t3, t4, "   ==> Grouping by Transformation Time: ");
  cout << "Number of Groups: " << groupRules.size() << endl;
}





void Aggregator::NoAggregatation()
{
  timeval t1, t2, t3, t4;
  gettimeofday(&t1, NULL);

  Group();

  Wrapper::logTime(t1, t2, "   ==> Grouping by Structure Time etc: ");

  // Do not need to calculate constant terms

  Wrapper::logTime(t2, t3, "   ==> Calculating Constant Terms Time: ");


  ruleGroup.assign(rules.size(), -1);
  ruleStatus.assign(rules.size(), 0);
  ruleDir.assign(rules.size(), false);

  Path uniqu_path;
  uniqu_path.path.emplace_back(Pos(Regex::CAPS, 10000, "NO_GROUPING_BY_TRANSFORMATION", false), Pos(Regex::CAPS, 10000, "NO_GROUPING_BY_TRANSFORMATION", false));

  for (auto i = 0; i < format_group_rules.size(); i++)
    for (auto j = 0; j < format_group_rules[i].size(); j++)
    {
      uniqu_path.path.front().first.label.index += 1;
      pair<Path, vector<int>> transform_group_rules;
      transform_group_rules.first = uniqu_path;
      transform_group_rules.second.push_back(format_group_rule_ids[i][j]);
      int group_id = transformations.size();
      transformations.push_back(transform_group_rules.first);
      groupRules.emplace_back(transform_group_rules.second);
      for (auto rid : transform_group_rules.second)
      {
        ruleGroup[rid] = group_id;
        if (forward_list[rid].first == -1)
          ruleDir[rid] = true;
      }
    }

  Wrapper::logTime(t3, t4, "   ==> Grouping by Transformation Time: ");
  cout << "Number of Groups: " << groupRules.size() << endl;
}
