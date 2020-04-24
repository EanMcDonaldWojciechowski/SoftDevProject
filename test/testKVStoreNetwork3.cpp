//lang::CwC
// #include "../src/network/KVStore.h"
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {
  size_t nodeIndex = 2;
  size_t SZ = 100*50 + 25;
  Column *vals = new StringColumn();
  for (size_t i = 0; i < SZ; i++) {
    char nodeIdxChar[256];
    snprintf(nodeIdxChar,sizeof(i), "%d", i);
    strcat(nodeIdxChar, " TestingString");
    String *s = new String(nodeIdxChar);
    // std::cout<<"Inserting into col "<<s->c_str()<<"\n";
    vals->push_back(s);
  }

  // Column *vals = new BoolColumn();
  // for (size_t i = 0; i < SZ; i++) {
  //   size_t s = i % 2;
  //   vals->push_back((bool)s);
  // }

  // float currnum = 1.2;
  // Column *vals = new FloatColumn();
  // for (size_t i = 0; i < SZ; i++) {
  //   vals->push_back((float)currnum);
  //   currnum += 1;
  // }

  Schema *s = new Schema();
  DataFrame *df = new DataFrame(*s);

  df->add_column(vals);
  std::cout << "Printing DF" << "\n";
  df->print();

  KVStore *kv = new KVStore(nodeIndex, 3);
  Key *k = new Key("main", 0);
  kv->put(k, df);
  //DataFrame *newDF = kv->get(k);
  //std::cout << "Printing final DF" << "\n";
  //newDF->print();
  sleep(20);
  std::cout << "COMPLETED" << "\n";
}
