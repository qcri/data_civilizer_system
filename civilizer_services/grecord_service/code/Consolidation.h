
#ifndef ENUM_RULE_GOLDENRECORD_H
#define ENUM_RULE_GOLDENRECORD_H

#include "Table.h"
#include "CSVReader.h"
#include "Rules.h"
#include "Synthesize.h"
#include "Wrapper.h"


class Consolidation
{
  public:

    string cluster_id_name;
    string csvfilepath;

    Reader *csvreader;
    int number_of_tables;

    int cluster_id_col;
    vector<vector<int>> clusters;

    Aggregator *agg;
    Rules *rules;

    vector<int> groupStatus;
    vector<vector<tuple<int, int, int>>> modification;
    vector<pair<pair<string, string>, int>> termRules;

    Consolidation(string filepath, string cname);
    //: csvfilepath(filepath), cluster_id_name(cname) { }

    // void ConsolidationGo();

    string TryNextTable(int i);
    string ProfileColumn(int i, int col_id);
    string TryNextColumn(int i, int col_id, string skip);
    string ShowNextCluster();
    string ApplyCluster(int i, int col_id, int applied_group_num, int max_group_id, string tmp);

    void MaterializeTable(int i, string outfilepath);

    // void CountClusters(int column_id, const Table &table, const vector<vector<int>> &clusters);
    // bool GetClusters(int &cluster_id_col, const Table &table, vector<vector<int>> &clusters);
    // void PrintRules(int &print_rule_id, const vector<pair<pair<string, string>, int>> &rules);
    // void ShowDiffClusters(int column_id, const Table &table, const vector<vector<int>> &clusters);
};
#endif
