#ifndef KVDistServer_H_
#define KVDistServer_H_


#include "ps/ps.h"
#include "base.h"

using namespace ps;

class KVDistServer {
public:
  KVDistServer(bool sync=true, real_t alpha=0.1 , real_t beta=1,
                    real_t lambda1=0.01, real_t lambda2=0.0005) {
    sync_ = sync;
    alpha_ = alpha;
    beta_ = beta;
    lambda1_ = lambda1;
    lambda2_ = lambda2;

    using namespace std::placeholders;
    ps_server_ = new KVServer<real_t>(0);
    ps_server_->set_request_handle(
      std::bind(&KVDistServer::DataHandle, this, _1, _2, _3));
  }

  ~KVDistServer() {
    if (ps_server_) {
      delete ps_server_;
    }
  }

private:

  struct FTRLEntry {
    real_t g;
    real_t n;
    real_t w;

    FTRLEntry(): g(0), n(0), w(0) {
    }
  };

  struct MergeBuf {
    std::vector<KVMeta> request;
    std::vector<real_t> grads;
  };

  inline void Update(std::vector<FTRLEntry>& entries, std::vector<real_t>& grads) {
    CHECK_EQ(entries.size(), grads.size());
    size_t n = entries.size();
    /*
    for (size_t i = 0; i < n; ++i){
      entries[i].w += grads[i];
    }
    */
    for (size_t i = 0; i < n; ++i){
      entries[i].g += grads[i] - entries[i].w / alpha_ * 
                      (sqrt(entries[i].n + entries[i].g * entries[i].g) - sqrt(entries[i].n));
      entries[i].n += entries[i].g * entries[i].g;

      if (fabs(entries[i].g) < lambda1_){
        entries[i].w = 0;
      } else {
        entries[i].w = ((entries[i].g > 0 ? 1:-1) * lambda1_ - entries[i].g) /
                       ((beta_ + sqrt(entries[i].n)) / alpha_ + lambda2_);
      }
    }
  }

  void DataHandle(const KVMeta& req_meta,
                  const KVPairs<real_t>& req_data,
                  KVServer<real_t>* server) {
    CHECK_EQ(1, req_data.keys.size());
    Key key = req_data.keys[0];
    auto& entries = entries_[key];

    size_t n = 0;
    
    if (req_meta.push) { // push
      n = req_data.vals.size();
      if (entries.empty()) { // init
        entries.resize(n);
        for (size_t i = 0; i < n; ++i) {
          entries[i].w = req_data.vals[i];
        }
        server->Response(req_meta);
      } else if (sync_){ // sync
        CHECK_EQ(entries.size(), req_data.vals.size());
        auto& merged = merge_buf_[key];
        if (merged.grads.empty()) {
          merged.grads.resize(n, 0);
        }
        CHECK_EQ(merged.grads.size(), req_data.vals.size());
        for (size_t i = 0; i < n; ++i) {
          merged.grads[i] += req_data.vals[i];
        }

        merged.request.push_back(req_meta);

        if (merged.request.size() == (size_t)NumWorkers()) {
          int wrkNum = NumWorkers();
          for (size_t i = 0; i < n; ++i) {
            merged.grads[i] /= wrkNum;
          }

          Update(entries, merged.grads);

          for (const auto& req : merged.request) {
            server->Response(req);
          }
          merged.request.clear();
          merged.grads.clear();
        }
      } else { // async
        CHECK_EQ(entries.size(), req_data.vals.size());
        std::vector<real_t> grads;
        grads.resize(n);
        for (size_t i = 0; i < n; ++i) {
          grads[i] = req_data.vals[i];
        }
       
        Update(entries, grads);

        server->Response(req_meta);
      }
    } else { // pull
      CHECK(!entries.empty()) << "Init " << key << " first";      
      n = entries.size();

      KVPairs<real_t> response;
      response.keys = req_data.keys;
      
      response.vals.resize(n);
      for (size_t i = 0; i < n; ++i) {
        response.vals[i] = entries[i].w;
      }
      server->Response(req_meta, response);
    }
  }


  std::unordered_map<Key, std::vector<FTRLEntry>> entries_;
  std::unordered_map<Key, MergeBuf> merge_buf_;

  bool sync_;

  real_t alpha_;
  real_t beta_;
  real_t lambda1_;
  real_t lambda2_;

  KVServer<real_t>* ps_server_;
};


#endif // KVDistServer_H_ 
