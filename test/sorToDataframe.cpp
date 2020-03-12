//lang::CwC
#include "../src/application/sorer.h"

int main(int argc, char** argv) {
  char* file_name = "../../data/sampleData.sor";
  FILE *f = fopen(file_name, "r");
  SOR* reader = new SOR();
  reader->read(f, 0, 100000);
  DataFrame *df = reader->sorToDataframe();
  std::cout << "cols size " << df->ncols();
}
