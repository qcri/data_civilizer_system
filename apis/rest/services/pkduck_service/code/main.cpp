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

	//write sim string file
	string output_file_name1(out_dir);
	if (output_file_name1.back() != '/')
		output_file_name1 += "/";
	output_file_name1 += "simstring_pkduck.csv";
	ofstream fout1(output_file_name1.c_str());
	fout1 << "string1,string2" << endl;
	for (auto cp : results)
		fout1 << "\"" << cp.first << "\", \"" << cp.second << "\"" << endl;
	fout1.close();

	//write auxiliary table file
	string output_file_name2(out_dir);
	if (output_file_name2.back() != '/')
		output_file_name2 += "/";
	output_file_name2 += "auxiliary_pkduck.csv";
	ofstream fout2(output_file_name2.c_str());
	fout2 << "id,string,tablename,row,column,columnname" << endl;
	unordered_map<string, string> ss;
	for (auto cp : results)
	{
		ss[cp.first] = cp.second;
		ss[cp.second] = cp.first;
	}

	int id = 0;
	for (auto s : ss)
		for (auto i = 0; i < reader->tables.size(); i ++)
		{
			int row_no = reader->tables[i].row_no;
			int col_no = reader->tables[i].col_no;
			for (auto x = 0; x < row_no; x ++)
				for (auto y = 0; y < col_no; y ++)
					if (reader->tables[i].rows[x][y] == s.first)
						fout2 << "\"" << id << "\","
							  << "\"" << s.first << "\","
						      << "\"" << reader->tables[i].table_name << "\","
							  << "\"" << x << "\","
						      << "\"" << y << "\","
						      << "\"" << reader->tables[i].schema[y] << "\"" << endl;
		}
	fout2.close();

	// write updated tables
	for (auto i = 0; i < reader->tables.size(); i ++)
	{
		// check if there is values updated
		bool update = false;
		int row_no = reader->tables[i].row_no;
		int col_no = reader->tables[i].col_no;
		for (auto x = 0; x < row_no; x ++)
			for (auto y = 0; y < col_no; y ++)
//				if (ss.count(reader->tables[i].rows[x][y]))
//					update = true;
                        {
                                string p = reader->tables[i].rows[x][y];
                                if (ss.count(p) && p.size() > ss[p].size()) {
                                        reader->tables[i].rows[x][y] = ss[p];
					update = true;
				}
                        }
		if (! update)
			continue;

		// write a new file
		string output_file_name(out_dir);
		if (output_file_name.back() != '/')
			output_file_name += "/";
		output_file_name += "updated_" + reader->tables[i].table_name;
//		ofstream fout(output_file_name.c_str());
                reader->tables[i].OutputCSV(output_file_name.c_str());
/*
		// schema
		for (auto y = 0; y < col_no; y ++)
		{
			if (y) fout << ",";
//			fout << reader->tables[i].schema[y];
			fout << "\"" << reader->tables[i].schema[y] << "\"";
		}
		fout << endl;
		for (auto x = 0; x < row_no; x ++)
		{
			for (auto y = 0; y < col_no; y++)
			{
				if (y) fout << ",";
				string p = reader->tables[i].rows[x][y];
//				if (!ss.count(p) || p.size() <= ss[p].size())
//					fout << p;
//				else
//					fout << ss[p];
				if (ss.count(p) && p.size() > ss[p].size())
					p = ss[p];
				fout << "\"" << p << "\"";
			}
			fout << endl;
		}
*/
	}
}

int main()
{
	return 0;
}
