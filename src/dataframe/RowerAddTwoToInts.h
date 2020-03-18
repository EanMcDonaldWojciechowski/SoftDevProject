#pragma once
/*******************************************************************************
 *  Rower::
 *  An interface for iterating through each row of a data frame. The intent
 *  is that this class should subclassed and the accept() method be given
 *  a meaningful implementation. Rowers can be cloned for parallel execution.
 */
class RowerAddTwo : public Rower {
 public:
   RowerAddTwo() {

   }
   ~RowerAddTwo() {

   }
  /** This method is called once per row. The row object is on loan and
      should not be retained as it is likely going to be reused in the next
      call. The return value is used in filters to indicate that a row
      should be kept. */
  virtual bool accept(Row& r) {
    for (int i = 0; i < r.schema->width(); i++){
      if (r.col_type(i) == 'I') {
        r.set(i, r.get_int(i) + 2);
      }
    }
    return 1;
  }

  Rower* clone() {
    return new RowerAddTwo();
  }

  /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join method
      is reponsible for cleaning up memory. */
  void join_delete(Rower* other) {
    other->~Rower();
  }
};
