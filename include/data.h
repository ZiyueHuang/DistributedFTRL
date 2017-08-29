#ifndef DATA_H
#define DATA_H

#include <string>
#include <vector>
#include <iostream>

template<typename DType>
class DataIter {
 public:
  virtual ~DataIter(void) {}
  
  virtual void Reset(void) = 0;

  virtual bool Next(void) = 0;

  virtual const DType &Value(void) const = 0;
};


class Row {
 public:
  real_t *label;

  size_t length;

  real_t *value;

  inline real_t get_value(size_t i)  {
    return value[i];
  }

  inline real_t get_label()  {
    return *label;
  }

  inline real_t SDot(const std::vector<real_t>& weight)  {
    assert(weight.size() == length);
    real_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
      sum += weight[i] * value[i];
    }    
    return sum;
  }
};

#endif
