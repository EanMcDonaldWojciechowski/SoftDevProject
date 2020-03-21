#pragma once
// #include "../dataframe/modified_dataframe.h"
#include "map.h"

class DataFrame;

class KVStore : public Object {
 public:
   Hashmap* store;
   //Client node;
   size_t nodeIndex;

   KVStore(size_t nodeIndex) {
     store = new Hashmap();
   }

   ~KVStore() {

    }

   void put(Key *k, DataFrame *v);


   DataFrame* get(Key *k);

   // DataFrame* waitAndGet(Key k) {
   //
   // }

   /*void initializeNetwork() {
     if (nodeIndex == 0) {

     }
     node = new Client()
   }*/

};
