/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/


#include "csv_reader.h"
#include "Profiler.h"
#include "common.h"
#include "DV_Detector.h"
#include "OD.h"
#include "Fast_DiMaC.h"
 long max_num_terms_per_att = 200;


 string splitpath(const std::string str, 
                  const char delimiter){
    int i, j, k;
    // cout << str << endl;
    for(i = str.length(); i >= 0; i --){
        if (delimiter == str[i])
            break;
    }
    // cout << "i = " << i << "   string length = " << str.length() << endl;
    if ((i == 0) && (str[i] != delimiter)) i = -1;  
    std::string result = str.substr (i+1,str.length()-1);
    return result;
}

// // ========================================================================
// void Print_output_data(string tab_ref, string output_dir, string tab_name, std::vector<sus_disguised> sus_dis_values){
//     if (output_dir[output_dir.length()-1] != '/')
//         output_dir += '/';
//     char delim = '/';
//     string out_f_name = splitpath(tab_name, delim);
//     out_f_name = "DMV_"+out_f_name;
//     string out_file_name = output_dir + out_f_name;
//     fstream ofs(out_file_name, ios::out);
//     if (!ofs.good()){
//         cerr << "Problem opening output file .... \n" << out_file_name << endl;;
//         return;
//     }
//     if (sus_dis_values.size() < 1)
//         return;
//     ofs << "table reference, attribute name, DMV, frequency" << endl;
//     ofs << tab_ref << "," <<sus_dis_values[0].attr_name << "," 
//              << check_d_quotation(sus_dis_values[0].value) << "," 
//              << sus_dis_values[0].frequency << endl;
//              // << "," << sus_dis_values[0].tool_name << endl;
//     for (long i = 1; i < (long)sus_dis_values.size(); i++)
//         ofs << "," <<sus_dis_values[i].attr_name << "," 
//              << check_d_quotation(sus_dis_values[i].value) << "," 
//              << sus_dis_values[i].frequency << endl;
//              // << "," << sus_dis_values[i].tool_name << endl;
    
//     ofs.close();
// }


// ========================================================================
void Print_output_data(string tab_ref, string output_dir, string tab_name, std::vector<sus_disguised> sus_dis_values){
    if (output_dir[output_dir.length()-1] != '/')
        output_dir += '/';
    char delim = '/';
    string out_f_name = splitpath(tab_name, delim);
    out_f_name = "DMV_"+out_f_name;
    string out_file_name = output_dir + out_f_name;
    fstream ofs(out_file_name, ios::out);
    if (!ofs.good()){
        cerr << "Problem opening output file .... \n" << out_file_name << endl;;
        return;
    }
    if (sus_dis_values.size() < 1)
        return;
    ofs << "table reference, attribute name, DMV, frequency" << endl;
    ofs << tab_ref << "," <<sus_dis_values[0].attr_name << "," 
             << check_d_quotation(sus_dis_values[0].value) << "," 
             << sus_dis_values[0].frequency
             << "," << sus_dis_values[0].tool_name << endl;
    for (long i = 1; i < (long)sus_dis_values.size(); i++)
        ofs << "," <<sus_dis_values[i].attr_name << "," 
             << check_d_quotation(sus_dis_values[i].value) << "," 
             << sus_dis_values[i].frequency
             << "," << sus_dis_values[i].tool_name << endl;
    
    ofs.close();
}



// ================The main Function====================================
extern "C"
void execute(char* table_ref, char * table_name, char * out_directory) {
    // if (argc != 4)
    // {
    //   cout << "Wrong number of arguments entered (" << argc << ") \n";
    //   for (int k = 0; k < argc; k++)
    //     cerr << argv[k] << endl;
    //   cout << "Usage (" << argv[0] << "): <data file reference> <table_name> <output dir>"
    //           "\n\n";
    //   return 1;
    // }
    string tab_ref = string(table_ref); 
    string file_name = string(table_name);
    string output_dir = string(out_directory);

    cout << tab_ref << "::" << file_name << "::" << output_dir << endl;
    // string s = "ERIN";
    // transform( s.begin(), s.end(), s.begin(), ::tolower );
    // cout << "Checking " << s << "\t" << check_str_repetition(s) << endl;
    // return ;
    string full_output_path = realpath(output_dir.c_str(), NULL);

    std::vector<sus_disguised> sus_dis_values;

    // fstream ofs(output_file_name, ios::out);
    // if (!ofs.good())
    // {
    //   cerr << "Unable to open output file .. \n";
    //   return 0;
    // }
    doubleVecStr P;
    // cout << "Check 1" << endl;
    CSV_READER *dataReader = new CSV_READER();  
    Table T = dataReader->read_csv_file(file_name);
    // cerr << T.data.size() << "::" << T.data[0].size() << endl;
    F_DiMaC FDiMaC;
    

    DataProfiler * dvdDataProfiler = new DataProfiler();
    
    TableProfile TP;
    vector<struct item> most_common;
    vector<map<string, long> > tablehist =  dvdDataProfiler->TableHistogram(T);
    // for (long i = 0; i < tablehist.size(); i++)
    //     cout << T.header[i] << " has (" << tablehist[i].size() << ") distinct values" << endl;
    // cerr << "Starting Fast_DiMaC .....\n";
    sus_dis_values = FDiMaC.find_disguised_values(T, tablehist, max_num_terms_per_att);
    // return 0;
    // cerr << "Fast_DiMaC Finished .....\n";
    // cerr << "Frequency computed .. \n";
    TP = dvdDataProfiler->profile_table(T, tablehist, max_num_terms_per_att);
    // cerr << "Profiling is done .. <" << TP.profile.size() << ">\n";
    // for (long i = 0; i < TP.profile.size(); i++){
    //     ofs << "(" << TP.header[i] << ") has " << 
    //             TP.profile[i].num_distinct_values << " distinct value(s)" 
    //             << endl;
        
    //     ofs << "num numerics = " << TP.profile[i].num_numerics 
    //         << " distinct numerics = " << TP.profile[i].distinct_Numbers.size()
    //         << endl;
    //     ofs << "num nulls = " << TP.profile[i].num_nulls << endl;
    //     ofs << "num strings = " << TP.profile[i].num_strings 
    //         << " distinct strings = " << TP.profile[i].distinct_Strings.size()
    //         << endl;
    //     ofs << "=================================\n";
    // }
    // ofs.close();

    
    DV_Detector DVD;
    DVD.check_non_conforming_patterns(TP, tablehist, sus_dis_values);
    // cerr << "Checking non conforming patterns is done .. <" << TP.profile.size() << ">\n";
    OD od;
    od.detect_outliers(TP, sus_dis_values);
    // cerr << "Outliers are detected .. \n";
    Print_output_data(tab_ref, full_output_path, T.table_name, sus_dis_values);
             // << sus_dis_values[i].score << endl;
    // for (int i = 0; i < TP.number_of_atts; i ++){
    //     cout << TP.header[i] << endl;
    //     // if (TP.header[i] == "Plasma glucose concentration"){
    //     map<string, long>::iterator string_itr = TP.profile[i].common_Strings.begin();
    //     while (string_itr != TP.profile[i].common_Strings.end()){
    //         cout << string_itr->first << '\t' << string_itr->second << endl;
    //         string_itr ++;
    //     }
    //     cout << "=================================\n";   
    // }
    // cerr << "Freqs = " << TP.profile[7].freq_of_freq.size() << endl;
    // map<double, long>::iterator dbl_itr = TP.profile[7].freq_of_freq.begin();
    // for (;dbl_itr != TP.profile[7].freq_of_freq.end(); dbl_itr++)
    //     cout << dbl_itr->first << '\t' << dbl_itr->second << endl;
    // cout << "=================================\n"; 

    // map<string, long>::iterator s_itr = tablehist[7].begin();
    // for (;s_itr != tablehist[7].end(); s_itr++)
    //     cout << s_itr->first << '\t' << s_itr->first.length() << '\t' << s_itr->second << endl;
    // cout << "=================================\n";
}
