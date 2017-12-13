/**************************************
 **** 2017-7-25      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/


#ifndef _Fast_DiMaC_
#define _Fast_DiMaC_



#include "Table.h"
#include "common.h"
// #include "csv_reader.h"
typedef vector<map <string, doubleVecStr> > DiMaC_Index;
class F_DiMaC {
private:
    DiMaC_Index DiMaC_Index_T;
public:
    F_DiMaC();
    F_DiMaC(const Table & T) {  Table_Index_DiMaC(T, DiMaC_Index_T);  }
    void Table_Index_DiMaC(const Table & T, DiMaC_Index & IDX);
    // Table SELECT(const Table & T, const string & item, const long & idx);
    double subtable_correlation(const Table & , const string,
                                const long , long & );
    bool prune_attribute(const long idx, const long len, vector<map<string, long> > & M);
    vector< sus_disguised > find_disguised_values(const Table & T,
                                    vector<map<string, long> > & tablehist,
                                    long max_num_terms_per_att);
    long compute_num_cooccur(const vector<string> &, const long &);
    long compute_num_occur(string v, long A);
    long compute_num_cooccur(const vector<string> &, const long &, DiMaC_Index &);
    long compute_num_occur(string v, long A, DiMaC_Index & DiMaC_Index_subT);
    double record_correlation(const vector<string> & Vec, const Table & T, const Table & TP, 
                                        DiMaC_Index & DiMaC_Index_subT, const long idx);
   	// double Attribute_correlation(const Table & T, const Table & TP, DiMaC_Index & DiMaC_Index_subT,
    //                                     const long idx);
    // double compute_num_occur (string v);
};
#endif