
#ifndef _READER_H_
#define _READER_H_

#include "Table.h"
#include <string.h>
#include <stdio.h>

using namespace std;

class Reader {
public:
  vector<Table> tables;

  // shorten s to the first 10000 character
  // remove all the non-alphanum and '.' characters
  // collaps multiple whitespace and trim both sides
  void strNormalize(string &s); // also for the use of query normalization

  // read in tables from datafilespath
  virtual bool reading(string &datafilespath, bool normalize) = 0;
  virtual int get_max_val_len() = 0;
};

#endif
