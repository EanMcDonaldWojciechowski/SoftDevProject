//lang::CwC
#include "../src/application/sorer.h"

int main(int argc, char** argv) {
  // char* file_name = "../../data/sampleData.sor";
  char* file_name = "../data/sampleData.sor";
  FILE *f = fopen(file_name, "r");
  if (f == NULL) {
    std::cout << "ERROR opening file." << "\n";
  }
  SOR* reader = new SOR();
  reader->read(f, 0, 10000);
  // std::cout << "reading file" << "\n";
  DataFrame *df = reader->sorToDataframe();
  // std::cout << "raw cell data " << df->column[3]->as_string()->get(0)->c_str() << "\n";
  // std::cout << " length " << df->scm->length() << "\n";
  // std::cout << "past create df" << "\n";
  // std::cout << "cols size " << df->ncols();
  std::cout << "\n\n\n_____________Actual Dataframe________________\n" << "\n";
  df->print();
  Rower *r = new RowerAddTwo();
  std::cout << "\n\n\n_____________Adding two to all ints________________\n" << "\n";
  df->map(*r);
  df->print();
  std::cout << "COMPLETED" << "\n";
}
