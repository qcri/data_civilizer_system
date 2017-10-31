//
// Created by Wenbo Tao on 2/11/17.
//

#include "Exp.h"
#include "Joiner/Joiner.h"
#include "Joiner/PolynomialJoiner.h"
#include "Solver/Solver.h"
#include "Joiner/JaccardJoiner.h"
#include "RuleGenerator/BadBoy.h"
#include "RuleGenerator/FastLCS.h"

using namespace std;

/*unordered_set<string> Exp::file_names = unordered_set<string>({"data/dept_names/dept_names.txt",
															   "data/course_names/course_names.txt",
															   "data/area_names/area_names.txt",
															   "data/disease_names/disease_names.txt"
															  });
*/
unordered_set<string> Exp::file_names = unordered_set<string>({"data/disease_names/disease_names.txt"});

void Exp::preprocess()
{
	string file_names[3] = {"dept_names/dept_names",
							"course_names/course_names",
							"area_names/area_names"};

	for (string file_name : file_names)
	{
		string full_file_name = "data/" + file_name + "_raw.txt";
		ifstream fin(full_file_name.c_str());
		unordered_set<string> cell_set;
		vector<string> cells;
		string s;

		//cin
		while (getline(fin, s))
		{
			vector<string> raw_tokens = Common::get_tokens(s);
			vector<string> tokens;
			for (string t : raw_tokens)
			{
				bool contain_alpha = false;
				for (auto i = 0; i < t.size(); i ++)
					if (isalpha(t[i]))
						contain_alpha = true;
				if (! contain_alpha)
					continue;
				tokens.push_back(t);
			}
			string concat = "";
			for (string t : tokens)
				concat += t + " ";

			sort(tokens.begin(), tokens.end());
			string sort_concat = "";
			for (string t : tokens)
				sort_concat += t + " ";
			if (! cell_set.count(sort_concat))
				cell_set.insert(sort_concat), cells.push_back(concat);
		}

		//output
		full_file_name = "data/" + file_name + ".txt";
		ofstream fout1(full_file_name.c_str());
		for (string t : cells)
			fout1 << t << endl;
		fout1.close();

		full_file_name = "data/" + file_name + "_weights.txt";
		ofstream fout2(full_file_name.c_str());
		for (auto i = 0; i < cells.size(); i ++)
			fout2 << 1 << endl;
		fout2.close();
	}
}

void Exp::check()
{
	string methods[] = {"sigmod", "sim"};
	string files[] = {"course", "dept", "area"};

	for (string file : files)
	{
		string small_file_name = "data/" + file + "_names/" + file + "_names_small.txt";
		ifstream fin1(small_file_name.c_str());
		unordered_set<string> cells;
		for (string cell; getline(fin1, cell); )
			cells.insert(cell);
		fin1.close();

		set<string> pair_string_set;
		int count = 0;
		for (string method : methods)
		{
			string result_file_name = "exp/measure/" + file + "/" + method + ".log";
			ifstream fin2(result_file_name.c_str());
			string ss, s1, s2;
			getline(fin2, ss);
			int n = atoi(ss.c_str());

			for (int i = 0; i < n; i ++)
			{
				double sim;
				getline(fin2, s1);
				getline(fin2, s2);
				getline(fin2, ss);
				sim = atof(ss.c_str());
				getline(fin2, ss);

				if (method == "sigmod" && sim < 0.78)
					continue;
				if (cells.count(s1) && cells.count(s2) && s1[0] == s2[0])
				{
					if (s1 > s2)
						swap(s1, s2);
					string pair_string = s1 + "___" + s2;
					if (! pair_string_set.count(pair_string))
					{
						pair_string_set.insert(pair_string);
						count ++;
					}
				}
			}
			fin2.close();
		}

		//output to candidate file
		string cand_file_name = "data/" + file + "_names/" + file + "_names_small_cand.txt";
		ofstream fout(cand_file_name.c_str());
		fout << count << endl;
		for (string s : pair_string_set)
		{
			auto pos = s.find("___");
			string s1 = s.substr(0, pos);
			string s2 = s.substr(pos + 3);
			fout << endl << s1 << endl << s2 << endl << endl;
		}
		fout.close();
	}
}

void Exp::check2()
{
	string ids[] = {"1000", "2000", "3000", "4000", "5000", "8000", "20000", "25000", "45000", "60000"};
	unordered_set<string> dict_set;
	vector<pair<string, string>> dictionary;
	for (string id : ids)
	{
		string file_name = "data/gp_dict_" + id + ".txt";
		ifstream fin(file_name.c_str());
		string s1, s2, s3;
		while (getline(fin, s1) && getline(fin, s2))
		{
			//dedup
			string concat = s1 + "___xxx___" + s2;
			if (dict_set.count(concat))
				continue;
			dict_set.insert(concat);

			//process s2
			for (auto i = 0; i < s2.size(); i ++)
				if (s2[i] == '-' || s2[i] == '/' || s2[i] == '/')
					s2[i] = ' ';
			while (1)
			{
				bool flag = false;
				for (auto i = 1; i < s2.size(); i ++)
					if (s2[i] == ' ' && s2[i - 1] == ' ')
					{
						s2.erase(i, 1);
						flag = true;
						break;
					}
				if (! flag)
					break;
			}
			if (s1 == "" || s2 == "")
				continue;

			dictionary.emplace_back(s1, s2);
		}
		fin.close();
	}
	ofstream fout("data/gp_dict.txt");
	for (auto cp : dictionary)
		fout << cp.first << endl << cp.second << endl << endl;
}

void Exp::runSolver()
{
	for (string f : file_names)
	{
		for (auto i = 0; i < 10; i ++)
			cout << endl;
		cout << f << " : " << endl << endl;
		Solver *solver = new Solver(f);
		delete solver;
	}
}

void Exp::varyDictionary()
{
	Common::set_default();
	Common::JAC_THRESHOLD = 0.7;
	Common::MEASURE = 0;

	//vldb 09, jac threshold = 0.4
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "VLDB09 dictionary, jac = 0.4" << endl;
	cout << "--------------------------" << endl << endl;
	Common::DICTIONARY = 1;
	Common::VLDB09_JAC_THRESHOLD = 0.4;
	runSolver();

	//vldb 09, jac threshold = 0.6
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "VLDB09 dictionary, jac = 0.6" << endl;
	cout << "--------------------------" << endl << endl;
	Common::DICTIONARY = 1;
	Common::VLDB09_JAC_THRESHOLD = 0.6;
	runSolver();

	//vldb 09, jac threshold = 0.8
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "VLDB09 dictionary, jac = 0.8" << endl;
	cout << "--------------------------" << endl << endl;
	Common::DICTIONARY = 1;
	Common::VLDB09_JAC_THRESHOLD = 0.8;
	runSolver();

	//lcs, delta = 0
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "LCS dictionary, delta = 0" << endl;
	cout << "--------------------------" << endl << endl;
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = false;
	runSolver();

	//lcs, delta = 1
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "LCS dictionary, delta = 1" << endl;
	cout << "--------------------------" << endl << endl;
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = true;
	runSolver();
}

void Exp::varyMeasure()
{
	Common::set_default();
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = false;
	Common::JAC_THRESHOLD = 0.65;

	//sigmod 13
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "SIGMOD 13 measure:" << endl;
	cout << "--------------------------" << endl << endl;
	Common::MEASURE = 1;
	runSolver();

	//our measure
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "Our Sim measure:" << endl;
	cout << "--------------------------" << endl << endl;
	Common::MEASURE = 0;
	runSolver();

	//Jaccard
	for (auto i = 0; i < 30; i ++)
		cout << endl;
	cout << "--------------------------" << endl;
	cout << "Simple Jaccard:" << endl;
	cout << "--------------------------" << endl;
	for (string f : file_names)
	{
		for (auto i = 0; i < 10; i ++)
			cout << endl;
		cout << f << " : " << endl << endl;

		vector<string> cells;
		ifstream fin1(f.c_str());
		for (string cell; getline(fin1, cell); )
			cells.push_back(cell);
		JaccardJoiner *jacJoiner = new JaccardJoiner(vector<t_rule>(), cells, Common::JAC_THRESHOLD);
		vector<pair<double, pair<string, string>>> joinedStringPairs;
		joinedStringPairs = jacJoiner->getJoinedStringPairs();

		cout << joinedStringPairs.size() << endl;
		sort(joinedStringPairs.begin(), joinedStringPairs.end());
		for (auto cp : joinedStringPairs)
			cout << cp.second.first << endl << cp.second.second << endl << cp.first << endl << endl;
		delete jacJoiner;
	}
}

void Exp::varyThreshold()
{
	Common::set_default();
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = false;
	Common::DO_JOIN = false;

	//vary threshold
	for (Common::JAC_THRESHOLD = 0.7; Common::JAC_THRESHOLD <= 0.9; Common::JAC_THRESHOLD += 0.1)
	{
		for (Common::MEASURE = 0; Common::MEASURE <= 2; Common::MEASURE ++)
		{
			//skip sigmod 13
			if (Common::MEASURE == 1)
				continue;
			for (auto i = 0; i < 30; i ++)
				cout << endl;

			if (Common::MEASURE == 0)
			{
				cout << "--------------------------" << endl;
				cout << "Sim, JAC_THRESHOLD = " << Common::JAC_THRESHOLD << " :" << endl;
				cout << "--------------------------" << endl;
			} else if (Common::MEASURE == 1)
			{
				cout << "--------------------------" << endl;
				cout << "SIGMOD 13, JAC_THRESHOLD = " << Common::JAC_THRESHOLD << " :" << endl;
				cout << "--------------------------" << endl;
			} else
			{
				cout << "--------------------------" << endl;
				cout << "Jacct, JAC_THRESHOLD = " << Common::JAC_THRESHOLD << " :" << endl;
				cout << "--------------------------" << endl;
			}

			//vary datasets
			for (string f : file_names)
			{
				for (auto i = 0; i < 10; i ++)
					cout << endl;
				cout << f << " : " << endl << endl;

				vector<string> cells;
				ifstream fin1(f.c_str());
				for (string cell; getline(fin1, cell); )
					cells.push_back(cell);
				fin1.close();

				RuleGenerator *ruleGenerator = new BadBoy(cells);
				vector<t_rule> rules = ruleGenerator->gen_rules();
				for (int i = 0, n = (int) rules.size(); i < n; i ++)
					rules.emplace_back(make_pair(rules[i].second, rules[i].first));


				Joiner *joiner = new PolynomialJoiner(rules, cells);
				joiner->getJoinedStringPairs();
				delete ruleGenerator;
				delete joiner;
			}
		}
	}
}

void Exp::genDirty()
{
	string folder = "exp/measure/";
	string files[] = {"area"};
	string methods[] = {"sim", "sigmod"};

	for (string file : files)
	{
		unordered_set<string> left, right;
		string file_name = "exp/quality_data/" + file + "/match.txt";
		ifstream fin1(file_name.c_str());
		string s1, s2, ss;
		while (getline(fin1, s1))
		{
			getline(fin1, s2);
			getline(fin1, ss);
			if (s1.back() != ' ')
				s1 += " ";
			if (s2.back() != ' ')
				s2 += " ";
			left.insert(s1);
			right.insert(s2);
		}
		fin1.close();

		unordered_set<string> dirty_record_set;
		for (string method : methods)
		{
			file_name = folder + file + "/" + method + ".log";
			ifstream fin(file_name.c_str());

			getline(fin, ss);
			int n = atoi(ss.c_str());
			for (auto j = 0; j < n; j ++)
			{
				getline(fin, s1);
				getline(fin, s2);
				getline(fin, ss);
				double sim = atof(ss.c_str());
				getline(fin, ss);

				if (left.count(s1))
					if (s2.find("new orleans") == string::npos)
						dirty_record_set.insert(s2);
				if (left.count(s2))
					if (s1.find("new orleans") == string::npos)
						dirty_record_set.insert(s1);
			}
		}

		vector<string> dirty_records;
		for (auto s : dirty_record_set)
			dirty_records.push_back(s);
		random_shuffle(dirty_records.begin(), dirty_records.end());
		for (int i = 0; i < 100; i ++)
			cout << dirty_records[i] << endl;
	}
}

void Exp::calculateMeasurePRF()
{
	string files[] = {"disease"};
	string methods[] = {"sim", "sigmod", "jaccard"};

	for (string file : files)
	{
		cout << file << " : " << endl << endl;
		unordered_set<string> cells;
		set<pair<string, string>> true_pairs;
		string file_name, s1, s2, ss;

		//cells
		string cell_file_name = "data/" + file + "_names/" + file + "_names_small.txt";
		ifstream fin1(cell_file_name.c_str());
		while (getline(fin1, s1))
		{
			if (s1.back() != ' ')
				s1 += " ";
			cells.insert(s1);
		}
		fin1.close();

		//gt
		string gt_file_name = "data/" + file + "_names/" + file + "_names_small_gt.txt";
		ifstream fin2(gt_file_name.c_str());
		unordered_set<string> lc;
		while (getline(fin2, s1))
		{
			getline(fin2, s2);
			getline(fin2, ss);
			if (s1.back() != ' ')
				s1 += " ";
			if (s2.back() != ' ')
				s2 += " ";
			true_pairs.insert(make_pair(s1, s2));
			lc.insert(s1);
		}
		fin2.close();

		//precision & recall
		for (string method : methods)
		{
			for (double th = 0.7; th <= 0.9; th += 0.1)
			{
				//cout << "threshold : " << th << endl;
				string log_file_name = "exp/measure/" + file + "/" + method + ".log";
				ifstream fin3(log_file_name.c_str());
				getline(fin3, ss);
				int n = atoi(ss.c_str());
				int correct = 0;
				int total = 0;

				for (int i = 0; i < n; i ++)
				{
					double sim;
					getline(fin3, s1);
					getline(fin3, s2);
					getline(fin3, ss);
					sim = atof(ss.c_str());
					getline(fin3, ss);
					if (sim < th)
						continue;

					if (s1.back() != ' ')
						s1 += " ";
					if (s2.back() != ' ')
						s2 += " ";

					if (file == "disease" && lc.count(s1) && lc.count(s2))
						continue;
					if (file == "disease" && ! lc.count(s1) && ! lc.count(s2))
						continue;

					if (cells.count(s1) && cells.count(s2))
					{
						total ++;
						bool f = false;
						if (true_pairs.count(make_pair(s1, s2)) ||
							true_pairs.count(make_pair(s2, s1)))
							correct ++, f = true;
//						if (th > 0.75 && method == "sim" && ! f)
//							cout << endl << s1 << endl << s2 << endl << endl;
					}
				}

//				cout << method << " : " << endl << "Presicion : " << correct << " / " << total << endl;
//				cout << "Recall : " << correct << " / " << true_pairs.size() << endl;

				cout << method << " : " << endl;
				double p = correct / (double) total;
				double r = correct / (double) true_pairs.size();
				double f = (p + r > 0 ? 2 * p * r / (p + r) : 0);
				printf("Precision : %.2lf\n", p);
				printf("Recall : %.2lf\n", r);
				printf("F1 Score : %.2lf\n", f);
				cout << endl;
			}
			cout << endl << endl << endl << endl;
		}
	}
}

void Exp::calculateDictPRF()
{
	string files[] = {"dept", "disease", "course", "area"};
	string methods[] = {"hand", "gp", "lcs_0", "lcs_1", "vldb09_0.4", "vldb09_0.6", "vldb09_0.8"};

	for (string file : files)
	{
		cout << file << " : " << endl << endl;
		unordered_set<string> cells;
		set<pair<string, string>> true_pairs;
		string file_name, s1, s2, ss;

		//cells
		string cell_file_name = "data/" + file + "_names/" + file + "_names_small.txt";
		ifstream fin1(cell_file_name.c_str());
		while (getline(fin1, s1))
		{
			if (s1.back() != ' ')
				s1 += " ";
			cells.insert(s1);
		}
		fin1.close();

		//gt
		string gt_file_name = "data/" + file + "_names/" + file + "_names_small_gt.txt";
		ifstream fin2(gt_file_name.c_str());
		unordered_set<string> lc;
		while (getline(fin2, s1))
		{
			getline(fin2, s2);
			getline(fin2, ss);
			if (s1.back() != ' ')
				s1 += " ";
			if (s2.back() != ' ')
				s2 += " ";
			true_pairs.insert(make_pair(s1, s2));
			lc.insert(s1);
		}
		fin2.close();

		//precision & recall
		for (string method : methods)
		{
			for (double th = 0.7; th <= 0.7; th += 0.1)
			{
				cout << "threshold : " << th << endl;
				string log_file_name = "exp/dictionary/" + method + "_" + file + "_names.txt";
				ifstream fin3(log_file_name.c_str());
				getline(fin3, ss);
				int n = atoi(ss.c_str());
				int correct = 0;
				int total = 0;

				for (int i = 0; i < n; i ++)
				{
					double sim;
					getline(fin3, s1);
					getline(fin3, s2);
					getline(fin3, ss);
					sim = atof(ss.c_str());
					getline(fin3, ss);
					if (sim < th)
						continue;

					if (s1.back() != ' ')
						s1 += " ";
					if (s2.back() != ' ')
						s2 += " ";

					if (file == "disease" && lc.count(s1) && lc.count(s2))
						continue;
					if (file == "disease" && ! lc.count(s1) && ! lc.count(s2))
						continue;

					if (cells.count(s1) && cells.count(s2))
					{
						total ++;
						bool f = false;
						if (true_pairs.count(make_pair(s1, s2)) ||
							true_pairs.count(make_pair(s2, s1)))
							f = true, correct ++;
//						if (method == "lcs_0" && ! f)
//							cout << endl << endl << s1 << endl << s2 << endl << endl << endl;
					}
				}

//				cout << method << " : " << endl << "Presicion : " << correct << " / " << total << endl;
//				cout << "Recall : " << correct << " / " << true_pairs.size() << endl;

				cout << method << " : " << endl;
				double p = correct / (double) total;
				double r = correct / (double) true_pairs.size();
				double f = (p + r > 0 ? 2 * p * r / (p + r) : 0);
				printf("Precision : %.2lf\n", p);
				printf("Recall : %.2lf\n", r);
				printf("F1 Score : %.2lf\n", f);
				cout << endl;
			}
		}
	}
}

void Exp::dictionary_scale()
{
	Common::set_default();
	Common::DICTIONARY = 0;

	RuleGenerator *ruleGenerator;
	for (string f : file_names)
	{
		cout << f << endl << endl;
		vector<string> cells;
		ifstream fin1(f.c_str());
		for (string cell; getline(fin1, cell); )
			cells.push_back(cell);
		random_shuffle(cells.begin(), cells.end());
		int N = (int) cells.size();
		int n = N / 4;
		fin1.close();

		cout << "\t delta = 0 : " << endl << endl;
		Common::ENABLE_DELTA = false;

		for (int i = 1; i <= 4; i ++)
		{
			vector<string> cells_partial;
			for (auto j = 0; j < n * i; j ++)
				cells_partial.push_back(cells[j]);
			struct timeval t1, t2;
			gettimeofday(&t1, NULL);
			ruleGenerator = new FastLCS(cells_partial);
			ruleGenerator->gen_rules();
			gettimeofday(&t2, NULL);
			double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
			cout << "\t\t i = " << i << " : " << elapsedTime << "s." << endl << endl;
			delete ruleGenerator;
		}

		cout << "\t delta = 1 : " << endl << endl;
		Common::ENABLE_DELTA = true;

		for (int i = 1; i <= 4; i ++)
		{
			vector<string> cells_partial;
			for (auto j = 0; j < n * i; j ++)
				cells_partial.push_back(cells[j]);
			struct timeval t1, t2;
			gettimeofday(&t1, NULL);
			ruleGenerator = new FastLCS(cells_partial);
			ruleGenerator->gen_rules();
			gettimeofday(&t2, NULL);
			double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
			cout << "\t\t i = " << i << " : " << elapsedTime << "s." << endl << endl;
			delete ruleGenerator;
		}
	}
}

void Exp::joinalgo_scale()
{
	Common::set_default();
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = false;
	Common::MEASURE = 0;
	Common::DO_JOIN = true;

	for (Common::JAC_THRESHOLD = 0.7; Common::JAC_THRESHOLD <= 0.9; Common::JAC_THRESHOLD += 0.1)
	{
		if (Common::JAC_THRESHOLD > 0.75 && Common::JAC_THRESHOLD < 0.85)
			continue;

		cout << endl << endl << "JAC threshold : " << Common::JAC_THRESHOLD << endl << endl << endl;
		RuleGenerator *ruleGenerator;
		Joiner *joiner;

		for (string f : file_names)
		{
			if (f.find("dept") != string::npos)
				continue;
			cout << f << endl << endl;

			//read files
			vector<string> cells;
			ifstream fin1(f.c_str());
			for (string cell; getline(fin1, cell); )
				cells.push_back(cell);
			random_shuffle(cells.begin(), cells.end());
			int N = (int) cells.size();
			int n = N / 4;
			fin1.close();

			//generate rules
			vector<t_rule> rules;
			ruleGenerator = new BadBoy(cells);
			rules = ruleGenerator->gen_rules();
			for (int i = 0, n = (int) rules.size(); i < n; i ++)
				rules.emplace_back(make_pair(rules[i].second, rules[i].first));
			delete ruleGenerator;

			for (int i = 1; i <= 4; i ++)
			{
				vector<string> cells_partial;
				for (auto j = 0; j < n * i; j ++)
					cells_partial.push_back(cells[j]);
				struct timeval t1, t2;
				gettimeofday(&t1, NULL);
				joiner = new PolynomialJoiner(rules, cells_partial);
				joiner->getJoinedStringPairs();
				gettimeofday(&t2, NULL);
				double elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
				cout << "\t i = " << i << " : " << elapsedTime << "s." << endl << endl;
				delete joiner;
			}
		}
	}
}

void Exp::show_datasets()
{
	for (string f : file_names)
	{
		for (auto i = 0; i < 10; i++)
			cout << endl;
		cout << f << " : " << endl << endl;

		vector<string> cells;
		ifstream fin1(f.c_str());
		for (string cell; getline(fin1, cell);)
			cells.push_back(cell);
		fin1.close();

		int max_token_len = 0, min_token_len = 100, sum_token_len = 0;
		int sum_token_sq = 0;
		for (string cell : cells)
		{
			vector<string> tokens = Common::get_tokens(cell);
			max_token_len = max(max_token_len, (int) tokens.size());
			min_token_len = min(min_token_len, (int) tokens.size());
			sum_token_len += (int) tokens.size();
			sum_token_sq += (int) tokens.size() * ((int) tokens.size() + 1) / 2;
		}
		cout << "max #token : " << max_token_len << endl;
		cout << "min #token : " << min_token_len << endl;
		cout << "avg #token : " << (double) sum_token_len / cells.size() << endl;
		cout << "search space : " << (long long) sum_token_sq * sum_token_len << endl;
		cout << endl;
	}
}

void Exp::testRuleCompression()
{
	Common::set_default();
	Common::DICTIONARY = 0;
	Common::ENABLE_DELTA = false;
	Common::MEASURE = 0;
	Common::DO_JOIN = false;

	//vary threshold
	for (Common::JAC_THRESHOLD = 0.7; Common::JAC_THRESHOLD <= 0.9; Common::JAC_THRESHOLD += 0.1)
	{
		for (int compress = 0; compress < 2; compress ++)
		{
			Common::FAST_SIG = (compress != 0);
			for (auto i = 0; i < 30; i ++)
				cout << endl;

			if (Common::FAST_SIG)
			{
				cout << "--------------------------" << endl;
				cout << "Compress, JAC_THRESHOLD = " << Common::JAC_THRESHOLD << " :" << endl;
				cout << "--------------------------" << endl;
			} else
			{
				cout << "--------------------------" << endl;
				cout << "No compress, JAC_THRESHOLD = " << Common::JAC_THRESHOLD << " :" << endl;
				cout << "--------------------------" << endl;
			}

			//vary datasets
			for (string f : file_names)
			{
				for (auto i = 0; i < 10; i ++)
					cout << endl;
				cout << f << " : " << endl << endl;

				vector<string> cells;
				ifstream fin1(f.c_str());
				for (string cell; getline(fin1, cell); )
					cells.push_back(cell);
				fin1.close();

				RuleGenerator *ruleGenerator = new BadBoy(cells);
				vector<t_rule> rules = ruleGenerator->gen_rules();
				for (int i = 0, n = (int) rules.size(); i < n; i ++)
					rules.emplace_back(make_pair(rules[i].second, rules[i].first));

				Joiner *joiner = new PolynomialJoiner(rules, cells);
				joiner->getJoinedStringPairs();
				delete ruleGenerator;
				delete joiner;
			}
		}
	}
}

void Exp::genSubset()
{
	string folder = "data/";
	string files[] = {"course", "dept", "area"};
	srand(time(NULL));
	for (string file : files)
	{
		string file_name = folder + file + "_names/" + file + "_names.txt";
		string out_file_name = folder + file + "_names/" + file + "_names_small.txt";
		ifstream fin(file_name.c_str());
		ofstream fout(out_file_name.c_str());
		vector<string> cells;
		int N;
		for (string cell; getline(fin, cell); )
		{
			N ++;
			bool ok = true;
			if (cell.find("new orleans la") != string::npos)
				ok = false;
			for (auto c = 0; c < cell.size(); c ++)
				if (isdigit(cell[c]))
					ok = false;
			if (file == "dept")
				ok = true;
			if (ok)
				cells.push_back(cell);
		}

		unordered_set<int> used_ids;
		if (file != "dept")
			N = (int) (N * 0.05);
		else
			N = (int) cells.size();
		for (int i = 0; i < N; i ++)
		{
			int x;
			while (1)
			{
				x = (int) ((long long) rand() * (long long) rand() % (long long) cells.size());
				if (! used_ids.count(x))
					break;
			}
			used_ids.insert(x);
			fout << cells[x] << endl;
		}
		fin.close();
		fout.close();
	}
}

void Exp::genDBData(double fraction)
{
	string files[] = {"area"};
	for (string file : files)
	{
		ifstream fin("data/" + file + "_names/" + file + "_names.txt");
		vector<string> cells_full, cells;
		string cell;
		for (string cell; getline(fin, cell); )
			cells_full.push_back(cell);
		fin.close();
		auto n = cells_full.size() * fraction;
		for (auto i = 0; i < n; i ++)
			cells.push_back(cells_full[i]);

		//generate rules
		struct timeval t1, t2;
		gettimeofday(&t1, NULL);
		cerr << "Generating rules......" << endl;
		RuleGenerator *ruleGenerator;
		ruleGenerator = new BadBoy(cells);
		vector<t_rule> rules = ruleGenerator->gen_rules();
		for (int i = 0, m = (int) rules.size(); i < m; i ++)
			rules.emplace_back(make_pair(rules[i].second, rules[i].first));

		//construct the joiner
		Common::JAC_THRESHOLD = 0.7;
		PolynomialJoiner *joiner = new PolynomialJoiner(rules, cells);

		//generate text file for name table
		ofstream fout1("exp/db/" + to_string(fraction) + file + "_name.txt");
		for (auto i = 0; i < n; i ++)
			fout1 << i << '\t' << cells[i] << endl;
		fout1.close();

		//generate text file for signature tables
		ofstream fout2("exp/db/" + to_string(fraction) + file + "_osig.txt");
		ofstream fout3("exp/db/" + to_string(fraction) + file + "_tsig.txt");
		int osig_id = 0;
		int tsig_id = 0;
		for (auto i = 0; i < n; i ++)
		{
			//osig
			unordered_set<string> o_sigs = joiner->get_o_sigs(i);
			for (string sigstr : o_sigs)
				fout2 << osig_id ++ << '\t' << i << '\t' << sigstr << endl;
			//tsig
			unordered_set<string> t_sigs = joiner->get_t_sigs(i);
			for (string sigstr : t_sigs)
				fout3 << tsig_id ++ << '\t' << i << '\t' << sigstr << endl;
		}
		fout2.close();
		fout3.close();

		//generate tex file for app_rule table
		ofstream fout4("exp/db/" + to_string(fraction) + file + "_apprule.txt");
		for (auto i = 0; i < n; i ++)
		{
			vector<t_rule> app_rules = joiner->get_applicable_rules(i);
			string app_rule_str = "";
			for (t_rule rule : app_rules)
			{
				string cur_lhs = "", cur_rhs = "";
				for (auto t : rule.first)
					cur_lhs += t + " ";
				for (auto t : rule.second)
					cur_rhs += t + " ";
				app_rule_str += cur_lhs + "|" + cur_rhs + "**";
			}
			fout4 << i << '\t' << i << '\t' << app_rule_str << endl;
		}
		fout4.close();
	}
}
