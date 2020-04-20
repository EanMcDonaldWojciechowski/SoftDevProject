//lang::CwC
// #include "../src/network/KVStore.h"
// #include "../src/dataframe/modified_dataframe.h"
#include "../src/linus/linus.h"

int main(int argc, char** argv) {
  Args* arg = new Args();
  arg->parse(argc, argv);

  Linus *linus = new Linus(arg);
  linus->run_();
  delete linus;
  delete arg;
  // pthread_exit(0);
  sleep(5);

  std::cout << "COMPLETED" << "\n";
}
