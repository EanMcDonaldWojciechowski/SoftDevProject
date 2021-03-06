#pragma once
/*******************************************************************************
 *  Rower::
 *  An interface for iterating through each row of a data frame. The intent
 *  is that this class should subclassed and the accept() method be given
 *  a meaningful implementation. Rowers can be cloned for parallel execution.
 */
class Rower : public Object {
 public:
  /** This method is called once per row. The row object is on loan and
      should not be retained as it is likely going to be reused in the next
      call. The return value is used in filters to indicate that a row
      should be kept. */
  virtual bool accept(Row& r) {return 0;}
  virtual bool visit(Row& r) {return 0;}

  /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join method
      is reponsible for cleaning up memory. */
  void join_delete(Rower* other) {}

  virtual Rower* clone() {
    return nullptr;
  }
};

class Writer : public Rower {
public:
  char* buf_;
  Writer() {}
  ~Writer() {}
   bool isspace(char ch) {
     if (ch == ' ') {
       return 1;
     } else {
       return 0;
     }
   }
   // virtual void visit(Row & r);
   virtual bool visit(Row & r) {return 0;};
   virtual bool done() {return 0;};
};

class Reader : public Rower {
public:
  Reader() {}
  ~Reader() {}
  // virtual void visit(Row & r);
   virtual bool visit(Row & r) {return 0;};
   virtual bool done() {return 0;};
};
