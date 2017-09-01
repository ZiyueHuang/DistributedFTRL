#include "data.h"
#include "strtonum.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <cassert>
#include <vector>


class CSVIter : public DataIter<RowBlock> {
 public:
  CSVIter(RowBlock *block, std::string& filename, int label_col, size_t batch_size) : 
    block_(block), batch_size_(batch_size), label_col_(label_col) {
    input_.open(filename.c_str(), std::ios::in);
  }

  void Reset(void) {
    input_.seekg(0, std::ios::beg);
  }

  bool Next(void) {
    size_t size = 0;
    std::string line;
    std::string strBlock;
    while (getline(input_, line)) {
      strBlock += line + "\n";
      size++;
      if (size == batch_size_) break;
    }
    if (size == 0) return false;
    std::cout << strBlock << std::endl;
    ParseBlock((char *)strBlock.c_str(), (char *)strBlock.c_str() + strBlock.length()-1);
    return true;
  }

  RowBlock &Value(void) {
    return *block_;
  }

  void ParseBlock(char *begin, char *end) {
    char *lbegin = begin;
    char *lend = lbegin;
    size_t size = 0;
    real_t *value = block_->value;
    real_t *label = block_->label;
  
    while (lbegin != end) {
      // get line end
      lend = lbegin + 1;
      while (lend != end && *lend != '\n' && *lend != '\r') ++lend;
      
      size++;
  
      char* p = lbegin;
      int column_index = 0;
      size_t idx = 0;
  
      while (p != lend) {
        char *endptr;
        float v = strtof(p, &endptr);
        p = endptr;
  
        if (column_index == label_col_){
          *label = v;
          label++;
          //std::cout << v << " label" << std::endl;
        } else {
          *value = v;
          value++;
          //std::cout << v << " value" << std::endl;
        }
  
        ++column_index;
        while (*p != ',' && p != lend) ++p;
        if (p != lend) ++p;
      }
      // skip empty line
      while ((*lend == '\n' || *lend == '\r') && lend != end) ++lend;
      lbegin = lend;
    }
  
    block_->size = size;
  }


  RowBlock *block_;
  int label_col_;
  std::ifstream input_;
  size_t batch_size_;
};
