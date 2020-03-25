#pragma once
// #include "helper.h"
// #include "object.h"
// #include "string.h"
// #include <stdio.h>
// #include <stdlib.h>

/*************************************************************************
 * Key::
 * Stores values by key
 */
 class Key : public Object {
   public:
     char* key;
     size_t nodeIndex;

     Key(char* key_, size_t nodeIndex_) {
       key = key_;
       nodeIndex = nodeIndex_;
     }

     ~Key() {
       delete[] key;
     }

     size_t hash() {
       unsigned long hash = 5381;
       int c;
       int i = 0;
       char *str = new char[strlen(key)];
       str = strcpy(str, key);
       while (c = str[i]) {
         hash = ((hash << 5) + hash) + c;
         i++;
       }
       delete[] str;
       hash_ = hash;
       return hash_;
     }

     bool equals(Object* o) {
       Key *castedO = dynamic_cast<Key*>(o);
       if (castedO == nullptr) {
           return false;
       }
       return (strcmp(this->key, castedO->key) == 0);
    }
 };

/*************************************************************************
 * Key::
 * Stores values by key
 */
class Value : public Object {
 public:
   char* value;

   Value(char* value_) {
     value = value_;
   }

   ~Value() {
     delete[] value;
   }

   char* dataToSend(Key* key) {
     // std::cout << "value before: " << value << "\n";
     char* data = new char[1024];
     data[1023] = '\0';
     char doubleChar[256];
     strcat(data, "PUT}");
     // std::cout << "this is key: " << key->key << "\n";
     strcat(data, key->key);
     strcat(data, "}");
     strcat(data, value);
     // std::cout << "value after: " << data << "\n";
     return data;
   }

   char* dataToRetrieve(Key* key, size_t chunkNum) {
     // std::cout << "value before: " << value << "\n";
     char* data = new char[1024];
     data[1023] = '\0';
     strcat(data, "GET}");
     // std::cout << "this is key: " << key->key << "\n";
     strcat(data, key->key);
     strcat(data, "_");
     char doubleChar[256];
     snprintf(doubleChar,sizeof(key->nodeIndex), "%d", key->nodeIndex);
     strcat(data, doubleChar);
     strcat(data, "_");
     char iIdxChar[256];
     snprintf(iIdxChar,sizeof(chunkNum), "%d", chunkNum);
     strcat(data, iIdxChar);
     strcat(data, "}");
     // std::cout << "value after: " << data << "\n";
     return data;
   }

};

// Hashmap_pair is a node, which is used to to store a pair of key and value.
// Later a list of Hashmap_pair will be stored into Hashmap data structure.
// It inherit Object class.
class Hashmap_pair : public Object {
    public:
        Object* key_;
        Object* val_;

        Hashmap_pair(Object *key, Object *val) : Object() {
            key_ = key;
            val_ = val;

        }
        ~Hashmap_pair() {
            delete key_;
            delete val_;
        }
};

// Hashmap class stores a list of hashmap_pairs, which contains equal number
// of keys and values.
// It has size and capcity, which size is the number of key-value pairs,
// and capcity is the physical size of hashmap.
class Hashmap : public Object {
    public:
        Hashmap_pair **data;
        size_t size_;
        size_t capacity_;
        size_t hash_code;
        //constructor
        //capcity will be initilized as 4, size is 0 by default.
        Hashmap() {
            data = new Hashmap_pair*[4];
            size_ = 0;
            capacity_ = 4;
            hash_code = 0;
            for (size_t i = 0; i < capacity_; i++) {
              data[i] = nullptr;
            }
        }

        Hashmap(size_t cap) {
          capacity_ = cap;
          size_ = 0;
          hash_code = 0;
          data = new Hashmap_pair*[capacity_];
          for (size_t i = 0; i < capacity_; i++) {
              data[i] = nullptr;
          }
        }

        // destructor
        ~Hashmap() {
            delete [] data;
        }

        // Double the capacity of hashmap when needed
        void expand() {
          Hashmap *copy = new Hashmap(capacity_ * 2);
          for (int i = 0; i < capacity_; i++) {
            if (!(data[i] == nullptr)) {
              copy->put(data[i]->key_, data[i]->val_);
            }
          }
          delete[] data;
          data = copy->data;
          capacity_ = capacity_ * 2;
        }

        // Returns the value to which the specified key is mapped,
        // or null if this map contains no mapping for the key.
        Object* get(Object *key) {
          size_t hashKey = (key->hash() % capacity_);
          //std::cout << "saved key " << data[hashKey]->key_ << " key " << key <<  " equals " << data[hashKey]->key_->equals(key) <<  "\n" ;
          while(!data[hashKey]->key_->equals(key)) {
            hashKey = (hashKey + 1) % capacity_;
          }
          return data[hashKey]->val_;
        }

        // Returns true if the key exists, otherwise false
        bool keyExists(Object* key) {
          size_t hashKey = (key->hash() % capacity_);
          size_t iteration = 0;
          while(!data[hashKey]->key_->equals(key)) {
            hashKey = (hashKey + 1) % capacity_;
            iteration += 1;
            if (iteration == capacity_ - 1) {
              return 0;
            }
          }
          return 1;
        }

        // Associates the specified value with the specified key in this map.
        void put(Object *key, Object *val) {
          if ((size_ + 1) * 2 > capacity_) {
            expand();
          }
          size_t hashKey = (key->hash() % capacity_);
          size_t i = hashKey;
          Hashmap_pair *temp = nullptr;
          Hashmap_pair *newObject = new Hashmap_pair(key, val);


          bool dupKey = false;
          // Logic for finding when the next spot in map's hash equals key's hash
          // Keep going until we reach the end of the line of objects in the map
          while(!(data[i] == nullptr)) {
            if (data[i]->key_->equals(key)) {
              dupKey = true;
              break;
            }
            // Either the hash of the object in the current spot is lower then \
            // the key's hash
            if ((data[i]->key_->hash() % capacity_) < hashKey) {
              // If so, we just look compare the next spot in the map and leave
              // this alone
              i = ((i + 1) % capacity_);
            } else {
              // otherwise we insert the key in the current spot and move the
              // thing that is in our spot forward
              temp = data[i];
              data[i] = newObject;
              newObject = temp;
              i = ((i + 1) % capacity_);
            }
          }
          if (!dupKey) {
            data[i] = newObject;
            size_++;
          }
        };

        // void printall() {
        //   for (size_t i = 0; i < capacity_; i++) {
        //     if (data[i] == nullptr) {
        //       std::cout << i << ": null \n";
        //     }
        //     else {
        //       String *castedO = dynamic_cast<String*>(data[i]->key_);
        //       std::cout << i << ": " << castedO->getStrValue() << "\n";
        //     }
        //   }
        //   std::cout << "\n";
        // }

        // Removes the mapping for the specified key from this map if present.
        void remove(Object *key) {
          size_t hashKey = (key->hash() % capacity_);
          //std::cout << "hashkey " << hashKey << "\n";
          size_t i = hashKey;
          while(!data[i]->key_->equals(key)) {
            // stop this loop if we cant find it. otherwise endless loop
            i = (i + 1) % capacity_;
          }
          data[i] = nullptr;
          size_--;
          i = (i + 1) % capacity_;
          Hashmap_pair *temp = nullptr;

          while(!(data[i] == nullptr) && (data[i]->key_->hash() % capacity_) != i) {
            temp = data[i];
            data[(i - 1) % capacity_] = temp;
            data[i] = nullptr;
            i = (i + 1) % capacity_;
          }


        }

        // Returns the number of key-value mappings in this map.
        size_t size() {
          return size_;
        }

        // Returns a list view of the keys contained in this map.
        Object** key_array() {
          Object ** ans = new Object*[size_];
          size_t iteration = 0;
          for (int i = 0; i < capacity_; i++) {
            if (!(data[i] == nullptr)){
              ans[iteration] = data[i]->key_;
              iteration++;
            }
          }
          return ans;
        }

        // Check if two Hashmaps are equal.
        // the input hashmap is an object.
        virtual bool equals(Object *map) { // TODO: change to virtual
          Hashmap *castedO = dynamic_cast<Hashmap*>(map);
          if (castedO == nullptr) {
              return false;
          }
          for (int i = 0; i < capacity_; i++) {
            if (data[i] == nullptr){
              if (!(castedO->data[i] == nullptr)) {
                return false;
              }
            } else {
              if (castedO->data[i] == nullptr)  {
                return false;
              }
              if (!data[i]->equals(castedO->data[i])) {
                return false;
              }
            }
          }
          return true;
        }

        // THIS IS OURS REMOVE LATER
        size_t hash() {
          Object **keys = key_array();
          size_t totalHash = capacity_;
          for (int i = 0; i < size_; i++) {
            totalHash += keys[i]->hash();
          }
          hash_code = totalHash;
          return totalHash;
        }
};
