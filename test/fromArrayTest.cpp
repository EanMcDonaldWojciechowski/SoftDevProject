//lang::CwC
// #include "../src/network/KVStore.h"
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {
  size_t SZ = 10*2;
  int *vals = new int[SZ];
  for (size_t i = 0; i < SZ; i++) {
    vals[i] = i;
  }

  KVStore *kv = new KVStore(0);
  Key *k = new Key("main", 0);
  DataFrame *df;
  DataFrame *fromArrayDF = df->fromArray(k, kv, SZ, vals);
  std::cout << "Printing fromArray DF orignal" << "\n";
  fromArrayDF->print();

  DataFrame *newDF = kv->get(k);
  std::cout << "Printing final newDF" << "\n";
  newDF->print();
  std::cout << "COMPLETED" << "\n";
}
