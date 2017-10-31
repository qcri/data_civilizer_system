#include "Common.h"
#include "Reader.h"
#include "Exp.h"
#include "Solver/Solver.h"

using namespace std;

void split(vector<string> &strs, string _s, char delimiter)
{
	strs.clear();
	string s = "";
	for (auto i = 0; i < _s.size(); i ++)
		if (_s[i] != ' ')
			s += _s[i];
	while (s.size())
	{
		auto pos = s.find(delimiter);
		if (pos == string::npos)
		{
			strs.push_back(s);
			break;
		}
		strs.push_back(s.substr(0, pos));
		s = s.substr(pos + 1);
	}
}


extern "C"
void execute(char *table_dir, char *out_dir, char *_columns, double tau)
{
	Common::JAC_THRESHOLD = tau;

	string col_str = string(_columns);
	vector<string> table_names, col_id_strs;
	split(col_id_strs, col_str, '#');

	//read csvs
	CSVReader *reader = new CSVReader();
	string dir(table_dir);
	reader->reading(dir, false);

	cout << "haha : " << endl;
	cout << reader->tables.size() << endl << endl;
	for (auto i = 0; i < reader->tables.size(); i ++)
		cout << reader->tables[i].table_name << endl;
	for (auto i = 0; i < reader->tables.size(); i ++)
	{
		reader->tables[i].row_no = (int) reader->tables[i].rows.size();
		reader->tables[i].col_no = (int) reader->tables[i].rows[0].size();
	}

	unordered_set<string> cells;
	//read columns
	for (auto i = 0; i < col_id_strs.size(); i ++)
	{
		vector<string> col_ids;
		split(col_ids, col_id_strs[i], ',');

		for (string s : col_ids)
		{
			int cur_col = atoi(s.c_str());
			for (auto r = 0; r < reader->tables[i].row_no; r ++)
				cells.insert(reader->tables[i].rows[r][cur_col]);
		}
	}

	//call solver
	Solver solver(vector<string>(cells.begin(), cells.end()));
	vector<pair<string, string>> results = solver.solve();

	//write it to file
	string output_file_name(out_dir);
	if (output_file_name.back() != '/')
		output_file_name += "/";
	output_file_name += "result_pkduck.csv";
	ofstream fout(output_file_name.c_str());
	fout << "string1,string2" << endl;
	for (auto cp : results)
		fout << "\"" << cp.first << "\", \"" << cp.second << "\"" << endl;
	fout.close();
}

int main()
{
	return 0;
}
