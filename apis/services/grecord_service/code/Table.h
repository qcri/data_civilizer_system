
#ifndef _TABLE_H_
#define _TABLE_H_

#include "Wrapper.h"

class Table {
public:
  int tid;
  int row_no, col_no;
  string table_name;
  vector<string> schema;
  vector<vector<string>> rows;
  vector<unordered_map<string, int>> cols;

  vector<int> type; // 0 for string, 1 for int, 2 for float
  vector<bool> isUnique;
  vector<bool> isKeyCand;
  vector<bool> hasNull;
  vector<int> notNullNum;
  
  Table (int id, const string &name);
  Table (int id, const string &name, vector<string> &data_headers, vector<vector<string>> &data_rows, 
      vector<unordered_map<string, int>> &data_columns);
  void Profile();
  void OutputCSV(const string &filename);

private:
  void NullValue();
  void Uniqueness();
  void TypeInference();
};
#endif
