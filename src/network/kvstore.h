#pragma once
// #include "../dataframe/modified_dataframe.h"

class DataFrame;

class ChunkStore : public Object {
public:
  Hashmap* store;
  Client *client;
  size_t nodeIndex;
  size_t basePort = 8810;


  ChunkStore(size_t nodeIndex_) {
    store = new Hashmap();
    nodeIndex = nodeIndex_;
    if (nodeIndex == 0) {
      // TODO: Create thread to create the server
      // Server *server = new Server("127.0.0.1");
      new std::thread(&Server::initialize, this);
      sleep(1);
      client = new Client("127.0.0.2", basePort + nodeIndex, store);
    } else if (nodeIndex == 1) {
      sleep(2);
      client = new Client("127.0.0.3", basePort + nodeIndex, store);
    } else if (nodeIndex == 2) {
      sleep(2);
      client = new Client("127.0.0.4", basePort + nodeIndex, store);
    }
   }

  ~ChunkStore() {

  }

  void put(Key *k, DataFrame *v);

  void sendInfo(Key *chunkKey, char* val) {
    if (chunkKey->nodeIndex == nodeIndex) {
      store->put(chunkKey, val);
    } else {
      client->sendMessage(basePort + chunkKey->nodeIndex, val);
    }

  }

  // TODO: add method to add values from an incoming client message
  // Add ChunkStore in Client constructor

};

class KVStore : public Object {
 public:
   ChunkStore *store;
   size_t nodeIndex;

   KVStore(size_t nodeIndex_) {
     nodeIndex = nodeIndex_;
     store = new ChunkStore(nodeIndex);
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
