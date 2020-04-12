//lang::CwC
// #include "../src/network/KVStore.h"
// #include "../src/dataframe/modified_dataframe.h"
#include "../src/linus/linus.h"

int main(int argc, char** argv) {
  Args* arg = new Args();
  arg->parse(argc, argv);

  Linus *linus = new Linus(arg);
  linus->run_();
  sleep(2);

  std::cout << "COMPLETED" << "\n";
}
