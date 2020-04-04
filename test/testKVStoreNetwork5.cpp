//lang::CwC
// #include "../src/network/KVStore.h"
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {
  size_t nodeIndex = 4;
  // size_t SZ = 10*30;
  // Column *vals = new StringColumn();
  // for (size_t i = 0; i < SZ; i++) {
  //   char nodeIdxChar[256];
  //   snprintf(nodeIdxChar,sizeof(i), "%d", i);
  //   strcat(nodeIdxChar, " TestingString");
  //   String *s = new String(nodeIdxChar);
  //   vals->push_back(s);
  // }

  // Schema *s = new Schema();
  // DataFrame *df = new DataFrame(*s);

  // df->add_column(vals);
  // df->print();

  KVStore *kv = new KVStore(nodeIndex, 5);
  Key *k = new Key("main", 0);
  // sleep(5);
  // kv->put(k, df);
  // DataFrame *newDF = kv->get(k);
  //std::cout << "Printing final DF" << "\n";
  //newDF->print();
  sleep(20);
  std::cout << "COMPLETED" << "\n";
}
