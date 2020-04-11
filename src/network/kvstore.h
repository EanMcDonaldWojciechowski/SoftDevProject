#pragma once
// #include "../dataframe/modified_dataframe.h"

class DataFrame;

class ChunkStore : public Object {
public:
  Hashmap* store;
  Client *client;
  size_t nodeIndex;
  size_t basePort = 8810;
  size_t num_nodes;

  ChunkStore(size_t nodeIndex_, size_t num_nodes_) {
    store = new Hashmap();
    nodeIndex = nodeIndex_;
    num_nodes = num_nodes_;

    if (nodeIndex == 0) {
      Server *server = new Server("127.0.0.1", num_nodes);
      std::thread *t1 = new std::thread(&Server::initialize, server);
    }

    char* clientIP = new char[256];
    memset(clientIP, 0, 256);
    strcat(clientIP, "127.0.0.");
    char* nodeIdxChar = new char[256];
    snprintf(nodeIdxChar, sizeof(nodeIndex + 2), "%d", nodeIndex + 2);
    strcat(clientIP, nodeIdxChar);
    std::cout << "Client IP is " << clientIP << "\n";
    client = new Client(clientIP, basePort + nodeIndex, store, num_nodes);

   }

  ~ChunkStore() {

  }

  void put(Key *k, DataFrame *v);

  DataFrame* waitAndGet(Key *k);

  DataFrame* get(Key *k);

  bool waitForKey(Key* k);

  Value* getChunkVal(size_t chunkNum, Key *ChunkKey);

  void sendInfo(Key *chunkKey, Value *val) {
    if (chunkKey->nodeIndex == nodeIndex) {
      usleep(10000);
      // std::cout << "Storing locally on key " << chunkKey->key << " with values " << val->value << "\n";
      store->put(chunkKey, val);
    } else {
      client->sendMessage(basePort + chunkKey->nodeIndex, val->dataToSend(chunkKey));
    }
  }

  Key* getChunkKey(Key* k, size_t clientNum, size_t chunkNum) {
    char* chunkStoreKey = new char[1024];
    memset(chunkStoreKey, 0, 1025);
    strcat(chunkStoreKey, k->key);
    strcat(chunkStoreKey, "_");
    char nodeIdxChar[256];
    snprintf(nodeIdxChar,sizeof(k->nodeIndex), "%d", k->nodeIndex);
    strcat(chunkStoreKey, nodeIdxChar);
    strcat(chunkStoreKey, "_");
    char iIdxChar[256];
    snprintf(iIdxChar,sizeof(chunkNum), "%d", chunkNum);
    strcat(chunkStoreKey, iIdxChar);
    Key *chunkKey = new Key(chunkStoreKey, clientNum);
    return chunkKey;
  }

  char* constructEndKey(Key *k) {
    char* chunkStoreKey = new char[1024];
    memset(chunkStoreKey, 0, 1025);
    strcat(chunkStoreKey, k->key);
    strcat(chunkStoreKey, "_");
    char nodeIdxChar[256];
    snprintf(nodeIdxChar,sizeof(k->nodeIndex), "%d", k->nodeIndex);
    strcat(chunkStoreKey, nodeIdxChar);
    strcat(chunkStoreKey, "_END");
    return chunkStoreKey;
  }
};

class KVStore : public Object {
 public:
   ChunkStore *store;
   size_t nodeIndex;
   size_t num_nodes;

   KVStore(size_t nodeIndex_, size_t num_nodes_) {
     nodeIndex = nodeIndex_;
     num_nodes = num_nodes_;
     store = new ChunkStore(nodeIndex, num_nodes);
   }

   ~KVStore() {

    }

   void put(Key *k, DataFrame *v);


   DataFrame* get(Key *k);

   DataFrame* waitAndGet(Key *k);

};
