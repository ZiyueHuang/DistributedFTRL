#ifndef DATA_H_
#define DATA_H_


#include "base.h"

#include <iostream>
#include <cstdint>
#include <cassert>
#include <vector>


template<typename DType>
class DataIter {
 public:
  virtual ~DataIter(void) {}
  
  virtual void Reset(void) = 0;

  virtual bool Next(void) = 0;

  virtual DType &Value(void) = 0;
};


class Row {
 public:
  real_t *label;

  size_t num_dim; // label is not included

  real_t *value;

  inline real_t get_value(size_t i) {
    assert(i < num_dim);
    return value[i];
  }

  inline real_t get_label() {
    return *label;
  }

  inline real_t SDot(const std::vector<real_t>& weight) {
    assert(weight.size() == num_dim);
    real_t sum = 0;
    for (size_t i = 0; i < num_dim; ++i) {
      sum += weight[i] * value[i];
    }    
    return sum;
  }
};


struct RowBlock {
  size_t size;
  size_t num_dim;

  real_t *label;
  real_t *value;

  inline Row operator[](size_t rowid) {
    assert(rowid < size);
    Row inst;
    inst.label = label + rowid;
    inst.value = value + rowid * num_dim;
    inst.num_dim = num_dim;
 
    return inst;
  }
};


#endif // DATA_H_
