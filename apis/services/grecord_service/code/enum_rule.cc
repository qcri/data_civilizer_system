
#include "Consolidation.h"

template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

int main(int argc, char ** argv)
{
  if (argc < 2)
  {
    cerr << "Wrong arguments." << endl;
    cerr << "Usage: ./enum_rule dataset_folder attribute_name rule_types[1-5] enable_auto_comfirm [0,1]" << endl;
    cerr << "Exampel: ./enum_rule ../data/book/ author 2 0 1" << endl;
    exit(-1);
  }

  string csvfilepath = string(argv[1]);
  string outfilepath = string(argv[2]);

  string cname = "cluster_id";
  Consolidation goldenRecord(csvfilepath, cname);

  for (auto i = 0; i < goldenRecord.number_of_tables; i++)
  {
    int num_of_col = stoi(goldenRecord.TryNextTable(i));
    for (int col_id = 0; col_id < num_of_col; col_id++) 
    {
      if (col_id == goldenRecord.cluster_id_col) continue;

      string message1 = goldenRecord.ProfileColumn(i, col_id);
      cout << "message1: " << message1;

      string skip;
      getline(cin, skip);
      if (skip != "1" && skip != "2" && skip != "3" && skip != "4" && skip != "5")  continue;

      string message2 = goldenRecord.TryNextColumn(i, col_id, skip);
      cout << "message2: " << message2;

      int applied_group_num = 0;
      while (true) 
      {
        string message3 = goldenRecord.ShowNextCluster();
        cout << "message 3:" << message3;

        vector<string> elems;
        split(message3, '\t', std::back_inserter(elems));
        cout << "max_group_id : " << elems[0] << endl;
        cout << "max_count :" << elems[1] << endl;
        int max_group_id = stoi(elems[0]);

        string xx; 
        getline(cin, xx);
        if (xx == "4" || elems[1] == "0")
        {
          cout << "Done with current column " << endl;
          break;
        }
        // parse max_group_id

        string message4 = goldenRecord.ApplyCluster(i, col_id, applied_group_num++, max_group_id, xx);
        cout << "message 4:" << message4;
      }
    }
    goldenRecord.MaterializeTable(i, outfilepath);
    cout << "Successfully Exit!" << endl;
    break;
  }
  // goldenRecord.ConsolidationGo();
}

