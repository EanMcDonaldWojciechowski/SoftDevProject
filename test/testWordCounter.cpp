//lang::CwC
// #include "../src/network/KVStore.h"
#include "../src/dataframe/modified_dataframe.h"

int main(int argc, char** argv) {

  Args* arg = new Args();
  arg->parse(argc, argv);

  WordCount *wordCounter = new WordCount(arg);

  wordCounter->run_();

  std::cout << "COMPLETED" << "\n";
}
