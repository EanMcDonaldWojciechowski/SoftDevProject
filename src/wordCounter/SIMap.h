#pragma once
#include "../network/map.h"
/*************************************************************************
 * Key::
 * Stores values by key
 */
class SIMap : public Hashmap {
 public:


   SIMap() {

   }

   ~SIMap() {

   }

   // Returns the value to which the specified key is mapped,
   // or null if this map contains no mapping for the key.
   virtual Num* get(String *key1) {
     size_t hashKey = (key1->hash() % capacity_);
     // std::cout << "saved key " << dynamic_cast<Key*>(data[hashKey])->key << " key " << key1->key <<  " equals " << dynamic_cast<Key*>(data[hashKey])->equals(key) <<  "\n" ;
     while(!data[hashKey]->key_->equals(key1)) {
       hashKey = (hashKey + 1) % capacity_;
     }
     return data[hashKey]->val_;
   }

   // Returns true if the key exists, otherwise false
   virtual bool keyExists(String* key1) {
     size_t hashKey = (key1->hash() % capacity_);
     size_t iteration = 0;
     // printall();

     for (int i = 0; i < capacity_; i++) {
       if (data[hashKey] == nullptr) {
         continue;
       }
       if (data[hashKey]->key_->equals(key1)) {
         return 1;
       }
       hashKey = (hashKey + 1) % capacity_;
     }
     return 0;
   }

   // Associates the specified value with the specified key in this map.
   virtual void put(String *key1, Num *val1) {
     if ((size_ + 1) * 2 > capacity_) {
       expand();
     }
     size_t hashKey = (key1->hash() % capacity_);
     // std::cout << " key->hash() " << key1->hash() << "\n";
     // std::cout << " hashKey " << hashKey << " capacity_ " << capacity_ << "\n";
     size_t i = hashKey;
     // std::cout << "IN PUT i " << i << " hashKey " << hashKey << "\n";
     Hashmap_pair *temp = nullptr;
     Hashmap_pair *newObject = new Hashmap_pair(key1, val1);


     bool dupKey = false;
     // Logic for finding when the next spot in map's hash equals key's hash
     // Keep going until we reach the end of the line of objects in the map
     while(!(data[i] == nullptr)) {
       if (data[i]->key_->equals(key1)) {
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
     // std::cout << "IN PUT i " << i << " hashKey " << hashKey << "\n";
     if (!dupKey) {
       data[i] = newObject;
       size_++;
     }
   };

   virtual void printall() {
     for (size_t i = 0; i < capacity_; i++) {
       if (data[i] == nullptr) {
         std::cout << i << ": null \n";
       }
       else {
         String *castedO = dynamic_cast<String*>(data[i]->key_);
         Num *castedV = dynamic_cast<Num*>(data[i]->val_);
         std::cout << i << ": " << castedO->c_str() << " with hash location " << castedO->hash() % capacity_ << " with value " << castedV->v <<"\n";
       }
     }
     std::cout << "\n";
   }

};
