//lang::CwC
#include "../src/network/KVStore.h"

int main(int argc, char** argv) {
  size_t SZ = 10*2;
  Column *vals = new IntColumn();
  for (size_t i = 0; i < SZ; i++) {
    vals->push_back((int)i);
  }

  Schema *s = new Schema();
  DataFrame *df = new DataFrame(*s);

  df->add_column(vals);
  // df->print();

  KVStore *kv = new KVStore(0);
  Key *k = new Key("main", 0);
  kv->put(k, df);
  DataFrame *newDF = kv->get(k);
  std::cout << "Printing final DF" << "\n";
  newDF->print();
  std::cout << "COMPLETED" << "\n";
}
