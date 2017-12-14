//
// Created by dongdeng on 6/26/17.
//

#include "Synthesize.h"

ostream& operator<<(ostream& os, const Pos& p) 
{
  os << "[$" << p.label.constr << "$, " << p.label.index << ", " << regex_str[p.label.regex] << ", " << p.direction << "]";
  return os;
}

void Graph::getPosList(vector<vector<Pos>> &posList)
{
  posList.assign(this->nodes.size(), vector<Pos>());

#if defined(STATIC_ORDERING_ENABLE) && defined(SINGLE_CONSTANT_TERM_ENABLE)

  for (auto &entry : all_pos)
    posList[entry.first].push_back(entry.second);

#elif !defined(STATIC_ORDERING_ENABLE) && !defined(SINGLE_CONSTANT_TERM_ENABLE)

  for (auto &edge : this->edges)
    for (auto &elabel : edge.labels)
    {
      posList[edge.src_id].emplace_back(elabel, false);
      posList[edge.end_id].emplace_back(elabel, true);
    }

#elif defined(SINGLE_CONSTANT_TERM_ENABLE)

  for (auto &entry : all_pos)
    posList[entry.first].push_back(entry.second);
  for (auto &edge : this->edges)
    for (auto &elabel : edge.labels)
    {
      posList[edge.src_id].emplace_back(elabel, false);
      posList[edge.end_id].emplace_back(elabel, true);
    }

#else

  cout << "DO NOT UNDEFINE THE MACRO SINGLE_CONSTANT_TERM_ENABLE WHILE DEFINE THE MACRO STATIC_ORDERING_ENABLE" << endl;
  assert(false);

#endif
}


void Graph::getAdjacentMatrix()
{
  int num_vertex = nodes.size();
  node2edge.resize(num_vertex);
  rev_node2edge.resize(num_vertex);
  for (auto i = 0; i < edges.size(); i++)
  {
    node2edge[edges[i].src_id].push_back(i);
    rev_node2edge[edges[i].end_id].push_back(i);
  }
}

void Graph::GetMatchId(const string &str, int &mid, const Regex r, const regex &exp, unordered_map<pair<int, int>, vector<ELabel>, pair_hash> &match_map)
{
  smatch res;
  string::const_iterator searchStart(str.cbegin());
  while (regex_search(searchStart, str.cend(), res, exp))
  {
    mid += 1;
    int base = distance(str.cbegin(), searchStart);
    match_map[make_pair(base + res.position(), base + res.position() + res.length())].emplace_back(r, mid);
    searchStart += res.position() + res.length();
  }
}


/*
bool Graph::TestPositionValid(const Pos &test)
{
  if (test.label.regex == Regex::CONSTANT)
  {
    if (test.label.constr.empty())
      return false;
    if (test.label.index == 0)
      return false;
    if (abs(test.label.index) > constants[test.label.constr])
      return false;
  }
  else if (test.label.regex == Regex::START_T || test.label.regex == Regex::END_T)
  {
    if (abs(test.label.index) != 1)
      return false;
  }
  else if (test.label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE)
  {
    if (test.label.constr.empty() && test.label.index < 0)
      return false;
  }
  return true;
}
 */


bool TestSegment(const string &segment, int &matching_pos, const string &target)
{
  int segment_length = segment.length();
  if (matching_pos + segment_length > target.length())
    return false;

  if (segment == target.substr(matching_pos, segment_length))
    matching_pos += segment_length;
  else
    return false;

  return true;
}

void Graph::UmapIndex()
{
  for (auto &e : edges)
  {
    for (auto &el : e.labels)
    {
      umap_labels[el] = make_pair(e.src_id, e.end_id);
    }
  }
}

bool Graph::TestContainment(const vector<pair<Pos, Pos>> &path, const string &target, const string &source)
{
  // test validility

  if (path.empty())
    return false;

  int matching_pos = 0;

  for (auto &entry : path)
  {
    if (entry.first.label.constr == "^END_T^" && entry.second.label.constr == "^END_T^" && matching_pos == target.length())
      return true;

    if (matching_pos >= target.length())
      return false;

    const auto &left_pos = entry.first;
    int begin_position = -1;

    if (left_pos.label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE)
    {
      // BUG TODO
      if (left_pos.label.index > 0)
      {
        begin_position = left_pos.label.index;
      }
      else if (left_pos.label.constr.empty())
      {
        return false;
      }
    }
    else if (left_pos.label.constr == "PREFIX")
    {
      // cout << "do nothing" << endl;
    }
    else if (left_pos.label.constr == "SUFFIX")
    {
      // cout << "do nothing" << endl;
    }
    else
    {
      if (!umap_labels.count(left_pos.label))
        return false;
      begin_position = umap_labels[left_pos.label].first;
      if (left_pos.direction)
        begin_position = umap_labels[left_pos.label].second;
    }

    const auto &right_pos = entry.second;
    int ending_position = -1;
    string substring = "";

    if (right_pos.label.regex == Regex::SPECIAL_CASE_FOR_DAG_EDGE) {
      if (!right_pos.label.constr.empty()) {
        if (!TestSegment(right_pos.label.constr, matching_pos, target))
          return false;
      } else if (right_pos.label.index >= 0) {
        if (begin_position <= 0)
          return false;

        int ending_position = right_pos.label.index;

        if (begin_position > source.length())
          return false;
        if (ending_position > source.length() + 1)
          return false;
        if (ending_position - begin_position <= 0)
          return false;

        string segment = source.substr(begin_position - 1, ending_position - begin_position);

        if (!TestSegment(segment, matching_pos, target))
          return false;
      } else {
        return false;
      }
    } else if (right_pos.label.constr == "PREFIX") {
      ELabel prefix_label(right_pos.label.regex, right_pos.label.index);
      if (!umap_labels.count(prefix_label))
        return false;

      int prefix_st = umap_labels[prefix_label].first;
      int prefix_en = umap_labels[prefix_label].second;

      if (matching_pos >= target.length())
        return false;

      if (source[prefix_st - 1] != target[matching_pos + 1])
        return false;

      for (auto pos_idx = prefix_st - 1; pos_idx < prefix_en - 2; pos_idx++) {
        if (matching_pos >= target.length())
          return false;
        if (source[pos_idx] != target[matching_pos])
          break;
        matching_pos++;
      }
    } else if (right_pos.label.constr == "SUFFIX") {
      ELabel suffix_label(right_pos.label.regex, right_pos.label.index);
      if (!umap_labels.count(suffix_label))
        return false;

      int suffix_st = umap_labels[suffix_label].first;
      int suffix_en = umap_labels[suffix_label].second;

      int first_begin_position = suffix_st;
      int segment_length = suffix_en - suffix_st - 1;

      while (segment_length > 0) {


        string src_seg = source.substr(first_begin_position, segment_length);
        if (matching_pos + segment_length <= target.length()) {
          string tgt_seg = target.substr(matching_pos, segment_length);
          if (src_seg == tgt_seg)
            break;
        }
        first_begin_position++;
        segment_length--;
      }
      if (segment_length > 0)
        matching_pos += segment_length;
      else
        return false;
    } else {

      if (!umap_labels.count(right_pos.label))
        return false;

      if (begin_position <= 0)
        return false;

      int ending_position = umap_labels[right_pos.label].first;
      if (right_pos.direction)
        ending_position = umap_labels[right_pos.label].second;

      if (begin_position > source.length())
        return false;
      if (ending_position > source.length() + 1)
        return false;
      if (ending_position - begin_position <= 0)
        return false;

      string segment = source.substr(begin_position - 1, ending_position - begin_position);
      if (!TestSegment(segment, matching_pos, target))
        return false;
    }
  }
  return false;
}

void Graph::GenGraphStr(const string &val, int id)
{
  // initialize the vertexes
  int len = val.length();
  for (int l = 0; l < len + 3; l++)
    this->nodes.emplace_back(l, id, l); // vertex_id, row_id, and position

  unordered_map<pair<int, int>, vector<ELabel>, pair_hash> match_map;
  vector<int> mid(ALL_REGEX_SIZE, 0);
  for (auto i = 0; i < REGEX_SIZE; i++)
    GetMatchId(val, mid[i], static_cast<Regex>(i), regexes[i], match_map);

  // building the edges
  this->edges.emplace_back(0, 1, Regex::START_T, 1);
  this->edges.emplace_back(len + 1, len + 2, Regex::END_T, 1);
  mid[Regex::START_T] = mid[Regex::END_T] = 1;

  unordered_map<string, int> constants;
  for (int n = 1; n < len + 1; n++)
    for (int m = n + 1; m < len + 2; m++)
    {
      Edge edge(n, m);
      string seg = val.substr(n - 1, m - n);
      constants[seg] += 1;
      edge.labels.emplace_back(Regex::CONSTANT, constants[seg], seg);
      auto mit = match_map.find(make_pair(n - 1, m - 1));
      if (mit != match_map.end())
        edge.labels.insert(edge.labels.end(), mit->second.begin(), mit->second.end());
      this->edges.push_back(edge);
    }

  for (auto &e : this->edges)
  {
    int label_num = e.labels.size();
    for (auto l = 0; l < label_num; l++)
    {
      Regex r = e.labels[l].regex;
      int seq = e.labels[l].index;
      if (r == Regex::CONSTANT)
        e.labels.emplace_back(r, seq - 1 - constants[e.labels[l].constr], e.labels[l].constr);
      else
        e.labels.emplace_back(r, seq - 1 - mid[r]);
    }
  }
}

void Graph::GenGraphStr(const string &val, int id, const unordered_map<string, pair<double, int>> &valid_terms)
{
  // initialize the vertexes
  int len = val.length();
  for (int l = 0; l < len + 3; l++)
    this->nodes.emplace_back(l, id, l); // vertex_id, row_id, and position

  unordered_map<pair<int, int>, vector<ELabel>, pair_hash> match_map;
  vector<int> mid(ALL_REGEX_SIZE, 0);
  for (auto i = 0; i < REGEX_SIZE; i++)
    GetMatchId(val, mid[i], static_cast<Regex>(i), regexes[i], match_map);

  this->edges.emplace_back(0, 1, Regex::START_T, 1);
  this->edges.emplace_back(len + 1, len + 2, Regex::END_T, 1);
  mid[Regex::START_T] = mid[Regex::END_T] = 1;

  unordered_map<string, int> constants;
  all_pos.clear();
  vector<pair<double, int>> scores(len + 3, make_pair(.0, 0));
  for (int n = 1; n < len + 1; n++)
    for (int m = n + 1; m < len + 2; m++)
    {
      // build the edges
      auto mit = match_map.find(make_pair(n - 1, m - 1));
      if (mit != match_map.end())
      {
        this->edges.emplace_back(n, m);
        this->edges.back().labels.insert(this->edges.back().labels.end(), mit->second.begin(), mit->second.end());
      }

      // build the constant positions
      string seg = val.substr(n - 1, m - n);
      constants[seg] += 1;
      auto it = valid_terms.find(seg);
      if (it != valid_terms.end())
      {
        // update the left hand side
        if (it->second.first > scores[n].first + EPS)  // strictly larger
        {
          scores[n].first = it->second.first;
          scores[n].second = m - n;
          all_pos[n] = Pos(Regex::CONSTANT, constants[seg], seg, false);
        }
        else if (it->second.first + EPS > scores[n].first) // larger or equal ---> equal 
        {
          if (m - n > scores[n].second) // current is longer
          {
            scores[n].first = it->second.first;
            scores[n].second = m - n;
            all_pos[n] = Pos(Regex::CONSTANT, constants[seg], seg, false);
          }
        }

        // update the right hand side
        if (it->second.first > scores[m].first + EPS) // strictly larger
        {
          scores[m].first = it->second.first;
          scores[m].second = m - n;
          all_pos[m] = Pos(Regex::CONSTANT, constants[seg], seg, true);   
        }
        else if (it->second.first + EPS > scores[m].first) // larger or equal ---> equal
        {
          if (m - n > scores[m].second)
          {
            scores[m].first = it->second.first;
            scores[m].second = m - n;
            all_pos[m] = Pos(Regex::CONSTANT, constants[seg], seg, true);
          }
        }
      }
    }

#ifdef STATIC_ORDERING_ENABLE
  // build the regex-based positions
  for (auto &e : this->edges)
    for (auto &l : e.labels)
    {
      Pos lhs(l, false);
      if (lhs > all_pos[e.src_id])
        all_pos[e.src_id] = lhs;

      Pos rhs(l, true);
      if (rhs > all_pos[e.end_id])
        all_pos[e.end_id] = rhs;
    }
#endif

  // build the reverse positions
  for (auto &p : all_pos)
  {
    Regex r = p.second.label.regex;
    int seq = p.second.label.index;
    if (r == Regex::CONSTANT)
    {
      int rev = seq - 1 - constants[p.second.label.constr];
      if (abs(rev) < seq)
        p.second.label.index = rev;
    }
    else
    {
      int rev = seq - 1 - mid[r];
      if (abs(rev) < seq)
        p.second.label.index = rev;
    }
  }

  // build the reverse edges
  for (auto &e : this->edges)
  {
    int label_num = e.labels.size();
    for (auto l = 0; l < label_num; l++)
    {
      Regex r = e.labels[l].regex;
      int seq = e.labels[l].index;
      if (r == Regex::CONSTANT)
        e.labels.emplace_back(r, seq - 1 - constants[e.labels[l].constr], e.labels[l].constr);
      else
        e.labels.emplace_back(r, seq - 1 - mid[r]);
    }
  }
}

void DAGraph::GenerateDag(unordered_map<string, int> &frequency)
{
  int len = output.length();
  for (int l = 0; l < len + 3; l++)
    this->nodes.emplace_back(l, rule_id, l);

  vector<vector<Pos>> posList;
  input_graph.getPosList(posList);

  // edges are sorted first in src_id order and the in end_id order
  this->edges.emplace_back(0, 1, start_special_token);
  for (int n = 1; n < len + 1; n++)
    for (int m = n + 1; m < len + 2; m++)
    {
      bool hasLabel = false;
      DagEdge edge(n, m);
      string seg = output.substr(n - 1, m - n);
      if (frequency[seg] > GLOBAL_FREQUENCY_THRESHOLD)
      {
        hasLabel = true;
        edge.constant = seg;
      }

      size_t pos = this->input.find(seg, 0);
      while(pos != string::npos)
      {
        hasLabel = true;
        edge.const_pos.emplace_back(pos + 1, pos + 1 + m - n);
        if (!posList[pos + 1].empty() && !posList[pos + 1 + m - n].empty())
          edge.input_pos.emplace_back(posList[pos + 1], posList[pos + 1 + m - n]);
        pos = this->input.find(seg, pos + 1);
      }

      if (hasLabel)
        this->edges.push_back(edge);
    }
  this->edges.emplace_back(len + 1, len + 2, end_special_token);
  this->starting_node_id = 1;
  this->ending_node_id = len + 2;
}

void DAGraph::GenerateDagEnhance(const unordered_map<string, pair<double, int>> &valid_terms)
{
  int len = output.length();
  for (int l = 0; l < len + 3; l++)
    this->nodes.emplace_back(l, rule_id, l);

  vector<vector<Pos>> posList;
  input_graph.getPosList(posList);

  // score and starting point
  vector<pair<double, int>> constant_edges(len + 3, make_pair(.0, -1));
  for (int n = 1; n < len + 1; n++)
  {
    double max_ratio = 0.0;
    int boundary = -1;
    for (int m = n + 1; m < len + 2; m++)
    {
      auto it = valid_terms.find(output.substr(n - 1, m - n));
      if (it != valid_terms.end())
      {
        if (it->second.first + EPS > max_ratio) // larger or equal
        {
          max_ratio = it->second.first;
          boundary = m;
        }
      }
    }
    if (boundary == -1)
      continue;

    if (max_ratio > constant_edges[boundary].first + EPS) // strictly larger
    {
      constant_edges[boundary].first = max_ratio;
      constant_edges[boundary].second = n;
    }
  }

  // edges are sorted first in src_id order and the in end_id order
  this->edges.emplace_back(0, 1, start_special_token);
  for (int n = 1; n < len + 1; n++)
    for (int m = n + 1; m < len + 2; m++)
    {
      bool hasLabel = false;
      DagEdge edge(n, m);
      string seg = output.substr(n - 1, m - n);

      if (constant_edges[m].second == n)
      {
        hasLabel = true;
        edge.constant = seg;
      }

      size_t pos = this->input.find(seg, 0);
      while(pos != string::npos)
      {
        hasLabel = true;

        if (!posList[pos + 1].empty() && !posList[pos + 1 + m - n].empty())
          edge.input_pos.emplace_back(posList[pos + 1], posList[pos + 1 + m - n]);

#ifdef PREFIX_SUFFIX_TERM_ENABLE
        bool is_right_longest = false;

        if (m >= len + 1 || pos + m - n >= input.length())
           is_right_longest = true;
        else if (output[m - 1] != input[pos + m - n])
           is_right_longest = true;
        else if (!isalnum(output[m - 1]))
           is_right_longest = true;

        bool is_left_longest = false;

        if (n <= 1 || pos <= 0)
          is_left_longest = true;
        else if (output[n - 2] != input[pos - 1])
          is_left_longest = true;
        else if (!isalnum(output[n - 2]))
          is_left_longest = true;

        // there is a bug here; we need to fix it later after the submission
        if (is_right_longest && is_left_longest)
        {
          ELabel l_elabel;
          for (auto graph_edge_id : input_graph.node2edge[pos + 1])
          {
            auto &input_graph_edge = input_graph.edges[graph_edge_id];
            if (input_graph_edge.end_id > pos + 1 + m - n)
            {
              for (auto &l : input_graph_edge.labels)
                if (l > l_elabel)
                  l_elabel = l;
            }
          }
          if (l_elabel.regex != Regex::SPECIAL_CASE_FOR_DAG_EDGE && l_elabel.regex != Regex::CONSTANT)
          {
            l_elabel.constr = "PREFIX";
            edge.prefixes.push_back(l_elabel);
          }

          ELabel r_elabel;
          for (auto graph_edge_id : input_graph.rev_node2edge[pos + 1 + m - n])
          {
            auto &input_graph_edge = input_graph.edges[graph_edge_id];
            if (input_graph_edge.src_id < pos + 1)
            {
              for (auto &l : input_graph_edge.labels)
                if (l > r_elabel)
                  r_elabel = l;
            }
          }
          if (r_elabel.regex != Regex::SPECIAL_CASE_FOR_DAG_EDGE && r_elabel.regex != Regex::CONSTANT)
          {
            r_elabel.constr = "SUFFIX";
            edge.suffixes.push_back(r_elabel);
          }

          edge.const_pos.emplace_back(pos + 1, pos + 1 + m - n);
        }
#else
        edge.const_pos.emplace_back(pos + 1, pos + 1 + m - n);
#endif
        pos = this->input.find(seg, pos + 1);
      }

      if (hasLabel)
        this->edges.push_back(edge);
    }

  this->edges.emplace_back(len + 1, len + 2, end_special_token);
  this->starting_node_id = 1;
  this->ending_node_id = len + 2;
}

void DAGraph::getAdjacentMatrix()
{
  int num_dag_vertex = nodes.size();
  dag_edge_adj.resize(num_dag_vertex);
  for (auto i = 0; i < edges.size(); i++)
    dag_edge_adj[edges[i].src_id].push_back(i);
}

uint64_t DAGraph::getEdgeSize(int eid)
{
  if (edgeSize[eid] == 0)
  {
    uint64_t nsize = 0;
    if (!edges[eid].constant.empty())
      nsize += 1;

    nsize += edges[eid].const_pos.size();
    for (auto &entry : edges[eid].input_pos)
    {
      nsize += entry.first.size() * entry.second.size();
    }
    edgeSize[eid] = nsize;
  }
  return edgeSize[eid];
}

uint64_t DAGraph::getGraphSize()
{
  edgeSize.assign(edges.size(), 0);
  nodeSize.assign(nodes.size(), 0);
  nodeSize[0] = 1;

  for (auto vid = 0; vid < nodes.size(); vid++)
    for (auto eid : dag_edge_adj[vid])
      nodeSize[edges[eid].end_id] += getEdgeSize(eid) * nodeSize[vid];

  graphSize = nodeSize[this->ending_node_id];
  if (graphSize == 0)
    graphSize = std::numeric_limits<uint64_t>::max();

  return graphSize;
}

// this function is dependent on InvIndex and GenerateDag functions. They need to guarantee the Elem order in invList
bool DAGraph::PickBest(const vector<Elem> &curList, const vector<Elem> &invList, vector<Elem> &resList, int local_threshold) {
  // curList is guaranteed to have a larger size than local_threshold

  resList.clear();
  if (invList.size() < local_threshold)
    return false;

  // Hard code here, if curList is empty, then it is the first list in the very beginning
  if (curList.empty()) {
    auto it = invList.begin();
    while (it != invList.end()) {
      if (it->beg == this->starting_node_id)
        resList.emplace_back(it->pid, it->beg, it->end);
      ++it;
    }
  } else {
    // this function is dependent on InvIndex and GenerateDag functions. They need to guarantee the Elem order in invList
    auto it1 = curList.begin();
    auto it2 = invList.begin();
    while (it1 != curList.end() && it2 != invList.end()) {
      if (it1->pid < it2->pid)
        ++it1;
      else if (it1->pid > it2->pid)
        ++it2;
      else if (it1->end != it2->beg)  // there is only 1 entry for each pid in curList
        ++it2;
      else {
        resList.emplace_back(it1->pid, it1->beg, it2->end);
        ++it1;
        ++it2;
      }
    }
  }

  if (resList.size() < local_threshold)
    return false;
  else
    return true;

}

// node_id: start from this node
// dag_edge_adj: out-going edges of a node
// end: the ending node id
// list: the valid element list
// synsizer: context
// threshold: the maximum number of applicable rules have seen now
// best_trans: the best path has seen now
// current_trans: the path checking now

int s_cnt = 0;

void DAGraph::DeepFirstSearch(const int src_id, const int end_id,
                              vector<Elem> &curList, Path &curPath,
                              vector<Elem> &maxList, Path &maxPath,
                              Synthesizer &synsizer, int &local_threshold)
{
  if (src_id == end_id)
  {
    if ((int)curList.size() > (int)maxList.size() || ((int)curList.size() == (int)maxList.size() && curPath > maxPath))
    {
      maxPath = curPath;
      maxList = curList;
#ifdef LOCAL_THRESHOLD_ENABLE
      local_threshold = maxList.size();
#endif
    }

#ifdef GLOBAL_THRESHOLD_ENABLE
    for (auto &elem : curList)
    {
      if (synsizer.thresholds[elem.pid] < curList.size())
        synsizer.thresholds[elem.pid] = curList.size();
    }
#endif
    return;
  }

  for (auto next_eid : dag_edge_adj[src_id])
  {
    auto &e = edges[next_eid];
#ifdef OUTPUT_INFO
    assert(src_id == e.src_id);
#endif
    if (curPath.path.size() == max_path_length - 1 && e.end_id != end_id)
      continue;

    if (!e.constant.empty())
    {
      auto it = synsizer.constr_index.find(e.constant);
      if (it != synsizer.constr_index.end())
      {
        vector<Elem> tmpList;
        auto &invList = it->second;
        if (PickBest(curList, invList, tmpList, local_threshold))
        {
          curPath.path.emplace_back(Pos(Regex::SPECIAL_CASE_FOR_DAG_EDGE, -1, e.constant, false), Pos(Regex::SPECIAL_CASE_FOR_DAG_EDGE, -1, e.constant, true));
          DeepFirstSearch(edges[next_eid].end_id, end_id, tmpList, curPath, maxList, maxPath, synsizer, local_threshold);
          curPath.path.pop_back();
        }
      }
    }

    for (auto &entry : e.const_pos)
    {
      auto it = synsizer.cpos_index.find(entry);
      if (it != synsizer.cpos_index.end())
      {
        vector<Elem> tmpList;
        auto &invList = it->second;
        if (PickBest(curList, invList, tmpList, local_threshold))
        {
          curPath.path.emplace_back(Pos(Regex::SPECIAL_CASE_FOR_DAG_EDGE, entry.first, false), Pos(Regex::SPECIAL_CASE_FOR_DAG_EDGE, entry.second, true));
          DeepFirstSearch(edges[next_eid].end_id, end_id, tmpList, curPath, maxList, maxPath, synsizer, local_threshold);
          curPath.path.pop_back();
        }
      }
    }

    for (auto &entry : e.prefixes)
    {
      auto it = synsizer.prefix_index.find(entry);
      if (it != synsizer.prefix_index.end())
      {
        vector<Elem> tmpList;
        auto &invList = it->second;
        if (PickBest(curList, invList, tmpList, local_threshold))
        {
          curPath.path.emplace_back(Pos(entry, false), Pos(entry, true));
          DeepFirstSearch(edges[next_eid].end_id, end_id, tmpList, curPath, maxList, maxPath, synsizer, local_threshold);
          curPath.path.pop_back();
        }
      }
    }

    for (auto &entry : e.suffixes)
    {
      auto it = synsizer.suffix_index.find(entry);
      if (it != synsizer.suffix_index.end())
      {
        vector<Elem> tmpList;
        auto &invList = it->second;
        if (PickBest(curList, invList, tmpList, local_threshold))
        {
          curPath.path.emplace_back(Pos(entry, false), Pos(entry, true));
          DeepFirstSearch(edges[next_eid].end_id, end_id, tmpList, curPath, maxList, maxPath, synsizer, local_threshold);
          curPath.path.pop_back();
        }
      }
    }

    for (auto &entry : e.input_pos)
      for (auto &l : entry.first)
        for (auto &r : entry.second) 
        {
          auto it = synsizer.substr_index.find(make_pair(l, r));
          if (it != synsizer.substr_index.end()) 
          {
            vector<Elem> tmpList;
            auto &invList = it->second;
            if (PickBest(curList, invList, tmpList, local_threshold))
            {
              curPath.path.emplace_back(l, r);
              DeepFirstSearch(edges[next_eid].end_id, end_id, tmpList, curPath, maxList, maxPath, synsizer, local_threshold);
              curPath.path.pop_back();
            }
          }
        }
  }
}


void DAGraph::DynamicRanking(Path &maxPath, Synthesizer &synsizer)
{
  Path curPath;
  int threshold = synsizer.thresholds[rule_id];
  vector<Elem> curList;
  vector<Elem> maxList;

#ifdef OUTPUT_INFO
  timeval st, en;
  gettimeofday(&st, NULL);
  cout << " ------- start dynamic ranking --------- " << endl;
  cout << "InvIndex Sizes: " << synsizer.cpos_index.size() << " " << synsizer.constr_index.size() << " " << synsizer.substr_index.size() << endl;
  cout << "Rule Id: " << rule_id << endl;
  cout << input << " ---> " << output << endl;
  cout << "Graph Size: " << getGraphSize() << " " << edges.size() << endl;
  cout << "Global Threshold Before Searching: " << threshold << endl;
#endif

  DeepFirstSearch(starting_node_id, ending_node_id, curList, curPath, maxList, maxPath, synsizer, threshold);

#ifdef OUTPUT_INFO
  cout << "Local Threshold After Searching: " << threshold << endl << endl;
  cout << "# of applicable rules: " << threshold << endl;
  cout << " transformation size: " << maxPath.path.size() << " the transformation: " << endl;
  for (auto &e : maxPath.path)
    cout << e.first << " --- " << e.second << endl;
  gettimeofday(&en, NULL);
  cout << "# ranking time (for one rule) :" << en.tv_sec - st.tv_sec + (en.tv_usec - st.tv_usec) * 1.0 / CLOCKS_PER_SEC << endl << endl;
  cout << endl << endl;
#endif
}

// double input_time = 0;
// double dag_time = 0;
// double index_time = 0;

// timeval st, en;
// gettimeofday(&st, NULL);
// gettimeofday(&en, NULL);
// double input_time += en.tv_sec - st.tv_sec + (en.tv_usec - st.tv_usec) * 1.0 / CLOCKS_PER_SEC << endl << endl;
// logTime(st, en, " building the graph.");
// logTime(mid2, en, " building the indexes.");


void Synthesizer::InvIndex(const vector<DAGraph> &dags)
{
#ifdef OUTPUT_INFO
  timeval t1, t2;
  gettimeofday(&t1, NULL);
#endif

  for (auto &d : dags)
    for (auto &e : d.edges)
    {
      if (!e.constant.empty())
        constr_index[e.constant].emplace_back(d.rule_id, e.src_id, e.end_id);

      for (auto &l : e.const_pos)
        cpos_index[l].emplace_back(d.rule_id, e.src_id, e.end_id);

#ifdef PREFIX_SUFFIX_TERM_ENABLE
      for (auto &l : e.prefixes)
        prefix_index[l].emplace_back(d.rule_id, e.src_id, e.end_id);

      for (auto &l : e.suffixes)
        suffix_index[l].emplace_back(d.rule_id, e.src_id, e.end_id);
#endif

      for (auto &l : e.input_pos)
        for (auto &p1 : l.first)
          for (auto &p2 : l.second)
            substr_index[make_pair(p1, p2)].emplace_back(d.rule_id, e.src_id, e.end_id);
    }

#ifdef OUTPUT_INFO
  cout << "constr_index: " << constr_index.size()
       << "cpos_index: " << cpos_index.size()
       << "substr_index: " << substr_index.size()
       << "prefix_index: " << prefix_index.size()
       << "suffix_index: " << suffix_index.size() << endl;

  logTime(t1, t2, "Finished Indexing String Functions: ");
#endif
}

void Synthesizer::Indexing(const unordered_map<string, pair<double, int>> &valid_terms)
{
  // limiting constant str and constant string terms
  /*
  for (auto &entry : rules)
  {
    const string &lhs = entry.first.first;
    const string &rhs = entry.first.second;

    unordered_set<string> distinct_substrings;
    for (auto i = 0; i < lhs.length(); i++)
      for (auto j = i; j < lhs.length(); j++)
        distinct_substrings.insert(lhs.substr(i, j - i + 1));

    for (auto i = 0; i < rhs.length(); i++)
      for (auto j = i; j < rhs.length(); j++)
        distinct_substrings.insert(rhs.substr(i, j - i + 1));

    for (auto &str : distinct_substrings)
      frequency[str] += 1;
  }
  */

#ifdef UNIQUE_THRESHOLD_ENABLE
  thresholds.assign(rules.size(), 2);
#else
  thresholds.assign(rules.size(), 0);
#endif

  int id = 0;
  for (auto &entry : rules)
  {
    const string &lhs = entry.first.first;
    const string &rhs = entry.first.second;
    // cout << "materializing graph: " << id << endl;
    // cout << lhs << " --- " << rhs << endl;
    dags.emplace_back(lhs, rhs, id++, valid_terms);
  }
  InvIndex(dags);

  // cout << "finished building" << endl;
  // PosIndex(dags);
  // cout << "finished indexing" << endl;
  // SurveyGraph();
  // cout << "finished surveying" << endl;
  // cout << "finished initializing" << endl;
}


/*
void Synthesizer::PosIndex(const vector<DAGraph> &dags)
{
  unordered_map<Pos, vector<Elem>, std::hash<Pos>> l_rpos_index;
  unordered_map<Pos, vector<Elem>, std::hash<Pos>> r_rpos_index;
  for (auto &d : dags)
    for (auto &e : d.edges)
      for (auto &entry : e.input_pos)
      {
        for (auto &l : entry.first)
          l_rpos_index[l].emplace_back(d.rule_id, e.src_id, e.end_id);

        for (auto &r : entry.second)
          r_rpos_index[r].emplace_back(d.rule_id, e.src_id, e.end_id);
      }
}

void Synthesizer::SurveyGraph()
{
  for (auto &d : dags)
  {
    cout << "surveying dag " << d.rule_id << endl;
    for (auto &e : d.edges)
    {
      cout << "constant string: " << e.constant << endl;

      for (auto &entry : e.const_pos)
        cout << "constant positions: " << entry.first << " " << entry.second << endl;

      for (auto &entry : e.input_pos)
        for (auto &l : entry.first)
          cout << "left positions: " << l << "  size: " << l_rpos_index[l].size() << endl;

      for (auto &entry : e.input_pos)
        for (auto &r : entry.second)
          cout << "right positions: " << r << " size: " << r_rpos_index[r].size() << endl;

      cout << endl;
    }
  }
}

void Synthesizer::Statistics()
{
  cout << "constant string count: " << endl;

  map<int, int> cnt_freq;
  for (auto &entry : frequency)
    cnt_freq[entry.second] += 1;
  for (auto &entry : cnt_freq)
    cout << "[" << entry.first << "]: " << entry.second << endl;

  unordered_map<Pos, int, std::hash<Pos>> l_rpos_cnt;
  unordered_map<Pos, int, std::hash<Pos>> r_rpos_cnt;
  unordered_map<pair<int, int>, int, pair_hash> const_pos_cnt;

  for (auto &d : dags) {
    unordered_set<pair<int, int>, pair_hash> const_pos_set;
    unordered_set<Pos, std::hash<Pos>> l_rpos_set;
    unordered_set<Pos, std::hash<Pos>> r_rpos_set;
    for (auto &e : d.edges) {
      for (auto &entry : e.const_pos)
        const_pos_set.insert(entry);

      for (auto &entry : e.input_pos) {
        for (auto &l : entry.first)
          l_rpos_set.insert(l);

        for (auto &r : entry.second)
          r_rpos_set.insert(r);
      }
    }
    for (auto &entry : const_pos_set)
      const_pos_cnt[entry] += 1;
    for (auto &l : l_rpos_set)
      l_rpos_cnt[l] += 1;
    for (auto &r : r_rpos_set)
      r_rpos_cnt[r] += 1;
  }

  cout << endl << "constant position count: " << endl;
  cnt_freq.clear();
  for (auto &entry : const_pos_cnt)
    cnt_freq[entry.second] += 1;
  for (auto &entry : cnt_freq)
    cout << "[" << entry.first << "]: " << entry.second << endl;

  cout << endl << "left regex position count: " << endl;
  cnt_freq.clear();
  for (auto &entry : l_rpos_cnt)
    cnt_freq[entry.second] += 1;
  for (auto &entry : cnt_freq)
    cout << "[" << entry.first << "]: " << entry.second << endl;

  cout << endl << "right regex position count: " << endl;
  cnt_freq.clear();
  for (auto &entry : r_rpos_cnt)
    cnt_freq[entry.second] += 1;
  for (auto &entry : cnt_freq)
    cout << "[" << entry.first << "]: " << entry.second << endl;
}
*/

void Synthesizer::SynAggregating(unordered_map<Path, vector<int>, PathHash> &groups, const vector<int> &rule_id_maps)
{
  // Statistics();
#ifdef OUTPUT_INFO
  timeval t1, t2;
  gettimeofday(&t1, NULL);
#endif

  Path uniqu_path;
  uniqu_path.path.emplace_back(Pos(Regex::CAPS, 100, "UNIQUE_PATH", false), Pos(Regex::CAPS, 101, "UNIQUE_PATH", false));

  for (auto &d : dags)
  {
    Path p;
    d.DynamicRanking(p, *this);

    if (p.path.empty())
    {
      uniqu_path.path.front().first.label.index += 1;
      groups[uniqu_path].emplace_back(rule_id_maps[d.rule_id]);
    }
    else
    {
      groups[p].emplace_back(rule_id_maps[d.rule_id]);
    }
  }

#ifdef OUTPUT_INFO
  logTime(t1, t2, "Aggregating Time.");
#endif
}

