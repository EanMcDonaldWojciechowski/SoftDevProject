#pragma once
/*************************************************************************
 * Key::
 * Stores values by key
 */
class Writer : public Rower {
 public:

   Writer() {

   }

   ~Writer() {

   }

   void visit(Row & r);

   bool done() {return 0;}

};
