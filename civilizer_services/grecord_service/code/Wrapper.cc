
#include "Wrapper.h"

vector<vector<int>> Wrapper::matrix;
int Wrapper::maxLength = 0;

regex Wrapper::agg_regexes[] = {
        regex("[A-Z]+"),  // caps
        regex("[a-z]+"),  // lower case
        regex("[0-9]+"),  /// digitst
        regex("\\s+") // whitespace
};

string Wrapper::agg_regex_str[] = {
        "Capitals",
        "Lower case",
        "Digitals",
        "White Space"
};

string Wrapper::agg_replace_str[] = {
        "<CAP>",
        "<low>",
        "<num>",
        "<whs>"
};

