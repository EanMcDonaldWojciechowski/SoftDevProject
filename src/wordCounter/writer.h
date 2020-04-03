#pragma once
/*************************************************************************
 * Key::
 * Stores values by key
 */
class Writer : public Object {
 public:
   int v;

   Num() {
     v = 0;
   }

   Num(int v_) {
     v = v_;
   }

   ~Num() {

   }

};
