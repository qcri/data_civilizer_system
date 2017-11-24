#ifndef _COMMON_H_
#define _COMMON_H_

#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <map>
#include <fstream>
#include <sys/time.h>
#include <algorithm>
#include <regex>

#include <unordered_map>
#include <unordered_set>

using namespace std;

#define INT_TYPE_THRESH 0.8
#define UNIQUE_THRESH 0.8

#define EPS 1e-6
// #define OUTPUT_INFO

/******************** TECHNIQUE CONTROL MACRO************/

// #define BOOST_DUP_COUNTS_ENABLE
// #define REAPPLY_RULE_ENABLE


#ifdef DATASET_ADDRESS
// for address dataset
#define SIMILARITY_CHECK_ENABLE
#define SIMILARITY_THRESHOLD 0.48
#define MAX_PATH_LENGTH 6     // the length of the transformation is at most MAX_PATH_LENGTH - 1 after trimming
#define PRUNE_BY_STRUCTURE_THRESHOLD 5
#define PRUNE_BY_STRUCTURE_GROUP_NUM 30000
#define MAX_CONSTANT_LENGTH 1000
#elif defined(DATASET_AUTHOR)
#define MAX_PATH_LENGTH 5     // the length of the transformation is at most MAX_PATH_LENGTH - 1 after trimming
#define PRUNE_BY_STRUCTURE_THRESHOLD 5
#define PRUNE_BY_STRUCTURE_GROUP_NUM 30000
#define MAX_CONSTANT_LENGTH 1000
#elif defined(DATASET_TITLE)
#define MAX_PATH_LENGTH 5     // the length of the transformation is at most MAX_PATH_LENGTH - 1 after trimming
#define PRUNE_BY_STRUCTURE_THRESHOLD 5
#define PRUNE_BY_STRUCTURE_GROUP_NUM 2000
#define MAX_CONSTANT_LENGTH 100
#elif defined(DATASET_LONG)
#define LCS_CHECK_ENABLE
#define LCS_THRESHOLD 0.6
#define SIZE_FILTER_THRESHOLD 0.6
#define MAX_PATH_LENGTH 5     // the length of the transformation is at most MAX_PATH_LENGTH - 1 after trimming
#define PRUNE_BY_STRUCTURE_THRESHOLD 5
#define PRUNE_BY_STRUCTURE_GROUP_NUM 2000
#define MAX_CONSTANT_LENGTH 10
#else
#define MAX_PATH_LENGTH 5     // the length of the transformation is at most MAX_PATH_LENGTH - 1 after trimming
#define PRUNE_BY_STRUCTURE_THRESHOLD 5
#define PRUNE_BY_STRUCTURE_GROUP_NUM 10000
#define MAX_CONSTANT_LENGTH 10
#endif


// #define REVERSE_MAPPING_DIR_ENABLE   // only use 0-1 of the 2 MACROs on the left
// #define RANDOM_MAPPING_DIR_ENABLE
// #define LONG_FIRST_MAPPING_ENABLE

// #define NO_AGGREGATION_ENABLE
// #define STRUCTURE_AGGREGATION_ENABLE
// #define BOTH_AGGREGATION_ENABLE
// #define TRANSFORM_AGGREGATION_ENABLE

// #define LOCAL_THRESHOLD_ENABLE   // can use any of them, no restrict
// #define GLOBAL_THRESHOLD_ENABLE
// #define UNIQUE_THRESHOLD_ENABLE


#define GLOBAL_FREQUENCY_THRESHOLD 10
#define LOCAL_FREQUENCY_THRESHOLD 0.7

#define MAX_NUMBER_OF_RULES 1000000
#define PRUNE_BY_STRUCTURE_ENABLE
// #define INCREMENTAL_UPDATE_RULE_ENABLE


// #define RULE_UPDATE_ENABLE
// #define RULE_CONTAINMENT_ENABLE   // TODO THIS ONE

// #define STATIC_ORDERING_ENABLE   // do not DEFINE STATIC_ORDERING_ENABLE while undefine  SINGLE_CONSTANT_TERM_ENABLE
// #define SINGLE_CONSTANT_TERM_ENABLE
// #define PREFIX_SUFFIX_TERM_ENABLE

/******************** TECHNIQUE CONTROL MACRO************/

#define AGG_REGEX_NUM 4


using namespace std;

struct pair_hash {
  template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
      size_t res = 1009;
      res = res * 9176 + std::hash<T1>{}(p.first);
      res = res * 9176 + std::hash<T2>{}(p.second);
      return res;
    }
};
 
class Wrapper 
{
  public:

    static int maxLength;
    static vector<vector<int>> matrix;

    // Regex based find and replace
    static string FindReplace(const string &str, const regex &exp, const string &replace)
    {
      int prev_pos = 0;
      string stripped = "";

      smatch res;
      string::const_iterator searchStart(str.cbegin());
      while (regex_search(searchStart, str.cend(), res, exp))
      {
        int base = distance(str.cbegin(), searchStart);
        stripped += str.substr(base, res.position());
        stripped += replace;
        searchStart += res.position() + res.length();
      }
      int base = distance(str.cbegin(), searchStart);
      stripped += str.substr(base, str.length() - base + 1);

      return stripped;
    }

    static string agg_regex_str[];
    static string agg_replace_str[];

    static void logTime(timeval &begin, timeval &end, const string &log) {
      gettimeofday(&end, NULL);
      fprintf(stderr, ("# " + log + ": %.3fs\n").c_str(), end.tv_sec - begin.tv_sec +
          (end.tv_usec - begin.tv_usec) * 1.0 / CLOCKS_PER_SEC);
    }

    static regex agg_regexes[];

    static void print_green(const string &s)
    {
      cout << "\033[1;32m" << s << "\033[0m";
    }

    static void strToTokens(const string &s, vector<string> &res, const string &delims) {
      string::size_type begIdx, endIdx;
      begIdx = s.find_first_not_of(delims);
      while (begIdx != string::npos) {
        endIdx = s.find_first_of(delims, begIdx);
        if (endIdx == string::npos)
          endIdx = s.length();
        res.push_back(s.substr(begIdx, endIdx - begIdx));
        begIdx = s.find_first_not_of(delims, endIdx);
      }
    }


   template<typename T>
   static void LCSLength(const T &s1, const T &s2, int l1, int l2)
   {
     if (matrix.empty())
     {
#ifdef OUTPUT_INFO
       cout << "max length: " << maxLength << endl;
#endif
       matrix.resize(maxLength + 1);
       for (auto i = 0; i < maxLength + 1; i++)
         matrix[i].resize(maxLength + 1);
     }

     for (auto i = 0; i <= l1; i++) matrix[i][0] = 0;
     for (auto j = 0; j <= l2; j++) matrix[0][j] = 0;

     for (auto i = 1; i <= l1; i++)
       for (auto j = 1; j <= l2; j++)
         if (s1[i - 1] == s2[j - 1])
           matrix[i][j] = matrix[i - 1][j - 1] + 1;
         else
           matrix[i][j] = std::max(matrix[i][j - 1], matrix[i - 1][j]);
   }

  template<typename T>
  static void printDiff(const T &s1, const T &s2, int i, int j, bool flag, vector<pair<pair<int, int>, pair<int, int>>> &results)
  {
    if (i > 0 && j > 0 && s1[i - 1] == s2[j - 1])
    {
      if (flag == true)
      {
        results.back().first = make_pair(i, j);
        flag = false;
      }
      printDiff(s1, s2, i - 1, j - 1, flag, results);
    }
    else if (i > 0 || j > 0)
    {
      if (flag == false)
      {
        results.emplace_back(make_pair(-1, -1), make_pair(i, j));
        flag = true;
      }
      if (j > 0 && (i == 0 || matrix[i][j - 1] >= matrix[i - 1][j]))
        printDiff<T>(s1, s2, i, j - 1, flag, results);
      else if (i > 0 && (j == 0 || matrix[i][j - 1] < matrix[i - 1][j]))
        printDiff<T>(s1, s2, i - 1, j, flag, results);
    }
    else if (flag == true)
      results.back().first = make_pair(i, j);
  }

  template<typename T>
  static void Alignment(const T &input, const T &output, int l1, int l2, vector<pair<pair<int, int>, pair<int, int>>> &results, bool isprint = false)
  {
    LCSLength<T>(input, output, l1, l2);
    printDiff<T>(input, output, l1, l2, false, results);

    if (isprint)
      for (auto &entry : results)
        printf("(%d, %d) -- (%d, %d)\n", entry.first.first, entry.first.second, entry.second.first, entry.second.second);
  }
};
#endif
