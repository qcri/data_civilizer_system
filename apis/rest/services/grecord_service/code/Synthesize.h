//
// Created by dongdeng on 6/26/17.
//

#ifndef ENUM_RULE_SYNTHESIZE_H
#define ENUM_RULE_SYNTHESIZE_H

#include "Aggregator.h"
#include "Wrapper.h"

#include <regex>
#include <map>
#include <assert.h>


#define SUBSTR_SCORE 1.5
#define CPOS_SCORE 0.8
#define CONSTR_SCORE 0.1
#define CONSTANT_TOKEN_RATIO 1.1
#define NEG_INF -100000000


#define REGEX_SIZE 8
#define ALL_REGEX_SIZE 11

class Aggregator;
class Synthesizer;

enum Regex : int {
  PROPER_CASE = 0,
  CAPS = 1,
  LOWER_CASE = 2,
  DIGITS = 3,
  ALPHABETS = 4,
  ALPHANUMERIC = 5,
  WHITESPACE = 6,
  PUNCT = 7,
  CONSTANT = 8,
  START_T = 9,
  END_T = 10,
  SPECIAL_CASE_FOR_DAG_EDGE = 11
};

const regex regexes[] = {
        regex("[A-Z][a-z]+"), // proper_case
        regex("[A-Z]+"),  // caps
        regex("[a-z]+"),  // lower case
        regex("[0-9]+"),  /// digitst
        regex("[a-zA-Z]+"),  // alphabets
        regex("[a-zA-Z0-9]+"),  //  alphanum
        regex("\\s+"), // whitespace
        regex("[:.,;'\"&*-+=/]+"), // punctuation
};

const string regex_str[] = {
        "Proper_case",
        "Capitals",
        "Lower case",
        "Digitals",
        "Alphabets",
        "Alphanums",
        "White Space",
        "Punctuation",
        "Constant",
        "Start_Token",
        "End_Token",
        "Very_Special_for_Path"
};

const string start_special_token = "$START_T$";
const string end_special_token = "^END_T^";

// VLabel
class VLabel
{
public:
  int pid; // match id
  int pos; // actual position
  VLabel(int id, int p)
  {
    pid = id;
    pos = p;
  }
};

class Vertex
{
public:
  int vertex_id; // vertex id in the graph
  vector<VLabel> labels; // row_id, position

  Vertex(int vid, int rid, int pos)
  {
    vertex_id = vid;
    labels.emplace_back(rid, pos);
  }

  Vertex(int vid, const vector<VLabel> &l1, const vector<VLabel> &l2)
  {
    vertex_id = vid;
    labels = l2;
    labels.insert(labels.end(), l1.begin(), l1.end());
  }

  // friend ostream& operator<<(ostream& os, const Vertex& v);
};

struct PathHash;

// ELabel
class ELabel
{
  public:
  Regex regex;
  int index;
  string constr = "";

  ELabel()
  {
    regex = Regex::SPECIAL_CASE_FOR_DAG_EDGE;
  }
  ELabel(Regex r, int idx)
  {
    regex = r;
    index = idx;
  }
  ELabel(Regex r, int idx, const string &s)
  {
    regex = r;
    index = idx;
    constr = s;
  }
  bool operator==(const ELabel &o) const
  {
    return (constr == o.constr && index == o.index && regex == o.regex);
  }
  bool operator>(const ELabel &p) const
  {
    if (*this == p)
      return false;

    if (regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE) 
    {
      return false;
    }
    else if (p.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE)
    {
      return true;
    }
    else
    {
      if (regex > p.regex)
        return true;
      if (regex < p.regex)
        return false;

      if (abs(index) < abs(p.index))
        return true;
      if (abs(index) > abs(p.index))
        return false;

      if (index < p.index)
        return true;
      if (index > p.index)
        return false;

      if (constr > constr)
        return true;
      if (constr < constr)
        return false;

      return false;
    }
  }
};

// Edge
class Edge
{
  public:
  int src_id; // src vertex id
  int end_id; // end vertex id
  vector<ELabel> labels;

  Edge() {}
  Edge(int src, int end) {
    src_id = src;
    end_id = end;
  }
  Edge(int src, int end, Regex r, int idx) {
    src_id = src;
    end_id = end;
    labels.emplace_back(r, idx);
  }
  Edge(int src, int end, Regex r, int idx, const string &s) {
    src_id = src;
    end_id = end;
    labels.emplace_back(r, idx, s);
  }
  // friend ostream& operator<<(ostream& os, const Edge& e);
};

class Pos
{
  public:
  ELabel label;
  bool direction;

  Pos()
  {
    label.regex = Regex::CONSTANT;
    label.index = 0;
  }

  Pos(const ELabel &elabel, bool dir)
  {
    label = elabel;
    direction = dir;
  }
  Pos(Regex r, int i, bool dir)
  {
    label.regex = r;
    label.index = i;
    direction = dir;
  }
  Pos(Regex r, int i, const string &s, bool dir)
  {
    label.regex = r;
    label.index = i;
    label.constr = s;
    direction = dir;
  }
  bool operator==(const Pos &o) const
  {
    return (label == o.label && direction == o.direction);
  }
  bool operator!=(const Pos &o) const
  {
    return !(*this == o);
  }
  bool operator>(const Pos &p) const
  {
    if (*this == p)
      return false;

    if (label.regex != Regex::CONSTANT && p.label.regex != Regex::CONSTANT)
    {
      if (label.regex != Regex::SPECIAL_CASE_FOR_DAG_EDGE && p.label.regex != Regex::SPECIAL_CASE_FOR_DAG_EDGE)
      {
        if (label.regex > p.label.regex)
          return true;
        if (label.regex < p.label.regex)
          return false;

        if (abs(label.index) < abs(p.label.index))
          return true;
        if (abs(label.index) > abs(p.label.index))
          return false;

        if (label.index < p.label.index)
          return true;
        if (label.index > p.label.index)
          return false;

        if (direction > p.direction)
          return true;
        if (direction < p.direction)
          return false;

        if (label.constr > label.constr)
          return true;
        if (label.constr < label.constr)
          return false;

        return false;
      }
      else if (label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE && p.label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE)
      {
        if (abs(label.index) < abs(p.label.index))
          return true;
        if (abs(label.index) > abs(p.label.index))
          return false;

        if (label.index < p.label.index)
          return true;
        if (label.index > p.label.index)
          return false;

        if (direction > p.direction)
          return true;
        if (direction < p.direction)
          return false;

        if (label.constr > label.constr)
          return true;
        if (label.constr < label.constr)
          return false;

        return false;
      }
      else if (label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE)
      {
        return false;
      }
      else
      {
        return true;
      }
    }

    if (label.regex == Regex::CONSTANT && p.label.regex == Regex::CONSTANT)
    {
      if (label.constr.length() > p.label.constr.length())
        return true;
      if (label.constr.length() < p.label.constr.length())
        return false;

      if (label.constr > p.label.constr)
        return true;
      if (label.constr < p.label.constr)
        return false;

      if (abs(label.index) < abs(p.label.index))
        return true;
      if (abs(label.index) > abs(p.label.index))
        return false;

      if (label.index < p.label.index)
        return true;
      if (label.index > p.label.index)
        return false;

      if (direction > p.direction)
        return true;
      if (direction < p.direction)
        return false;

      return false;
    }

    if (label.regex == Regex::CONSTANT)
      return false;
    else
      return true;

    return false;
  }
  bool operator<(const Pos &p) const
  {
    if (*this == p)
      return false;
    else
      return !(*this > p);
  }
  friend ostream& operator<<(ostream& os, const Pos& p);
};


struct ELabelHash
{
  size_t operator()(const ELabel &o) const
  {
    size_t res = 1009;
    res = res * 9176 + o.index;
    res = res * 9176 + static_cast<int>(o.regex);
    if (!o.constr.empty())
      res = res * 9176 + std::hash<string>()(o.constr);
    return res;
  }
};

namespace std
{
  template<>
  struct hash<Pos>
  {
    size_t operator()(const Pos &o) const
    {
      size_t res = 1009;
      res = res * 9176 + o.label.index;
      res = res * 9176 + static_cast<int>(o.label.regex);
      if (!o.label.constr.empty())
        res = res * 9176 + std::hash<string>()(o.label.constr);
      res = res * 9176 + static_cast<int>(o.direction);
      return res;
    }
  };
}


// Graph
class Graph
{
public:
  vector<Edge> edges;
  vector<Vertex> nodes;
  unordered_map<int, Pos> all_pos;

  vector<vector<int>> node2edge;
  vector<vector<int>> rev_node2edge;
  unordered_map<ELabel, pair<int, int>, ELabelHash> umap_labels;

  void UmapIndex();
  void GetMatchId(const string &str, int &mid, const Regex r, const regex &exp, unordered_map<pair<int, int>, vector<ELabel>, pair_hash> &match_map);
  void GenGraphStr(const string &val, int id, const unordered_map<string, pair<double, int>> &valid_terms);
  void GenGraphStr(const string &val, int id);
  void getPosList(vector<vector<Pos>> &posList);
  void getAdjacentMatrix();
  // bool TestPositionValid(const Pos &test);
  bool TestContainment(const vector<pair<Pos, Pos>> &path, const string &target, const string &source);
};


class Elem
{
  public:
  int pid;  // match id
  int beg;
  int end;
  Elem(int r, int b, int e)
  {
    pid = r;
    beg = b;
    end = e;
  }
  bool operator<(const Elem &e) const
  {
    if (pid < e.pid)
      return true;
    if (pid > e.pid)
      return false;
    if (beg < e.beg)
      return true;
    if (beg > e.beg)
      return false;
    return end < e.end;
  }
  bool operator>(const Elem &e) const
  {
    if (pid > e.pid)
      return true;
    if (pid < e.pid)
      return false;
    if (beg > e.beg)
      return true;
    if (beg < e.beg)
      return false;
    return end > e.end;
  }
  inline bool operator==(const Elem &e) const
  {
    return (pid == e.pid && beg == e.beg && end == e.end);
  }
};



class DagEdge
{
  public:
  int src_id;  // vertex id
  int end_id;  // vertex id

  string constant = "";  // ConstantStr
  vector<ELabel> prefixes;
  vector<ELabel> suffixes;
  vector<ELabel> inffixes;
  vector<pair<int, int>> const_pos;  // SubStr(CPos, CPos)
  vector<pair<vector<Pos>, vector<Pos>>> input_pos;  // SubStr(MPos, MPos)

  double weight = 0.0; // for static ranking

  DagEdge()
  {
    src_id = end_id = -99; // should be reset later
  }
  DagEdge(int src, int end)
  {
    src_id = src;
    end_id = end;
  }
  DagEdge(int src, int end, const string &str)
  {
    src_id = src;
    end_id = end;
    constant = str;
  }
  // friend ostream& operator<<(ostream& os, const DagEdge& e);
};

class Path;

class DAGraph
{
public:
  vector<Vertex> nodes;
  vector<DagEdge> edges;

  int rule_id;
  string input;
  string output;

  Graph input_graph;

  int starting_node_id, ending_node_id;
  vector<vector<int>> dag_edge_adj;

  uint64_t graphSize = 0;
  vector<uint64_t> nodeSize;
  vector<uint64_t> edgeSize;

  DAGraph(const string &in, const string &out, const int id,  const unordered_map<string, pair<double, int>> &valid_terms)
          : input(in), output(out), rule_id(id)
  {
#if defined(STATIC_ORDERING_ENABLE) || defined(SINGLE_CONSTANT_TERM_ENABLE)
    input_graph.GenGraphStr(input, rule_id, valid_terms);
#else
    input_graph.GenGraphStr(input, rule_id);
#endif
    input_graph.getAdjacentMatrix();
    GenerateDagEnhance(valid_terms);
    getAdjacentMatrix();
  }

  // void StaticRanking(vector<pair<Pos, Pos>> &path);
  void DynamicRanking(Path &path, Synthesizer &synsizer);
  // void print_dag_graph();

  uint64_t getGraphSize();

  private:

  void getAdjacentMatrix();

  uint64_t getEdgeSize(int eid);
  // void topologicalSort(const vector<vector<int>> &adj, vector<int> &result);
  // void topologicalSortUtil(int v, vector<bool> &visited, const vector<vector<int>> &adj, vector<int> &result);
  bool PickBest(const vector<Elem> &list1, const vector<Elem> &list2, vector<Elem> &best_list, int local_threshold);
  void GenerateDag(unordered_map<string, int> &frequency);
  // void GenerateDag(const unordered_map<string, pair<double, int>> &valid_terms);
  void GenerateDagEnhance(const unordered_map<string, pair<double, int>> &valid_terms);
  void DeepFirstSearch(const int src_id, const int end_id,
                       vector<Elem> &curList, Path &curPath,
                       vector<Elem> &maxList, Path &maxPath,
                       Synthesizer &synsizer, int &local_threshold);

#ifdef PATH_LENGTH_THREE
  int max_path_length = 4;
#elif defined(PATH_LENGTH_FOUR)
  int max_path_length = 5;
#elif defined(PATH_LENGTH_FIVE)
  int max_path_length = 6;
#else
  int max_path_length = MAX_PATH_LENGTH;
#endif
};


class Synthesizer
{
  public:

  // inverted index
  unordered_map<string, vector<Elem>> constr_index;
  unordered_map<pair<int, int>, vector<Elem>, pair_hash> cpos_index;
  unordered_map<pair<Pos, Pos>, vector<Elem>, pair_hash> substr_index;
  unordered_map<ELabel, vector<Elem>, ELabelHash> prefix_index;
  unordered_map<ELabel, vector<Elem>, ELabelHash> suffix_index;

  vector<DAGraph> dags;
  vector<int> thresholds;
  const vector<pair<pair<string, string>, int>> &rules;

  Synthesizer(const vector<pair<pair<string, string>, int>> &r, const unordered_map<string, pair<double, int>> &valid_terms)
    : rules(r)
  {

#ifdef OUTPUT_INFO
    timeval t1, t2;
    gettimeofday(&t1, NULL);
#endif

    Indexing(valid_terms);

#ifdef OUTPUT_INFO
    logTime(t1, t2, "Finished Indexing. ");
    for (auto &entry : r)
    {
      const string &lhs = entry.first.first;
      const string &rhs = entry.first.second;
      cout << lhs << " --- " << rhs << endl;
    }
#endif
    /*
    string str;
    cout << "printing scores?" << endl;
    getline(cin, str);
    if (str != "0")
    {
      for (auto &entry : r)
      {
        const string &lhs = entry.first.first;
        const string &rhs = entry.first.second;
        cout << lhs << " --- " << rhs << endl;

        for (auto i = 0; i < lhs.length(); i++) 
        {
          cout << endl << "checking " << i << endl;
          for (auto j = 0; j < i; j++) 
          {
            cout << lhs.substr(j, i - j) << ": ";
            auto it = valid_terms.find(lhs.substr(j, i - j));
            if (it == valid_terms.end())
              cout << "0" << endl;
            else
              cout << it->second.first << " " << it->second.second << endl;
          }
          for (auto j = i + 1; j <= lhs.length(); j++) 
          {
            cout << lhs.substr(i, j - i) << ": ";
            auto it = valid_terms.find(lhs.substr(i, j - i));
            if (it == valid_terms.end())
              cout << "0" << endl;
            else
              cout << it->second.first << " " << it->second.second << endl;
          }
        }

        for (auto i = 0; i < rhs.length(); i++) 
        {
          cout << endl << "checking " << i << endl;
          for (auto j = 0; j < i; j++) {
            cout << rhs.substr(j, i - j) << ": ";
            auto it = valid_terms.find(rhs.substr(j, i - j));
            if (it == valid_terms.end())
              cout << "0" << endl;
            else
              cout << it->second.first << " " << it->second.second << endl;
          }
          for (auto j = i + 1; j <= rhs.length(); j++) 
          {
            cout << rhs.substr(i, j - i) << ": ";
            auto it = valid_terms.find(rhs.substr(i, j - i));
            if (it == valid_terms.end())
              cout << "0" << endl;
            else
              cout << it->second.first << " " << it->second.second << endl;
          }
        }
      }
    }
    */
  }

  void Indexing(const unordered_map<string, pair<double, int>> &valid_terms);
  void InvIndex(const vector<DAGraph> &dags);
  void SynAggregating(unordered_map<Path, vector<int>, PathHash> &groups, const vector<int> &rule_id_maps);

  // void Statistics();
  // void SurveyGraph();
  // void PosIndex(const vector<DAGraph> &dags);

};

class Path 
{
public:
  vector<pair<Pos, Pos>> path;

  bool operator==(const Path &p2) const
  {
    if (path.size() != p2.path.size())
      return false;

    for (auto i = 0; i < path.size(); i++)
    {
      if (path[i].first != p2.path[i].first)
        return false;
      if (path[i].second != p2.path[i].second)
        return false;
    }
    return true;
  }
  bool operator!=(const Path &p2) const
  {
    return !(*this == p2);
  }
  bool operator<(const Path &p2) const
  {
    // the shorter the better the larger
    if (path.size() < p2.path.size())
      return false;
    if (path.size() > p2.path.size())
      return true;

    for (auto i = 0; i < path.size(); i++)
    {
      if (path[i].first != p2.path[i].first)
      {
        if (path[i].first < p2.path[i].first)
          return true;
        if (path[i].first > p2.path[i].first)
          return false;
      }
      if (path[i].second != p2.path[i].second)
      {
        if (path[i].second < p2.path[i].second)
          return true;
        if (path[i].second > p2.path[i].second)
          return false;
      }
    }
    return false;
  }
  bool operator>(const Path &p2) const
  {
    if (*this != p2)
      return !(*this < p2);
    else
      return false;
  }
};


struct PathHash
{
  size_t operator()(const Path &p) const
  {
    size_t res = 1009;
    for (auto i = 0; i < p.path.size(); i++)
    {
      res = res * 9176 + std::hash<Pos>()(p.path[i].first);
      res = res * 9176 + std::hash<Pos>()(p.path[i].second);
    }
    return res;
  }
};

#endif //ENUM_RULE_SYNTHESIZE_H
