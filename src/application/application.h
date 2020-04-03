#pragma once
/*************************************************************************
 * Key::
 * Stores values by key
 */
class Application : public Object {
 public:
   size_t nodeIndex;
   KVStore kv;

   Application(size_t nodeIndex_) {
     nodeIndex = nodeIndex_;
     kv = new KVStore(nodeIndex);
   }

   ~Application() {

   }




};
