//lang::CwC
#include "../src/application/sorer.h"

int main(int argc, char** argv) {
  // char* file_name = "../../data/sampleData.sor";
  char* file_name = "sampleData.sor";
  FILE *f = fopen("./sampleData.sor", "r");
  if (f == NULL) {
    std::cout << "ERROR opening file." << "\n";
  } else {
    std::cout << "IT WORRRKEDDDDDDDDDDDDDDDDD" << "\n";
  }
  SOR* reader = new SOR();
  std::cout << "created sor" << "\n";
  reader->read(f, 0, 10000);
  std::cout << "reading file" << "\n";
  DataFrame *df = reader->sorToDataframe();
  std::cout << "past create df" << "\n";
  std::cout << "cols size " << df->ncols();
}
