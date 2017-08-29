#include "ps/ps.h"
#include "KVDistServer.h"

using namespace ps;

void StartServer() {
  if (!IsServer()) {
    return;
  }
  auto server = new KVDistServer();
  RegisterExitCallback([server](){ delete server; });
}

void RunWorker() {
  if (!IsWorker()) return;
  KVWorker<float> kv(0);

  std::vector<Key> keys;
  std::vector<float> vals;
  keys.resize(1, 0);
  vals.resize(100, 0);

  if (MyRank() == 0) {
    kv.Wait(kv.Push(keys, vals));
  }

  if (!Postoffice::Get()->is_recovery()) {
    Postoffice::Get()->Barrier(kWorkerGroup);
  }

  for (size_t i=0; i<100; ++i){
    vals[i] = 1;
  }
  kv.Wait(kv.Push(keys, vals));

  std::vector<float> rets;
  kv.Wait(kv.Pull(keys, &rets));

  float res = 0;
  for (int i = 0; i < 100; ++i) {
    res += fabs(rets[i] - 2*vals[i]);
  }
  LL << "error: " << res;
}

int main() {
  StartServer();

  Start();
  RunWorker();

  Finalize();
  return 0;
}
