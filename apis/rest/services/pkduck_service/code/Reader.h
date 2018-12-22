//
// Created by Wenbo Tao on 10/28/17.
//

#ifndef XCLEAN_READER_H
#define XCLEAN_READER_H

#include "Common.h"
using namespace std;

#define MAX_CSV_FILE_SIZE 1024000000

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
	void OutputCSV(const string &filename);

private:
	void NullValue();
};

class Reader {
public:
	vector<Table> tables;

	// shorten s to the first 10000 character
	// remove all the non-alphanum and '.' characters
	// collaps multiple whitespace and trim both sides
	void strNormalize(string &s); // also for the use of query normalization

	// read in tables from datafilespath
	virtual bool reading(string &datafilespath, bool normalize) = 0;
	virtual int get_max_val_len() = 0;
};

class CSVReader : public Reader {
private:
	char field_delimiter = ','; // '\t'
	int filesize(const char* filename);
	bool ends_with(std::string const & value, std::string const & ending);
	void csv_read_row(std::istream &in, std::vector<std::string> &row, bool isNorm = true);
	int max_val_len;
public:
	CSVReader() {}
	bool read_files(vector<string> &filepaths, bool normalize);
	bool reading(string &datafilespath, bool normalize);
	bool get_table(const string &filepath, vector<string> &headers, vector<unordered_map<string, int>> &columns, vector<vector<string>> &rows, bool normalize);
	int get_max_val_len() { return max_val_len; };
};


#endif //XCLEAN_READER_H
