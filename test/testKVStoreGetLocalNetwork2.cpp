//lang::CwC
// #include "../src/network/KVStore.h"
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {
  size_t nodeIndex = 1;
  // size_t SZ = 10*30;
  // Column *vals = new IntColumn();
  // for (size_t i = 0; i < SZ; i++) {
  //   vals->push_back((int)i);
  // }

  // Schema *s = new Schema();
  // DataFrame *df = new DataFrame(*s);
  //
  // df->add_column(vals);
  // df->print();

  KVStore *kv = new KVStore(nodeIndex);
  Key *k = new Key("main", 0);
  // kv->put(k, df);
  // sleep(4);
  // DataFrame *newDF = kv->get(k);
  DataFrame *newDF = kv->get(k);
  std::cout << "Printing final DF" << "\n";
  newDF->print();
  // sleep(1);
  std::cout << "COMPLETED" << "\n";
}
