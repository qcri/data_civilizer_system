
#include "Consolidation.h"

bool GetClusters(int &cluster_id_col, const Table &table, vector<vector<int>> &clusters, string &cluster_id_name)
{
  unordered_map<string, int> cluster_id_map;

  bool found = false;
  for (int j = 0; j < table.schema.size(); j++)
  {
    auto &attr = table.schema[j];
    cout << "where is the CID"<< attr << "=="<< cluster_id_name<< endl;
    if (attr == cluster_id_name)
    {
      cluster_id_col = j;
      found = true;
    }
  }

  if (!found)
  {
    cout << "didn't find cluster id column!" << endl;
    return false;
  }

  for (int row_id = 0; row_id < table.rows.size(); row_id++)
  {
    auto &row = table.rows[row_id];
    if (!cluster_id_map.count(row[cluster_id_col])) {
      cluster_id_map[row[cluster_id_col]] = clusters.size();
      clusters.emplace_back(1, row_id);
    } else {
      clusters[cluster_id_map[row[cluster_id_col]]].push_back(row_id);
    }
  }

  map<int, int> cluster_size;
  for (auto &entry : clusters)
    cluster_size[entry.size()] += 1;
#ifdef OUTPUT_INFO
  cerr << "before deduplicate." << endl;
  for (auto &xid : cluster_size)
    cerr << xid.first << " " << xid.second << endl;
#endif
  return true;
}

void ShowDiffClusters(int column_id, const Table &table, const vector<vector<int>> &clusters)
{
  map<int, int> cluster_sizes;
  for (auto i = 0; i < clusters.size(); i++)
  {
    auto &vec = clusters[i];
    unordered_map<string, int> uniq_vals;
    uniq_vals[""] = 0;
    for (auto rid : vec)
      uniq_vals[table.rows[rid][column_id]] += 1;
    cluster_sizes[uniq_vals.size()] += 1;

    if (uniq_vals.size() > 2)
    {
      Wrapper::print_green("cluster " + to_string(i) + ":");
      cout << endl;
      for (auto &str : uniq_vals)
      {
        Wrapper::print_green(str.first + " x" + to_string(str.second));
        cout << endl;
      }
      cout << endl;
    }
  }
}

string CountClusters(int column_id, const Table &table, const vector<vector<int>> &clusters)
{
  ostringstream buffer;
  long total_diff = 0;
  map<int, int> cluster_sizes;
  for (auto i = 0; i < clusters.size(); i++)
  {
    auto &vec = clusters[i];
    unordered_set<string> uniq_vals;
    uniq_vals.insert("");
    for (auto rid : vec)
      uniq_vals.insert(table.rows[rid][column_id]);
    cluster_sizes[uniq_vals.size()] += 1;
  }
  for (auto &entry : cluster_sizes)
  {
    total_diff += entry.second * (entry.first - 1) * (entry.first - 2) / 2;
    buffer << entry.first << "[" << entry.second << "], ";
  }
  buffer << endl << "number of duplicates: " << total_diff << endl;
  return buffer.str();
}

void PrintRules(int &print_rule_id, const vector<pair<pair<string, string>, int>> &rules)
{
  for (auto &entry : rules)
    Wrapper::print_green("Rule " + to_string(print_rule_id++) + " [" + to_string(entry.second) + "]: " + entry.first.first + "-->" + entry.first.second + "\n");
}

Consolidation::Consolidation(string filepath, string cname)
{
  cout << "GLOBAL_FREQUENCY_THRESHOLD set to " << GLOBAL_FREQUENCY_THRESHOLD << endl;
  cout << "LOCAL_FREQUENCY_THRESHOLD set to " << LOCAL_FREQUENCY_THRESHOLD << endl;
  cout << "MAX_NUMBER_OF_RULES set to " << MAX_NUMBER_OF_RULES << endl;
  cout << "MAX_PATH_LENGTH set to " << MAX_PATH_LENGTH << endl;
  cout << "PRUNE_BY_STRUCTURE_THRESHOLD set to " << PRUNE_BY_STRUCTURE_THRESHOLD << endl;

#ifdef RANK_BY_STRUCTURE_ENABLE
  cout << "~> RANK_BY_STRUCTURE_ENABLE." << endl;
#endif
#ifdef PRUNE_BY_STRUCTURE_ENABLE
  cout << "~> PRUNE_BY_STRUCTURE_ENABLE." << endl;
#endif
#ifdef REVERSE_MAPPING_DIR_ENABLE   // only use 0-1 of the 2 MACROs on the left
  cout << "~> REVERSE_MAPPING_DIR_ENABLE." << endl;
#endif
#ifdef RANDOM_MAPPING_DIR_ENABLE
  cout << "~> RANDOM_MAPPING_DIR_ENABLE." << endl;
#endif
#ifdef LONG_FIRST_MAPPING_ENABLE
  cout << "~> LONG_FIRST_MAPPING_ENABLE." << endl;
#endif
#ifdef NO_AGGREGATION_ENABLE
  cout << "~> NO_AGGREGATION_ENABLE." << endl;
#endif
#ifdef STRUCTURE_AGGREGATION_ENABLE
  cout << "~> STRUCTURE_AGGREGATION_ENABLE." << endl;
#endif
#ifdef BOTH_AGGREGATION_ENABLE
  cout << "~> BOTH_AGGREGATION_ENABLE." << endl;
#endif
#ifdef TRANSFORM_AGGREGATION_ENABLE
  cout << "~> TRANSFORM_AGGREGATION_ENABLE." << endl;
#endif
#ifdef LOCAL_THRESHOLD_ENABLE   // can use any of them, no restrict
  cout << "~> LOCAL_THRESHOLD_ENABLE." << endl;
#endif
#ifdef GLOBAL_THRESHOLD_ENABLE
  cout << "~> GLOBAL_THRESHOLD_ENABLE." << endl;
#endif
#ifdef UNIQUE_THRESHOLD_ENABLE
  cout << "~> UNIQUE_THRESHOLD_ENABLE." << endl;
#endif
#ifdef RULE_UPDATE_ENABLE
  cout << "~> RULE_UPDATE_ENABLE." << endl;
#endif
#ifdef RULE_CONTAINMENT_ENABLE   // TODO THIS ONE
  cout << "~> RULE_CONTAINMENT_ENABLE." << endl;
#endif
#ifdef STATIC_ORDERING_ENABLE   // do not DEFINE STATIC_ORDERING_ENABLE while undefine  SINGLE_CONSTANT_TERM_ENABLE
  cout << "~> STATIC_ORDERING_ENABLE." << endl;
#endif
#ifdef SINGLE_CONSTANT_TERM_ENABLE
  cout << "~> SINGLE_CONSTANT_TERM_ENABLE." << endl;
#endif
#ifdef PREFIX_SUFFIX_TERM_ENABLE
  cout << "~> PREFIX_SUFFIX_TERM_ENABLE." << endl;
#endif


  csvfilepath = filepath;
  cout << " filepath: "<<filepath<<endl;
  
  cluster_id_name = cname;

  bool normalize = true;

  csvreader = new CSVReader();
  csvreader->reading(csvfilepath, normalize);
  number_of_tables = csvreader->tables.size();

  Wrapper::maxLength = csvreader->get_max_val_len();
}

// i: the i-th table
// going to return the num of cols (as a string)
string Consolidation::TryNextTable(int i)
{
  timeval tbeg, tend;
  gettimeofday(&tbeg, NULL);
  
  ostringstream buffer;
  Table &table = csvreader->tables[i];

  buffer << " number of rows: " << table.rows.size() << endl;
  buffer << " the schema: ";
  for (auto &attr : table.schema) buffer << attr << "; ";
  buffer << endl;

  clusters.clear();

  cout << buffer.str() ;

  bool found = GetClusters(cluster_id_col, table, clusters, cluster_id_name);
  if (!found) return "0";

  return to_string(table.schema.size());
}

// just return some message
string Consolidation::ProfileColumn(int i, int col_id)
{
  ostringstream buffer;
  Table &table = csvreader->tables[i];

  buffer << "looking for rules for table " << table.table_name << " at column " << col_id << endl;
  // buffer << "number of clusters " << clusters.size() << endl << "avg size " << table.row_no * 1.0 / clusters.size() << endl;
  buffer << "column: " << table.schema[col_id] << endl;
  buffer << "number of distinct values: " << table.cols[col_id].size() << endl;
  buffer << "option: 0 (default): skip,  " << endl
         << "1: replacing rule only, " << endl
         << "2: replacing rules + full rules, " << endl
         << "3: replacing rules + deletion rules, " << endl
         << "4: replacing rules + deletion rules + full rules," << endl
         << "5: full rules only" << endl;

  return buffer.str();
}

// get input "skip"
// trying to calculate the clusters
// we can print some information optionally
string Consolidation::TryNextColumn(int i, int col_id, string skip)
{
  int rule_types = 0;

  if (skip == "1")
    rule_types = 1;
  else if (skip == "2")
    rule_types = 2;
  else if (skip == "3")
    rule_types = 3;
  else if (skip == "4")
    rule_types = 4;
  else if (skip == "5")
    rule_types = 5;
  else
    return "skipped this column\n";

  ostringstream buffer;
  Table &table = csvreader->tables[i];

  timeval t1, t2, t3, t4, t5, t6;
  gettimeofday(&t1, NULL);

  // Buid a deduplicated table
  vector<string> new_values;  // the new column, with index as id
  vector<vector<int>> new_clusters;  // map the new column to clusters
  vector<vector<int>> id_mappings;  // mapping the new id to old ids
  vector<string> new_cluster_to_old_cluster;
  vector<int> counts;  // the duplicates of the old id
  for (int cluster_id = 0; cluster_id < clusters.size(); cluster_id++) {
    new_clusters.push_back(vector<int>());
    unordered_map<string, int> value_to_id;
    new_cluster_to_old_cluster.push_back(table.rows[clusters[cluster_id].front()][cluster_id_col]);
    for (auto row_id : clusters[cluster_id]) {
      const string &val = table.rows[row_id][col_id];
      if (!value_to_id.count(val)) {
        value_to_id[val] = new_values.size();
        new_clusters.back().push_back(new_values.size());
        counts.push_back(1);
        new_values.push_back(val);
        id_mappings.emplace_back(1, row_id);
      } else {
        id_mappings[value_to_id[val]].push_back(row_id);
        counts[value_to_id[val]]++;
      }
    }
  }

  bool enable_auto_confirm_rules = false;
  rules = new Rules(table, col_id, new_values, new_clusters, id_mappings, counts, rule_types, enable_auto_confirm_rules);
  rules->GenerateRules();
  Wrapper::logTime(t1, t2, "# S1: Generating Rules.");
  buffer << "Number of Rules Generated: " << rules->termRules.size() << endl;

  termRules.clear();
  int num_rules = min(MAX_NUMBER_OF_RULES, (int) rules->termRules.size());
  rules->RankRules(rules->termRules, termRules, num_rules);
  Wrapper::logTime(t2, t4, "# S2: Ranking and Printing Rules.");
  buffer << "Number of Rules In Use: " << termRules.size() << endl;

  agg = new Aggregator(termRules);
  agg->GroupAggregate();   // Grouping by Structure First and then by transformation

  modification.clear();
  groupStatus.clear();

  modification.resize(new_values.size(), vector<tuple<int, int, int>>());
  groupStatus.resize(agg->groupRules.size(), 0);
  Wrapper::logTime(t4, t5, "# S3: Aggregating Rules.");
  buffer << "Number of Groups: " << agg->groupRules.size() << endl;

  return buffer.str();
}

// print the cluster information, start with max_group_id and max_count
string Consolidation::ShowNextCluster()
{
  ostringstream buffer;
  buffer << rules->ShowNextCluster(termRules, agg->ruleDir, agg->ruleGroup, agg->ruleStatus, agg->groupRules, groupStatus, agg->transformations, agg->structures, modification);
  return buffer.str();
}

// print the statistics
string Consolidation::ApplyCluster(int i, int col_id, int applied_group_num, int max_group_id, string tmp)
{
  ostringstream buffer;

#ifdef RULE_UPDATE_ENABLE
  rules->ApplyRule(termRules, agg->ruleDir, agg->ruleGroup, agg->ruleStatus, agg->groupRules, groupStatus, agg->transformations, agg->structures, modification, max_group_id, tmp);
#else
  rules->ApplyGroupRuleComplex(termRules, agg->ruleDir, agg->ruleGroup, agg->ruleStatus, agg->groupRules, groupStatus, agg->transformations, modification, max_group_id, tmp);
#endif

  Table &table = csvreader->tables[i];
  buffer << "After applying " << applied_group_num << " groups: " << endl;
  buffer << CountClusters(col_id, table, clusters);

  return buffer.str();
}

void Consolidation::MaterializeTable(int i, string outfilepath)
{
  Table &table = csvreader->tables[i];

	Table ans(-1, "whatever");
	ans.row_no = 0;
	ans.col_no = table.col_no;
	ans.schema = table.schema;

	//loop over every cluster
	for (auto &row_ids : clusters)
	{
		vector<string> cur_ans_row;

		for (auto i = 0; i < table.col_no; i ++)
		{
			unordered_map<string, int> votes;
			for (int row : row_ids)
				if (table.rows[row][i].size())
					votes[table.rows[row][i]]++;

			string cur_value = "";
			int max_vote = -1;
			for (auto cp2 : votes)
				if (cp2.second > max_vote)
					max_vote = cp2.second, cur_value = cp2.first;
			cur_ans_row.push_back(cur_value);
		}
		ans.rows.push_back(cur_ans_row);
	}
	ans.row_no = (int) ans.rows.size();
  ans.OutputCSV(outfilepath);
}

/*
void Consolidation::ConsolidationGo()
{
  timeval ta, tb;
  gettimeofday(&ta, NULL);

  Wrapper::logTime(ta, tb, "# Reading Tables: ");

  for (auto i = 0; i < csvreader->tables.size(); i++)
  {
    Table &table = csvreader->tables[i];
    cout << " number of rows: " << table.rows.size() << endl;
    cout << " the schema: ";
    for (auto &attr : table.schema) cout << attr << "; ";
    cout << endl;

    timeval tbeg, tend;
    gettimeofday(&tbeg, NULL);

    int cluster_id_col;
    vector<vector<int>> clusters;
    bool found = GetClusters(cluster_id_col, table, clusters);
    if (!found) continue;

    Wrapper::logTime(tbeg, tend, "# Getting Clusters: ");
    for (int col_id = 0; col_id < table.schema.size(); col_id++) 
    {
      if (col_id == cluster_id_col) continue;

      cout << "looking for rules for table " << table.table_name << " at column " << col_id << endl;
      cout << "number of clusters " << clusters.size() << endl << "avg size " << table.row_no * 1.0 / clusters.size()
        << endl;
      cout << "column: " << table.schema[col_id] << endl;
      cout << "option: 0 (default): skip,  " << endl
                                    << "1: replacing rule only, " << endl
                                    << "2: replacing rules + full rules, " << endl
                                    << "3: replacing rules + deletion rules, " << endl
                                    << "4: replacing rules + deletion rules + full rules," << endl
                                    << "5: full rules only"
                                    << endl;

      int rule_types = 0;

      string skip;
      getline(cin, skip);

      if (skip == "1")
        rule_types = 1;
      else if (skip == "2")
        rule_types = 2;
      else if (skip == "3")
        rule_types = 3;
      else if (skip == "4")
        rule_types = 4;
      else if (skip == "5")
        rule_types = 5;
      else {
        cout << "skipped this column" << endl;
        continue;
      }

      timeval t1, t2, t3, t4, t5, t6;
      gettimeofday(&t1, NULL);

      // Buid a deduplicated table
      vector<string> new_values;  // the new column, with index as id
      vector<vector<int>> new_clusters;  // map the new column to clusters
      vector<vector<int>> id_mappings;  // mapping the new id to old ids
      vector<string> new_cluster_to_old_cluster;
      vector<int> counts;  // the duplicates of the old id
      for (int cluster_id = 0; cluster_id < clusters.size(); cluster_id++) {
        new_clusters.push_back(vector<int>());
        unordered_map<string, int> value_to_id;
        new_cluster_to_old_cluster.push_back(table.rows[clusters[cluster_id].front()][cluster_id_col]);
        for (auto row_id : clusters[cluster_id]) {
          const string &val = table.rows[row_id][col_id];
          if (!value_to_id.count(val)) {
            value_to_id[val] = new_values.size();
            new_clusters.back().push_back(new_values.size());
            counts.push_back(1);
            new_values.push_back(val);
            id_mappings.emplace_back(1, row_id);
          } else {
            id_mappings[value_to_id[val]].push_back(row_id);
            counts[value_to_id[val]]++;
          }
        }
      }


#ifdef OUTPUT_INFO
      for (auto &list_ids : new_clusters) {
        cout << "new cluster: " << endl;
        for (auto vid : list_ids)
          cout << new_values[vid] << endl;
        cout << endl;
      }

      map<int, int> cluster_size;
      for (auto &entry : new_clusters)
        cluster_size[entry.size()] += 1;

      cerr << "after deduplicate." << endl;
      for (auto &xid : cluster_size)
        cerr << xid.first << " " << xid.second << endl;
#endif

      // random pick clusters
#ifdef GENERATE_GROUND_TRUTH
      ofstream ground_truth("groundtruth.csv", ios::out);
      std::set<int> random_cid;
      srand(0);
      while (random_cid.size() != 200)
      {
        int rcid = rand() % new_clusters.size();
        if (new_clusters[rcid].size() > 1)
          random_cid.insert(rcid);
      }
      ground_truth << "id1, value1, id2, value2, reduciable" << endl;
      int num_cluster_now = 0;
      for (auto cid : random_cid)
      {
        for (auto i = 0; i < new_clusters[cid].size(); i++)
        {
          for (auto j = i + 1; j < new_clusters[cid].size(); j++) {
            cout << "cluster: " << num_cluster_now << " " << i << " " << j << endl;
            cout << new_values[new_clusters[cid][i]] << endl;
            cout << new_values[new_clusters[cid][j]] << endl;
            string dec;
            getline(cin, dec);
            ground_truth << new_clusters[cid][i] << ",\"" << new_values[new_clusters[cid][i]] << "\",";
            ground_truth << new_clusters[cid][j] << ",\"" << new_values[new_clusters[cid][j]] << "\"," << dec << endl;
          }
        }
        num_cluster_now++;
      }
      ground_truth.close();
      return 0;
#endif

      Rules rules(table, col_id, new_values, new_clusters, id_mappings, counts, rule_types, enable_auto_confirm_rules);
      rules.GenerateRules();

      Wrapper::logTime(t1, t2, "# S1: Generating Rules.");

      vector<pair<pair<string, string>, int>> termRules;
      int num_rules = min(MAX_NUMBER_OF_RULES, (int) rules.termRules.size());

      cout << "Number of Rules Generated: " << rules.termRules.size() << endl;

      rules.RankRules(rules.termRules, termRules, num_rules);

      cout << "Number of Rules In Use: " << termRules.size() << endl;

#ifdef OUTPUT_INFO
      int print_rule_id = 0;
      cout << endl << "term rules: " << endl;
      PrintRules(print_rule_id, termRules);
#endif
      Wrapper::logTime(t2, t4, "# S2: Ranking and Printing Rules.");


      Aggregator agg(termRules);
#ifdef NO_AGGREGATION_ENABLE
      agg->NoAggregatation();
#elif defined(STRUCTURE_AGGREGATION_ENABLE)
      agg->AggregateStructure();         // Group only by Structure
#elif defined(BOTH_AGGREGATION_ENABLE)
      agg->GroupAggregate();   // Grouping by Structure First and then by transformation
#elif defined(TRANSFORM_AGGREGATION_ENABLE)
      agg->Aggregate();     // Group directly by Transforamtion
#endif

      Wrapper::logTime(t4, t5, "# S3: Aggregating Rules.");

      vector<int> new_id_to_cluster;
      new_id_to_cluster.resize(new_values.size());
      for (auto cid = 0; cid < new_clusters.size(); cid++)  // map the new column to clusters
        for (auto xid : new_clusters[cid])
          new_id_to_cluster[xid] = cid;

      vector<vector<tuple<int, int, int>>> modification;
      modification.resize(new_values.size(), vector<tuple<int, int, int>>());
      vector<int> groupStatus;
      groupStatus.resize(agg->groupRules.size(), 0);

      int applied_group_num = 0;
      do {
#ifdef REAPPLY_RULE_ENABLE
        rules.AutoApplyGroup(termRules, agg->ruleDir, agg->ruleGroup, groupStatus, agg->ruleStatus, agg->groupRules, agg->transformations, agg->structures, modification);
#endif
        applied_group_num++;

        cout << "After applying " << applied_group_num << " groups: " << endl;
        CountClusters(col_id, table, clusters);
      }
#ifdef RULE_UPDATE_ENABLE
      while (rules.ApplyRule(termRules, agg->ruleDir, agg->ruleGroup, agg->ruleStatus, agg->groupRules, groupStatus,
            agg->transformations, agg->structures, modification));
#else
      while (rules.ApplyGroupRuleComplex(termRules, agg->ruleDir, agg->ruleGroup, agg->ruleStatus, agg->groupRules, groupStatus, agg->transformations, modification));
#endif
    }

    cout << "Successfully Exit!" << endl;
  }
}
*/
