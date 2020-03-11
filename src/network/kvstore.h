#pragma once


class KVStore : public Object {
 public:
   Hashmap store;
   Client node;
   size_t nodeIndex;

   KVStore(size_t nodeIndex) {
     initizlizeNetwork();

   }

   ~KVStore() {

   }

   void put(Key k, Value v) {

   }

   Object* get(Key k) {

   }

   Object* waitAndGet(Key k) {

   }

   void initializeNetwork() {
     if (nodeIndex == 0) {
       
     }
     node = new Client()
   }

};
