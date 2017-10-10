
#ifndef _CSV_READER_H_
#define _CSV_READER_H_

#include "Reader.h"

#define MAX_CSV_FILE_SIZE 1024000000

class CSVReader : public Reader {
private:
  char field_delimiter = ','; // '\t'
  int filesize(const char* filename);
  bool ends_with(std::string const & value, std::string const & ending);
  void csv_read_row(std::istream &in, std::vector<std::string> &row, bool isNorm = true);
  bool get_table(const string &filepath, vector<string> &headers, vector<unordered_map<string, int>> &columns, vector<vector<string>> &rows, bool normalize);
  int max_val_len;
public:
  CSVReader() {}
  bool reading(string &datafilespath, bool normalize);
  int get_max_val_len() { return max_val_len; };
};

#endif
