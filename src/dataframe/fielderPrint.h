#include "helper.h"
/*****************************************************************************
 * Fielder::
 * A field vistor invoked by Row.
 */
class FielderPrint : public Fielder {
public:
  size_t rowIdx;

  FielderPrint() {
    rowIdx = 0;
  }

  FielderPrint(size_t idx) {
    rowIdx = idx;
  }

  /** Called before visiting a row, the argument is the row offset in the
    dataframe. */
  virtual void start(size_t r) {
    rowIdx = r;
    p("row index:"); p(r); p("   ");
  }

  /** Called for fields of the argument's type with the value of the field. */
  virtual void accept(bool b) {
    p(b); p("   ");
  }
  virtual void accept(float f) {
    p(f); p("   ");
  }
  virtual void accept(int i) {
    p(i); p("   ");
  }
  virtual void accept(String* s) {
    p(s->c_str()); p("   ");
  }

  /** Called when all fields have been seen. */
  virtual void done() {
    pln();
  }
};
