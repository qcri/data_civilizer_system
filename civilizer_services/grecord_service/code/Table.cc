
#include "Table.h"

Table::Table(int id, const string& name) {
  tid = id;
  table_name = name;
}

Table::Table(int id, const string &name, vector<string> &data_headers, vector<vector<string>> &data_rows, vector<unordered_map<string, int>> &data_columns) {
  tid = id;
  table_name = name;

  schema = data_headers;
  rows = data_rows;
  cols = data_columns;

  row_no = rows.size();
  col_no = cols.size();
}

void Table::Profile() {
  row_no = rows.size();
  col_no = cols.size();
  Uniqueness();
  TypeInference();
  NullValue();
}

void Table::NullValue() {
  hasNull.resize(cols.size());
  isKeyCand.resize(cols.size());
  notNullNum.resize(cols.size());
  for (auto i = 0; i < cols.size(); i++) {
    hasNull[i] = false;
    notNullNum[i] = rows.size();
    for (auto val : rows) {
      if (val[i] == "") {
        hasNull[i] = true;
        notNullNum[i] -= 1;
      }
    }
    isKeyCand[i] = false;
    if (!hasNull[i] && isUnique[i]) isKeyCand[i] = true;
  }
}


void Table::OutputCSV(const string &filename)
{
  ofstream outfile(filename);

  outfile << "\"" << schema.front() << "\"";
  for (auto i = 1; i < schema.size(); i++)
    outfile << ",\"" << schema[i] << "\"";
  outfile << endl;

  for (auto &row : rows)
  {
    outfile << "\"" << row.front() << "\"";
    for (auto i = 1; i < row.size(); i++)
      outfile << ",\"" << row[i] << "\"";
    outfile << endl;
  }
  outfile.close();
}


void Table::Uniqueness() {
  isUnique.resize(cols.size());
  for (auto i = 0; i < cols.size(); i++) {
    if (cols[i].size() > UNIQUE_THRESH * row_no)
      isUnique[i] = true;
    else
      isUnique[i] = false;
  }
}

/*
inline bool isInteger(const std::string & s) {
  if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
  char * p;
  strtod(s.c_str(), &p);
  return (*p == 0);
}
*/

bool isInteger(const std::string &s){
  return s.find_first_not_of( "0123456789" ) == string::npos;
}

void Table::TypeInference() {
  type.resize(cols.size());
  for (auto i = 0; i < cols.size(); i++) {
    int tf = 0;
    for (auto entry : cols[i])
      if (isInteger(entry.first)) {
        if (atoi(entry.first.c_str()) <= 1000)
          tf++;
      }
    type[i] = 0;
    if (tf * 1.0 / cols[i].size() + EPS >= INT_TYPE_THRESH) type[i] = 1;
  }
}
