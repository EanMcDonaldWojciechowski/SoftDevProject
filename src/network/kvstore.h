#pragma once
// #include "../dataframe/modified_dataframe.h"
#include "map.h"

class DataFrame;

class KVStore : public Object {
 public:
   ChunkStore store;
   size_t nodeIndex;

   KVStore(size_t nodeIndex_) {

   }

   ~KVStore() {

    }

   void put(Key *k, DataFrame *v) {
     store->put(k, v);
   }


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

class ChunkStore : public Object {
public:
  Hashmap* store;
  Client client;
  size_t nodeIndex;


  ChunkStore(size_t nodeIndex_) {
    store = new Hashmap();
    nodeIndex = nodeIndex_;
    if (nodeIndex == 0) {
      Server *server = new Server("127.0.0.1");
      sleep(1);
      client = new Client("127.0.0.2", 8811);
    } else if (nodeIndex == 1) {
      sleep(2);
      client = new Client("127.0.0.3", 8812);
    } else if (nodeIndex == 2) {
      sleep(2);
      client = new Client("127.0.0.4", 8813);
    }
   }

  ~ChunkStore() {

  }

  void put(Key *k, DataFrame *v) {

    Column* col = v->column[0];
    // serialize
    // add meta data for frist chunk
    char* data = new char[1024];
    data[1023] = '\0';
    char doubleChar[256];
    snprintf(doubleChar,sizeof(v->scm->length()), "%d", v->scm->length());
    strcat(data, doubleChar);
    strcat(data, "}");
    char *colType =  new char[2];
    colType[0] = v->scm->col_type(0);
    strcat(data, colType);
    strcat(data, "}");

    char* val;
    for (int i = 0; i < col->getNumChunks(); i++) {
      int storeClientLocation = i % 3; // TODO: change this 3 later to how many clients we have
      char* chunkStoreKey = new char[1024];
      strcat(chunkStoreKey, k->key);
      strcat(chunkStoreKey, "_");
      char nodeIdxChar[256];
      snprintf(nodeIdxChar,sizeof(i), "%d", key->nodeIndex);
      strcat(chunkStoreKey, nodeIdxChar);
      strcat(chunkStoreKey, "_");
      char iIdxChar[256];
      snprintf(iIdxChar,sizeof(i), "%d", i);
      strcat(chunkStoreKey, iIdxChar);

      Key *chunkKey = new Key(chunkStoreKey, storeClientLocation);

      if (col->get_type() == 'I') {
        val = col->as_int()->serializeChunk();
      } else if (col->get_type() == 'B') {
        val = col->as_bool()->serializeChunk();
      } else if (col->get_type() == 'F') {
        val = col->as_float()->serializeChunk();
      } else if (col->get_type() == 'S') {
        val = col->as_string()->serializeChunk();
      }
      // add meta data if fist chunk
      if (i == 0) {
        strcat(data, val);
        sendInfo(chunkKey, data);
      } else {
        sendInfo(chunkKey, val);
      }
    }

  }

  void sendInfo(Key *chunkKey, char* val) {
    if (chunkKey->nodeIndex == nodeIndex) {
      store->put(chunkKey, val);
    } else {
      // send data to right client
    }

  }

  // TODO: add method to add values from an incoming client message
  // Add ChunkStore in Client constructor

};
