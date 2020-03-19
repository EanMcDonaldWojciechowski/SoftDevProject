#pragma once
#include "../dataframe/modified_dataframe.h"
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

   void put(Key *k, DataFrame *v) {
     // serialize
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

     Column* col = v->column[0];
     for (int i = 0; i < v->scm->length(); i++) {
       char doubleChar[256];
       if (col->get_type() == 'I') {
         snprintf(doubleChar,sizeof(col->as_int()->get(i)), "%d", col->as_int()->get(i));
       } else if (col->get_type() == 'B') {
         snprintf(doubleChar,sizeof(col->as_bool()->get(i)), "%d", col->as_bool()->get(i));
       } else if (col->get_type() == 'F') {
         snprintf(doubleChar,sizeof(col->as_float()->get(i)), "%f", col->as_float()->get(i));
       } else if (col->get_type() == 'S') {
         strcat(doubleChar, col->as_string()->get(i)->c_str());
       }
       strcat(data, doubleChar);
       strcat(data, "}");
     }

     // store
     Value *value = new Value(data);
     if (k->nodeIndex == nodeIndex) {
       store->put(k, value);
     } else {
       // serialize command
       // Send the put command to the right client
       //sendPut(k, v);
     }

   }

   DataFrame* get(Key *k) {
     if (k->nodeIndex == nodeIndex) {
       Value *gotValue = dynamic_cast<Value*>(store->get(k));
       size_t col_len = 0;
       char col_type = '\0';
       Column *c;
       size_t num;
       float numFloat;

       int charSize = strlen(gotValue->value);
       char* currField = new char[256];
       currField[255] = '\0';
       int setFieldNum = 0;
       for (int i = 0; i < charSize; i++) {
         if (gotValue->value[i] == '}' && setFieldNum == 0) {
           char* pEnd;
           col_len = strtod(currField, &pEnd);
           setFieldNum++;
           memset(currField, 0, strlen(currField));
           // std::cout<<"Column len is: "<< col_len << "\n";
           continue;
         } else if (gotValue->value[i] == '}' && setFieldNum == 1) {
           col_type = currField[0];
           if (col_type == 'I') {
             c = new IntColumn();
           } else if (col_type == 'B') {
             c = new BoolColumn();
           } else if (col_type == 'F') {
             c = new FloatColumn();
           } else if (col_type == 'S') {
             c = new StringColumn();
           }
           setFieldNum++;
           memset(currField, 0, strlen(currField));
           continue;
         } else if (gotValue->value[i] == '}' && setFieldNum > 1) {
           // std::cout<<"Adding currField to Column: "<< currField << "\n";
             if (col_type == 'I') {
               char* pEnd;
               num = strtol(currField, &pEnd, 10);
               c->as_int()->push_back((int)num);
               std::cout << c->size() << "\n";
             } else if (col_type == 'B') {
               char* pEnd;
               num = strtol(currField, &pEnd, 10);
               c->as_bool()->push_back((bool)num);
             } else if (col_type == 'F') {
               char* pEnd;
               numFloat = strtod(currField, &pEnd);
               c->as_float()->push_back((float)numFloat);
             } else if (col_type == 'S') {
               c = new StringColumn();
               String *str = new String(currField);
               c->as_string()->push_back(str);
             }
             setFieldNum++;
             memset(currField, 0, strlen(currField));
             continue;
         }
         char theval[2] = {0};
         theval[0] = gotValue->value[i];
         strcat(currField, theval);
         // std::cout<<"Current value of currField: "<< currField << "\n";
       }

       Schema *s = new Schema();
       DataFrame *ret = new DataFrame(*s);
       // std::cout << c->size() << "\n";
       ret->add_column(c);
       return ret;

     } else {
       // serialize command
       // Send the get command to the right client
       //get(k);
       // deserialize
     }

     return nullptr;
   }

   DataFrame* waitAndGet(Key k) {

   }

   /*void initializeNetwork() {
     if (nodeIndex == 0) {

     }
     node = new Client()
   }*/

};
