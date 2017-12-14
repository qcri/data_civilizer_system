
#include <istream>
#include <sstream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include "CSVReader.h"

bool CSVReader::reading(string &datafilespath, bool normalize) {
  DIR *dir;
  struct dirent *ent;
  int id = 0;
  max_val_len = 0;
  if ((dir = opendir(datafilespath.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {

      // check file
      if (!ends_with(ent->d_name, ".csv")) {
        cerr << "WARNING: Skipped non-csv file " << ent->d_name << endl;
        continue;
      }

      // cout << "LOADING AND PROFILING : " << ent->d_name;
      if (filesize((datafilespath + ent->d_name).c_str()) > MAX_CSV_FILE_SIZE) {
        cerr << "WARNING: Skipped a large file " << ent->d_name << endl;
        continue;
      }

      tables.emplace_back(id, string(ent->d_name));
      if (get_table(datafilespath + "/" + ent->d_name, tables.back().schema, tables.back().cols, tables.back().rows, normalize)) {
        // successfully added, profile and increase the id
        id = id + 1;
        tables.back().Profile();
        // cout << " Added. " << tables.back().schema.size() << " columns and " << tables.back().rows.size() << " rows " << endl;
      } else {
        tables.pop_back();
      }
    }
    closedir (dir);
  } else {
    perror ("could not open catalog directory!!");
    return false;
  }
  return true;
}

bool CSVReader::get_table(const string &filepath, vector<string> &headers, vector<unordered_map<string, int>> &columns, vector<vector<string>> &rows, bool normalize) {

  ifstream in(filepath, ios::in);
  if (in.fail()) return (cout << "File not found: " + filepath << endl) && false;

  csv_read_row(in, headers, normalize);
  columns.resize(headers.size());

  while(in.good())
  {
    vector<string> tuple;
    tuple.reserve(headers.size());
    csv_read_row(in, tuple, normalize);

    if (tuple.size() != headers.size()) {
      // cout << "Skipped a row" << endl;
      continue; // return (cout << "Skipped Broken csv file: " +  filepath << endl) && false;
    }
    for (auto col = 0; col < tuple.size(); col++) {
      if (tuple[col].empty()) continue;
      if (tuple[col].length() > max_val_len)  max_val_len = tuple[col].length();
      if (columns[col].find(tuple[col]) == columns[col].end())
        columns[col][tuple[col]] += 1;
    }
    rows.push_back(tuple);
  }
  in.close();
  return true;
}

// **************************************************
// YOU DO NOT HAVE TO CHANGE ANY OF THE FOLLOWING CODE
// **************************************************
// Read in a row and fill in row, normalize the row if isNorm = true
void CSVReader::csv_read_row(std::istream &in, std::vector<std::string> &row, bool isNorm) {
  std::stringstream ss;
  bool inquotes = false;
  while(in.good()) 
  {
    char c = in.get();
    if (!inquotes && c == '"') //beginquotechar
      inquotes = true;
    else if (inquotes && c == '"') //quotechar
    {
      if ( in.peek() == '"') //2 consecutive quotes resolve to 1
        ss << (char)in.get();
      else //endquotechar
        inquotes = false;
    }
    else if (!inquotes && c == field_delimiter) //end of field
    {
      string temp_str = ss.str();
      if (isNorm) strNormalize(temp_str);
      row.push_back(temp_str);
      ss.str("");
    }
    else if (!inquotes && (c == '\r' || c == '\n'))
    {
      if (in.peek() == '\n')  in.get();
      string temp_str = ss.str();
      if (isNorm) strNormalize(temp_str);
      row.push_back(temp_str);
      return;
    }
    else
    {
      ss << c;
    }
  }
}

int CSVReader::filesize(const char* filename) {
  std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
  return in.tellg(); 
}

bool CSVReader::ends_with(std::string const & value, std::string const & ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
