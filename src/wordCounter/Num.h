#pragma once
/*************************************************************************
 * Key::
 * Stores values by key
 */
class Num : public Object {
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
