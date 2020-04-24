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
  int* neighborMapSizes;
  int* neighborMapCapacity;
  Server *server;
  std::thread *t1;
  char* clientIP;

  ChunkStore(size_t nodeIndex_, size_t num_nodes_) {
    store = new Hashmap();
    // std::cout << "\n\n\nOfficial Map chunkStore adr: " << store << "\n\n\n";
    nodeIndex = nodeIndex_;
    num_nodes = num_nodes_;
    neighborMapSizes = new int[num_nodes];
    neighborMapCapacity = new int[num_nodes];
    for (int k = 0; k < num_nodes; k++) {
      neighborMapSizes[k] = 0;
      neighborMapCapacity[k] = store->capacity_;
    }

    if (nodeIndex == 0) {
      server = new Server("127.0.0.1", num_nodes);
      t1 = new std::thread(&Server::initialize, server);
      t1->detach();
    } else {
      server = nullptr;
      t1 = nullptr;
    }

    clientIP = new char[256];
    memset(clientIP, 0, 256);
    strcat(clientIP, "127.0.0.");
    char* nodeIdxChar = new char[256];
    memset(nodeIdxChar, 0, 256);
    snprintf(nodeIdxChar, sizeof(nodeIndex + 2), "%d", nodeIndex + 2);
    strcat(clientIP, nodeIdxChar);
    std::cout << "Client IP is " << clientIP << "\n";
    client = new Client(clientIP, basePort + nodeIndex, store, num_nodes);
    delete[] nodeIdxChar;
   }

  ~ChunkStore() {
    delete client;
    delete[] clientIP;
    if (nodeIndex == 0) {
      t1->join();
      delete t1;
      delete server;
    }
  }

  void put(Key *k, DataFrame *v);

  DataFrame* waitAndGet(Key *k);

  DataFrame* get(Key *k);

  bool waitForKey(Key* k);

  Value* getChunkVal(size_t chunkNum, Key *ChunkKey);

  void sendInfo(Key *chunkKey, Value *val) {
    // std::cout << "Adding one to neighborMapSizes " << chunkKey->nodeIndex << " with current value " << neighborMapSizes[chunkKey->nodeIndex] << " and capacity " << neighborMapCapacity[chunkKey->nodeIndex] << "\n";
    neighborMapSizes[chunkKey->nodeIndex]++;
    if (chunkKey->nodeIndex == nodeIndex) {
      // std::cout << "Storing locally on key " << chunkKey->key << " with values " << val->value << "\n";
      store->put(chunkKey, val);
    } else {
      char* data = val->dataToSend(chunkKey);
      data[strlen(data)] = '{';
      client->sendMessage(basePort + chunkKey->nodeIndex, data);
      // Give hashmap time to expand
      if ((neighborMapSizes[chunkKey->nodeIndex]) * 2 > neighborMapCapacity[chunkKey->nodeIndex]) {
        std::cout << "Waiting for neighbor to grow their map...\n";
        usleep(250000);
        neighborMapCapacity[chunkKey->nodeIndex] = neighborMapCapacity[chunkKey->nodeIndex] * 2;
      }
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
    memset(chunkStoreKey, 0, 1024);
    strcat(chunkStoreKey, k->key);
    strcat(chunkStoreKey, "_");
    char *nodeIdxChar = new char[256];
    memset(nodeIdxChar, 0, 256);
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
