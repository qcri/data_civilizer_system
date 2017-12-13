
#include "Reader.h"

void Reader::strNormalize(string &s) {
  if (s.size() > 10000) s.resize(10000);
  string str = "";
  str.reserve(s.size());
  char prev_char = ' ';
  for (auto i = 0; i < s.size(); i++) {
    if (prev_char == ' ' && s[i] == ' ')
      continue;
    prev_char = s[i];
    str.push_back(tolower(s[i]));
    // if (isalnum(s[i]) || s[i] == '_')  str.push_back(tolower(s[i]));
    // else if (!str.empty() && str.back() != ' ') str.push_back(' ');
  }
  if (!str.empty() && str.back() == ' ') str.pop_back();
  s = str;
}
