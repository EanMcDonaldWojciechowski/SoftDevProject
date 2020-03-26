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
      std::cout << "creating server \n";
      Server *server = new Server("127.0.0.1");
      std::cout << "creating server thread \n";
      std::thread *t1 = new std::thread(&Server::initialize, server);
      // t1->join();
      // sleep(1);
      std::cout << "creating client on server machine \n";
      client = new Client("127.0.0.2", basePort + nodeIndex, store);
    } else if (nodeIndex == 1) {
      // sleep(2);
      client = new Client("127.0.0.3", basePort + nodeIndex, store);
    } else if (nodeIndex == 2) {
      // sleep(2);
      client = new Client("127.0.0.4", basePort + nodeIndex, store);
    }
   }

  ~ChunkStore() {

  }

  void put(Key *k, DataFrame *v);

  DataFrame* get(Key *k);

  void waitForKey(Key* k);

  void sendInfo(Key *chunkKey, Value *val) {
    if (chunkKey->nodeIndex == nodeIndex) {
      store->put(chunkKey, val);
    } else {
      client->sendMessage(basePort + chunkKey->nodeIndex, val->dataToSend(chunkKey));
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
