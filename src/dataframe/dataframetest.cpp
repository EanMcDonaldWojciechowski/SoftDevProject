#include "modified_dataframe.h"


int main(int argh , char *argv[]) {
  Schema* s = new Schema();
  s->add_column('I');
  s->add_column('B');
  s->add_column('F');
  s->add_column('S');
  DataFrame* df = new DataFrame(*s);

  return 0;
}
