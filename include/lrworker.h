#ifndef LRWorker_H_
#define LRWorker_H_


#include "base.h"

#include <vector>


class LRWorker {
public:
  LRWorker(size_t num_dim, real_t learning_rate=0.001): num_dim_(num_dim), learning_rate_(learning_rate) {
    kv = new KVWorker<float>(0);
  }

  ~LRWorker() {
    if (kv) {
      delete kv;
    }
  }

  void Train(RowBlock& batch);

  std::vector<int> Test(RowBlock& batch);

  std::vector<real_t> GetWeight();

  void InitWeight();

  int Predict(std::vector<real_t>& features);

  void PullWeight();

  void PushGradient(std::vector<real_t>& grads);

  size_t num_dim_;
  real_t learning_rate_;

  std::vector<real_t> weight_;

  ps::KVWorker<real_t>* kv;
};


#endif  // LRWorker_H_
