#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include "helper.h"
#include "object.h"
#include "../network/map.h"
#include "string.h"
#include "../application/column.h"
#include "schema.h"
#include "fielder.h"
#include "fielderPrint.h"
#include "row.h"
#include "rower.h"
#include "printRower.h"
#include "RowerAddTwoToInts.h"
#include <thread>
#include "../network/network.h"
#include "../network/KVStore.h"
#include "../application/sorer.h"
// #include "../wordCounter/SIMap.h"
// #include "../wordCounter/wordCounter.h"
// #include "../linus/linus.h"

// class KVStore;
// class Key;
// class IntColumn;
// class BoolColumn;
// class FloatColumn;
// class StringColumn;
// class Schema;

/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 */
class DataFrame : public Object {
 public:
   Schema* scm;
   Column** column;

  /** Create a data frame with the same columns as the given df but with no rows or rownmaes */
  DataFrame(DataFrame& df) {
    scm = df.scm;
    column = df.column;
  }

  /** Create a data frame from a schema and columns. All columns are created
    * empty. */
  DataFrame(Schema& schema) {
    scm = &schema;
    column = new Column*[scm->width()];
    for (int i = 0; i < scm->width(); i++) {
      if (scm->col_type(i) == 'I') {
        IntColumn *intCol = new IntColumn(scm->length());
        column[i] = intCol;
      } else if (scm->col_type(i) == 'B') {
        BoolColumn *boolCol = new BoolColumn(scm->length());
        column[i] = boolCol;
      } else if (scm->col_type(i) == 'F') {
        FloatColumn *floatCol = new FloatColumn(scm->length());
        column[i] = floatCol;
      } else if (scm->col_type(i) == 'S') {
        StringColumn *strCol = new StringColumn(scm->length());
        column[i] = strCol;
      }
    }
  }

  /** Returns the dataframe's schema. Modifying the schema after a dataframe
    * has been created in undefined. */
  Schema& get_schema() {
    return *scm;
  }

  /** Adds a column this dataframe, updates the schema, the new column
    * is external, and appears as the last column of the dataframe, the
    * name is optional and external. A nullptr colum is undefined. */
  void add_column(Column* col) {;
    scm->add_column(col->get_type()); // scm width now 5

    Column** temp = new Column*[scm->width() + 1];
    for (int i = 0; i < scm->width() - 1; i++) {
      temp[i] = column[i];
    }
    temp[scm->width() - 1] = col->clone();
    delete[] column;
    column = temp;
    if (scm->length() == 0) {
      if (col->get_type() == 'I') {
        // std::cout<< "value at intcols: " << col->as_int()->get(1) << "\n";
        scm->colSize_ = col->as_int()->count_;
      } else if (col->get_type() == 'B') {
        // std::cout<< "value at boolcols: " << col->as_bool()->get(1) << "\n";
        scm->colSize_ = col->as_bool()->count_;
      } else if (col->get_type() == 'F') {
        // std::cout<< "value at floatcols: " << col->as_float()->get(1) << "\n";
        scm->colSize_ = col->as_float()->count_;
      } else if (col->get_type() == 'S') {
        // std::cout<< "value at strcols: " << col->as_string()->get(1)->c_str() << "\n";
        scm->colSize_ = col->as_string()->count_;
      }

    }
    // else if (scm->length() != col->count_) {
    //   std::cout << "Error columns are different lengths.\n";
    //   exit(1);
    // }
  }

  /** Return the value at the given column and row. Accessing rows or
   *  columns out of bounds, or request the wrong type is undefined.*/
  int get_int(size_t col, size_t row) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'I') {
      exit(1);
    }
    return column[col]->as_int()->get(row);
  }
  bool get_bool(size_t col, size_t row) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'B') {
      exit(1);
    }
    return column[col]->as_bool()->get(row);
  }
  float get_float(size_t col, size_t row) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'F') {
      exit(1);
    }
    return column[col]->as_float()->get(row);
  }
  String* get_string(size_t col, size_t row) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'S') {
      exit(1);
    }
    return column[col]->as_string()->get(row);
  }

  /** Set the value at the given column and row to the given value.
    * If the column is not  of the right type or the indices are out of
    * bound, the result is undefined. */
  void set(size_t col, size_t row, int val) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'I') {
      exit(1);
    }
    column[col]->as_int()->set(row, val);
  }
  void set(size_t col, size_t row, bool val) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'B') {
      exit(1);
    }
    column[col]->as_bool()->set(row, val);
  }
  void set(size_t col, size_t row, float val) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'F') {
      exit(1);
    }
    column[col]->as_float()->set(row, val);
  }
  void set(size_t col, size_t row, String* val) {
    if (row >= scm->length() || col >= scm->width()) {
      exit(1);
    }
    if (scm->col_type(col) != 'S') {
      exit(1);
    }
    column[col]->as_string()->set(row, val);
  }

  /** Set the fields of the given row object with values from the columns at
    * the given offset.  If the row is not form the same schema as the
    * dataframe, results are undefined.
    */
  void fill_row(size_t idx, Row& row) {
    // std::cout << "FILL ROW ROW WIDTH " << scm->width() << "\n";
    for (int i = 0; i < scm->width(); i++) {
      if (row.col_type(i) == 'I') {
        row.set(i, column[i]->as_int()->get(idx));
      } else if (row.col_type(i) == 'B') {
        row.set(i, column[i]->as_bool()->get(idx));
      } else if (row.col_type(i) == 'F') {
        row.set(i, column[i]->as_float()->get(idx));
      } else if (row.col_type(i) == 'S') {
        row.set(i, column[i]->as_string()->get(idx));

      }
      row.set_idx(idx);
    }
    // std::cout << "FILL ROW done \n";
  }

  /** Add a row at the end of this dataframe. The row is expected to have
   *  the right schema and be filled with values, otherwise undedined.  */
  void add_row(Row& row) {
    scm->add_row();
    if (row.width() != scm->width()) {
      exit(1);
    }
    for (int i = 0; i < scm->width(); i++) {
      if (row.col_type(i) == 'I') {
        column[i]->push_back(row.get_int(i));
      } else if (row.col_type(i) == 'B') {
        column[i]->push_back(row.get_bool(i));
      } else if (row.col_type(i) == 'F') {
        column[i]->push_back(row.get_float(i));
      } else if (row.col_type(i) == 'S') {
        column[i]->push_back(row.get_string(i));
      }

    }
  }

  /** The number of rows in the dataframe. */
  size_t nrows() {
    return scm->colSize_;
  }

  /** The number of columns in the dataframe.*/
  size_t ncols() {
    return scm->rowSize_;
  }

  /** Visit rows in order */
  void map(Rower& r) {
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      s1->add_column(scm->col_type(i));
    }
    Row *currRow = new Row(*s1);
    DataFrame *retDF = new DataFrame(*s1);
    // print();
    // std::cout<< "SCHEMA len ..... "  << scm->length() << "\n";
    for (int j = 0; j < scm->length(); j++) {
      // std::cout << "j = " << j << "\n";
      // std::cout<< "SCHEMA len ..... "  << scm->length() << "\n";
      this->fill_row(j, *currRow);
      // std::cout << "BEFORE Accept \n";
      r.accept(*currRow);
      // std::cout << "BEFORE VISIT \n";
      r.visit(*currRow);
      // std::cout << "AFTER VISIT \n";
      retDF->add_row(*currRow);
      // std::cout << "AFTER add row \n\n\n\n";
    }
    delete scm;
    delete[] column;
    scm = retDF->scm;
    column = retDF->column;
  }

  /** Visit subset of rows in order */
  void map_(Rower& r, size_t start, size_t end) {
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      s1->add_column(scm->col_type(i));
    }

    Row *currRow = new Row(*s1);
    for (int j = start; j < end; j++) {
      this->fill_row(j, *currRow);
      r.accept(*currRow);
      for (int k = 0; k < scm->width(); k++) {
        if (currRow->col_type(k) == 'I') {
          set(k, currRow->get_idx(), currRow->cols[k]->as_int()->get(0));
        } else if (currRow->col_type(k) == 'B') {
          set(k, currRow->get_idx(), currRow->cols[k]->as_bool()->get(0));
        } else if (currRow->col_type(k) == 'F') {
          set(k, currRow->get_idx(), currRow->cols[k]->as_float()->get(0));
        } else if (currRow->col_type(k) == 'S') {
          set(k, currRow->get_idx(), currRow->cols[k]->as_string()->get(0));
        }
      }
    }
  }



  /** Visit rows in P */
  void pmap(Rower& r) {
    Rower* r_clone = r.clone();
    if (scm->length() < 100) {
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, scm->length());

      t1.join();

    } else if (scm->length() >= 100 && scm->length() < 500) {
      size_t div_nrow = scm->length() / 2;
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, div_nrow);
      std::thread t2(&DataFrame::map_, this, std::ref(*r_clone), div_nrow, scm->length());

      t1.join();
      t2.join();

    } else if (scm->length() >= 500 && scm->length() < 1000) {
      size_t div_nrow = scm->length() / 3;
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, div_nrow);
      std::thread t2(&DataFrame::map_, this, std::ref(*r_clone), div_nrow, 2*div_nrow);
      std::thread t3(&DataFrame::map_, this, std::ref(*r_clone), 2*div_nrow, scm->length());

      t1.join();
      t2.join();
      t3.join();

    } else if (scm->length() >= 1000 && scm->length() < 5000) {
      size_t div_nrow = scm->length() / 4;
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, div_nrow);
      std::thread t2(&DataFrame::map_, this, std::ref(*r_clone), div_nrow, 2*div_nrow);
      std::thread t3(&DataFrame::map_, this, std::ref(r), 2*div_nrow, 3*div_nrow);
      std::thread t4(&DataFrame::map_, this, std::ref(*r_clone), 3*div_nrow, scm->length());

      t1.join();
      t2.join();
      t3.join();
      t4.join();

    } else if (scm->length() >= 5000 && scm->length() < 20000) {
      size_t div_nrow = scm->length() / 5;
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, div_nrow);
      std::thread t2(&DataFrame::map_, this, std::ref(*r_clone), div_nrow, 2*div_nrow);
      std::thread t3(&DataFrame::map_, this, std::ref(r), 2*div_nrow, 3*div_nrow);
      std::thread t4(&DataFrame::map_, this, std::ref(*r_clone), 3*div_nrow, 4*div_nrow);
      std::thread t5(&DataFrame::map_, this, std::ref(r), 4*div_nrow, scm->length());

      t1.join();
      t2.join();
      t3.join();
      t4.join();
      t5.join();

    } else {
      size_t div_nrow = scm->length() / 6;
      std::thread t1(&DataFrame::map_, this, std::ref(r), 0, div_nrow);
      std::thread t2(&DataFrame::map_, this, std::ref(*r_clone), div_nrow, 2*div_nrow);
      std::thread t3(&DataFrame::map_, this, std::ref(r), 2*div_nrow, 3*div_nrow);
      std::thread t4(&DataFrame::map_, this, std::ref(*r_clone), 3*div_nrow, 4*div_nrow);
      std::thread t5(&DataFrame::map_, this, std::ref(r), 4*div_nrow, 5*div_nrow);
      std::thread t6(&DataFrame::map_, this, std::ref(r), 5*div_nrow, scm->length());

      t1.join();
      t2.join();
      t3.join();
      t4.join();
      t5.join();
      t6.join();

    }


    // std::cout  << "schema len after pmap: "<< scm->length() <<  " \n";
  }

  /** Create a new dataframe, constructed from rows for which the given Rower
    * returned true from its accept method. */
  DataFrame* filter(Rower& r) {
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      s1->add_column(scm->col_type(i));
    }
    Row *currRow = new Row(*s1);
    DataFrame *retDF = new DataFrame(*s1);
    for (int j = 0; j < scm->length(); j++) {
      this->fill_row(j, *currRow);
      if (r.accept(*currRow)) {
        retDF->add_row(*currRow);
      }
    }
    return retDF;
  }

  /** Print the dataframe in SoR format to standard output. */
  void print() {
    Row *r = new Row(*scm);
    std::cout << "len " << scm->length() << " \n";
    for (int i = 0; i < scm->length(); i++) {
      this->fill_row(i, *r);
      PrintRower* pr = new PrintRower();
      pr->accept(*r);
    }
  }

  /** Stores float values in KVStore and returns it as a dataframe */
  DataFrame* fromArray(Key *key, KVStore *kv, size_t size, float* vals) {
    Column *c = new FloatColumn();
    for (int i = 0; i < size; i++) {
      c->as_float()->push_back((float)vals[i]);
    }
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    df->add_column(c);

    kv->put(key, df);
    return df;
  }

  /** Stores int values in KVStore and returns it as a dataframe */
  DataFrame* fromArray(Key *key, KVStore *kv, size_t size, int* vals) {
    Column *c = new IntColumn();
    for (int i = 0; i < size; i++) {
      c->as_int()->push_back((int)vals[i]);
    }
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    df->add_column(c);

    kv->put(key, df);
    return df;
  }

  /** Stores bool values in KVStore and returns it as a dataframe */
  DataFrame* fromArray(Key *key, KVStore *kv, size_t size, bool* vals) {
    Column *c = new BoolColumn();
    for (int i = 0; i < size; i++) {
      c->as_bool()->push_back((bool)vals[i]);
    }
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    df->add_column(c);

    kv->put(key, df);
    return df;
  }

  /** Stores String values in KVStore and returns it as a dataframe */
  DataFrame* fromArray(Key *key, KVStore *kv, size_t size, String** vals) {
    Column *c = new StringColumn();
    for (int i = 0; i < size; i++) {
      c->as_string()->push_back(vals[i]);
    }
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    df->add_column(c);

    kv->put(key, df);
    return df;
  }

  /** Stores int values in KVStore and returns it as a dataframe */
  DataFrame* fromScalar(Key *key, KVStore *kv, int val) {
    int *vals = new int[1];
    vals[0] = val;
    return fromArray(key, kv, 1, vals);
  }

  /** Stores int values in KVStore and returns it as a dataframe */
  DataFrame* fromScalar(Key *key, KVStore *kv, float val) {
    float *vals = new float[1];
    vals[0] = val;
    return fromArray(key, kv, 1, vals);
  }

  /** Stores int values in KVStore and returns it as a dataframe */
  DataFrame* fromScalar(Key *key, KVStore *kv, bool val) {
    bool *vals = new bool[1];
    vals[0] = val;
    return fromArray(key, kv, 1, vals);
  }

  /** Stores int values in KVStore and returns it as a dataframe */
  DataFrame* fromScalar(Key *key, KVStore *kv, String* val) {
    String **vals = new String*[1];
    vals[0] = val;
    return fromArray(key, kv, 1, vals);
  }

  DataFrame* fromVisitor(Key* key, KVStore *kv, const char* colType, Writer *vals) {
    size_t numCols = strlen(colType);
    Schema *s = new Schema(colType);
    DataFrame *df = new DataFrame(*s);
    Row *r = new Row(*s);
    // std::cout<<"DF in from Visitor Entering while loop: " << " Done ? " << vals->done() << " with colType " <<  colType <<" \n";

    while (!vals->done()) {
      // std::cout<<"DF in from Visitor IN while loop\n";
      vals->visit(*r);
      // std::cout << "Row after visit \n";
      if (s->rowSize_ > 1) {
        // std::cout << "ROW value after visit: " << r->get_string(0)->c_str() << " : " << r->get_int(1) << "\n";
      }
      df->add_row(*r);
      // std::cout<<"DF in from Visitor IN while loop3\n";
    }
    // std::cout<<"df in from visitor \n";
    // df->print();

    kv->put(key, df);
    // std::cout<<"df in from visitor AFTER PUT\n";
    // df->print();
    return df;
  }

  DataFrame* fromFile(const char* file, Key *key, KVStore* kv) {
    // char* file_name = "../data/sampleData.sor";
    std::cout << "Opening file path " << file << "\n";
    FILE *f = fopen(file, "r");
    if (f == NULL) {
      std::cout << "ERROR opening file." << "\n";
    }
    SOR* reader = new SOR();
    // reader->read(f, 0, 5000000000);
    reader->read(f, 0, 50000000);
    // reader->cols_[0]->printCol();
    std::cout << "reading file" << "\n";
    DataFrame *df = reader->sorToDataframe();
    std::cout << "This is file contents I found\n";
    kv->put(key, df);
    // df->print();
    // sleep(10);
    delete reader;
    return df;
  }
};


void KVStore::put(Key *k, DataFrame *v) {
  std::cout<<"KVSTORE put INITIAL KEY : " << k->key << "\n";
  size_t numCols = v->scm->width();



  char *colType = v->scm->colType;
  // std::cout<<"colTypecolTypecolTypecolTypecolType: " << colType << "\n";
  String *metaDataStr = new String(colType);
  Column *c = new StringColumn();
  c->push_back(metaDataStr);
  Schema *s = new Schema();
  DataFrame *metaData = new DataFrame(*s);
  metaData->add_column(c);

  // std::cout<<"Putting metadata df in key "<< k->key <<"\n";
  // metaData->print();

  store->put(k, metaData);

  // std::cout<<"second2\n";

  Key *colKey;
  for (int i = 0; i < numCols; i++) {
    char* colKeyChar = new char[1024];
    memset(colKeyChar, 0, 1025);
    strcat(colKeyChar, k->key);
    strcat(colKeyChar, "_");
    char nodeIdxChar[256];
    memset(nodeIdxChar, 0, 256);
    snprintf(nodeIdxChar,sizeof(i), "%d", i);
    strcat(colKeyChar, nodeIdxChar);
    strcat(colKeyChar, "_");
    char *colTypeChar = new char[2];
    memset(colTypeChar, 0, 2);
    colTypeChar[0] = colType[i];
    colTypeChar[1] = '\0';
    strcat(colKeyChar, colTypeChar);

    // std::cout<<"third colKeyChar is " << colKeyChar << " \n";

    Schema *colS = new Schema();
    DataFrame *colDf = new DataFrame(*colS);
    colDf->add_column(v->column[i]);
    // std::cout << "ith col i = " << i << " has the following type " << v->column[i]->get_type() << "\n";

    colKey = new Key(colKeyChar, k->nodeIndex);
    // std::cout<< "kvstore ith col = " << i <<" KEY : " << colKey->key << "\n";
    // std::cout<<"forth\n";
    store->put(colKey, colDf);
    delete colDf;
    delete colS;
  }

  char* chunkStoreKey = new char[1024];
  memset(chunkStoreKey, 0, 1024);
  strcat(chunkStoreKey, k->key);
  strcat(chunkStoreKey, "_DONE");
  Value *dataValFinal = new Value(colType);
  for (int i = 0; i < num_nodes; i++) {
    Key *chunkKeyFinal = new Key(chunkStoreKey, i);
    // std::cout<< "kvstore ith col = " << i <<" done KEY : " << chunkKeyFinal->key << "\n";
    // usleep(10000);

    store->sendInfo(chunkKeyFinal, dataValFinal);
  }
}


DataFrame* KVStore::get(Key *k) {
  // if (!store->store->keyExists(k)) {
  //   std::cout << "Key doesn't exist. \n";
  //   exit(1);
  // }
  std::cout<< "kvstore getting main key " << k->key << "\n";
  char* colKeyChar = new char[1024];
  memset(colKeyChar, 0, 1024);
  strcat(colKeyChar, k->key);
  strcat(colKeyChar, "_DONE");
  Key *chunkKey = new Key(colKeyChar, nodeIndex);
  // std::cout<< "kvstore getting chunkKey key " << chunkKey->key << "\n";
  while(!store->waitForKey(chunkKey)) {
    // std::cout<<"Looking for key " << colKeyChar << " with nodeidx " << nodeIndex << "\n";
    continue;
  }
  // std::cout<<"FOUND KEY MOVING ON \n";
  Schema *colS = new Schema();
  DataFrame *retDf = new DataFrame(*colS);

  Value *metaValue = dynamic_cast<Value*>(store->store->get(chunkKey));
  // std::cout<<"GOT Metadata \n";
  String *colTypes = new String(metaValue->value);

  char type;
  DataFrame *singleColDF;
  Key *colKey;
  for (int i = 0; i < strlen(colTypes->c_str()); i++) {
    type = colTypes->c_str()[i];

    char* colKeyChar = new char[1024];
    memset(colKeyChar, 0, 1025);
    strcat(colKeyChar, k->key);
    strcat(colKeyChar, "_");
    char nodeIdxChar[256];
    snprintf(nodeIdxChar,sizeof(i), "%d", i);
    strcat(colKeyChar, nodeIdxChar);
    strcat(colKeyChar, "_");
    char *typeStr = new char[2];
    typeStr[0] = type;
    typeStr[1] = '\0';
    strcat(colKeyChar, typeStr);
    colKey = new Key(colKeyChar, k->nodeIndex);

    singleColDF = store->get(colKey);
    retDf->add_column(singleColDF->column[0]);
  }

  return retDf;
}

DataFrame* KVStore::waitAndGet(Key *k) {
  char* colKeyChar = new char[1024];
  memset(colKeyChar, 0, 1024);
  strcat(colKeyChar, k->key);
  strcat(colKeyChar, "_DONE");
  Key *chunkKey = new Key(colKeyChar, k->nodeIndex);
  // std::cout << "BEFORE WAITING FOR END KEY " << colKeyChar << " \n";
  // store->store->printall();
  while(!store->waitForKey(chunkKey)) {
    continue;
  }
  // std::cout << "AFTER WAITING FOR END KEY \n";

  Schema *colS = new Schema();
  DataFrame *retDf = new DataFrame(*colS);

  // std::cout << "BEFORE get " << chunkKey->key << " \n";
  // DataFrame *metaDF = store->get(k);
  Value *metaValue = dynamic_cast<Value*>(store->store->get(chunkKey));
  // std::cout<<"GOT Metadata \n";
  // String *colTypes = metaDF->get_string(0,0);
  char *colTypes = metaValue->value;

  char type;
  DataFrame *singleColDF;
  Key *colKey;
  for (int i = 0; i < strlen(colTypes); i++) {
    type = colTypes[i];
    // std::cout << "finding column #" << i << " type " << type << "\n";

    char* colKeyChar = new char[1024];
    memset(colKeyChar, 0, 1025);
    strcat(colKeyChar, k->key);
    strcat(colKeyChar, "_");
    char nodeIdxChar[256];
    snprintf(nodeIdxChar,sizeof(i), "%d", i);
    strcat(colKeyChar, nodeIdxChar);
    strcat(colKeyChar, "_");
    char *typeStr = new char[2];
    typeStr[0] = type;
    typeStr[1] = '\0';
    strcat(colKeyChar, typeStr);
    colKey = new Key(colKeyChar, k->nodeIndex);
    // std::cout << "In KVStore wait and get.. waiting for key " << colKeyChar << " \n";
    singleColDF = store->waitAndGet(colKey);
    retDf->add_column(singleColDF->column[0]);
  }

  return retDf;
}


void ChunkStore::put(Key *k, DataFrame *v) {
  std::cout<< "chunk store initial key " << k->key << "\n";

  Column* col = v->column[0];
  // std::cout << "inside CS put col has type " << v->column[0]->get_type() << "\n";
  char* data = col->serializeMetadata();
  char* val;
  for (int i = 0; i < col->getNumChunks(); i++) {
    if (i >= ((col->size() / col->getSizeOfChunk()) + 1)) {
      // std::cout << "Breaking in chunkstore put bc read all data when i is " << i << " >=  col->size " << col->size() << " and chunkSize is " << col->getSizeOfChunk() << "\n";
      break;
    }
    int storeClientLocation = i % num_nodes;
    Key *chunkKey = getChunkKey(k, storeClientLocation, i);
    // std::cout<< "chunk store getChunkKey for i = " << i << " chunkKey: " << chunkKey->key << "\n";
    if (col->get_type() == 'I') {
      val = col->as_int()->serializeChunk(i);
    } else if (col->get_type() == 'B') {
      val = col->as_bool()->serializeChunk(i);
    } else if (col->get_type() == 'F') {
      val = col->as_float()->serializeChunk(i);
    } else if (col->get_type() == 'S') {
      val = col->as_string()->serializeChunk(i);
    }
    // std::cout<< "serialized chunk\n";
    if (i == 0) {
      strcat(data, val);
      Value *dataVal = new Value(data);
      sendInfo(chunkKey, dataVal);
    } else {
      Value *dataVal = new Value(val);
      sendInfo(chunkKey, dataVal);
    }
  }
  char* chunkStoreKey = constructEndKey(k);

  char *finalVal = new char[2];
  memset(finalVal, 0, 2);
  finalVal[1] = '\0';
  finalVal[0] = col->get_type();
  Value *dataValFinal = new Value(finalVal);
  for (int k = 0; k < num_nodes; k++) {
    Key *chunkKeyFinal = new Key(chunkStoreKey, k);
    // std::cout<< "chunk store constructEndKey for k = " << k << " key: " << chunkKeyFinal->key << "\n";
    // std::cout<< "with values: " << finalVal << "\n";
    sendInfo(chunkKeyFinal, dataValFinal);
  }
}

DataFrame* ChunkStore::get(Key *k) {
  std::cout << "In chunkstore get main key " << k->key << "\n";
  char* endKey = constructEndKey(k);
  Key *finalKey = new Key(endKey, k->nodeIndex);
  // std::cout << "In chunkstore get waiting for key " << finalKey->key << "\n";
  while(!waitForKey(finalKey)) {
    continue;
  }
  Value *finalVal = dynamic_cast<Value*>(store->get(finalKey));
  Column *col;
  if (finalVal->value[0] == 'I') {
    col = new IntColumn();
  } else if (finalVal->value[0] == 'B') {
    col = new BoolColumn();
  } else if (finalVal->value[0] == 'F') {
    col = new FloatColumn();
  } else if (finalVal->value[0] == 'S') {
    col = new StringColumn();
  } else {
    std::cout << "CHUNKSTORE ERROR: finalVal not found\n.";
    exit(1);
  }

  char* chunkStoreKey = new char[1024];
  memset(chunkStoreKey, 0, 1025);
  strcat(chunkStoreKey, k->key);
  strcat(chunkStoreKey, "_");
  char nodeIdxChar[256];
  snprintf(nodeIdxChar,sizeof(k->nodeIndex), "%d", k->nodeIndex);
  strcat(chunkStoreKey, nodeIdxChar);
  strcat(chunkStoreKey, "_");
  Key *parentKey = new Key(chunkStoreKey, k->nodeIndex);
  char* firstChunkStoreKey = new char[1024];
  strcat(firstChunkStoreKey, chunkStoreKey);
  strcat(firstChunkStoreKey, "0");
  Key** subKeys = store->getSubKeys(parentKey);
  size_t i = 0;
  Value *chunkVal;
  // std::cout << "firstChunkStoreKey is " << firstChunkStoreKey << " with parent key " << parentKey->key << "\n";
  while (subKeys[i] != nullptr) {
    // std::cout << "Looking for subkey: " << subKeys[i]->key << "\n";
    chunkVal = dynamic_cast<Value*>(store->get(subKeys[i]));

    // std::cout << "Print i " << i << "\n";
    if (strcmp(subKeys[i]->key, firstChunkStoreKey) == 0) {
      // std::cout << "Print ChunkVal 0 before taking off metadata: " << chunkVal->value << "\n";
      size_t fieldNum = 0;
      int m;
      for (m = 0; m < strlen(chunkVal->value); m++) {
        // std::cout << "current char " << chunkVal->value[m] << "\n";
        // std::cout<<"fieldNum : " << fieldNum << "\n";
        if (chunkVal->value[m] == '}') {
          fieldNum++;
        }
        if (fieldNum == 3) {
          break;
        }
      }
      char *val = new char[1024];
      memcpy(val, &chunkVal->value[m + 1], (strlen(chunkVal->value) - m + 1));
      // std::cout << "Print val after taking off metadata: " << val << "\n";
      chunkVal->value = val;
    }
    // std::cout << "Print chunkstore finalVal: " << finalVal->value << "\n";
    // std::cout << "Print chunkstore value: " << chunkVal->value << "\n";
    if (finalVal->value[0] == 'I') {
      col->as_int()->deserializeChunk(chunkVal->value);
    } else if (finalVal->value[0] == 'B') {
      col->as_bool()->deserializeChunk(chunkVal->value);
    } else if (finalVal->value[0] == 'F') {
      col->as_float()->deserializeChunk(chunkVal->value);
    } else if (finalVal->value[0] == 'S') {
      col->as_string()->deserializeChunk(chunkVal->value);
    } else {
      std::cout << "CHUNKSTORE ERROR again: finalVal not found\n.";
      exit(1);
    }
    // std::cout << "done with loop ... " << i << "\n";
    i++;
  }
  // std::cout << "out of loop.......\n";
  Schema *s = new Schema();
  DataFrame *df = new DataFrame(*s);
  // std::cout << "out of loop2.......\n";
  df->add_column(col);
  // std::cout << "done.......\n";
  return df;
}

DataFrame* ChunkStore::waitAndGet(Key *k) {
  Value *firstChunk;
  Key *firstChunkKey = getChunkKey(k, k->nodeIndex, 0);

  if (nodeIndex == 0) {
    // std::cout << "Chunk store WaitandGet waiting for key " << firstChunkKey->key << "\n";
    while(!waitForKey(firstChunkKey)) {
      continue;
    }
    // std::cout << "Found the key...\n";
    firstChunk = dynamic_cast<Value*>(store->get(firstChunkKey));
    // std::cout << "Retrieved the key...\n";
  } else {
    // std::cout << "starting else ...\n";
    char* data = new char[1024];
    memset(data, 0, 1024);
    strcat(data, "GET}");
    strcat(data, firstChunkKey->key);
    strcat(data, "}");
    // std::cout << "Chunk store WaitandGet requesting key " << firstChunkKey->key << "\n";
    data[strlen(data)] = '{';
    client->sendMessage(basePort + 0, data);

    char *keyVal = new char[4];
    memset(keyVal, 0, 4);
    strcat(keyVal, "RSP");
    Key *gotKey = new Key(keyVal, nodeIndex);
    std::cout << "waiting for rsp ...\n";
    while (!waitForKey(gotKey)) {
      std::cout << "1 seconds has expired, resending ...\n";
      client->sendMessage(basePort + 0, data);
      // continue;
     }

    firstChunk = dynamic_cast<Value*>(store->get(gotKey));
    // std::cout << "Found the key RSP...\n";
    // usleep(10000);
    store->remove(gotKey);
    // std::cout << "removing rsp...\n";
  }

  size_t numChunks;
  size_t colLen;
  char colType;
  char* keyChar = new char[256];
  size_t fieldNum = 0;
  int i;
  // std::cout << "Parsing metadata ...\n";
  for (i = 0; i < strlen(firstChunk->value); i++) {
    if (firstChunk->value[i] == '}') {
      if (fieldNum == 0) {
        char* pEnd;
        numChunks = strtol(keyChar, &pEnd, 10);
        fieldNum += 1;
        memset(keyChar, 0, 256);
      } else if (fieldNum == 1) {
        char* pEnd;
        colLen = strtol(keyChar, &pEnd, 10);
        fieldNum += 1;
        memset(keyChar, 0, 256);
      } else if (fieldNum == 2) {
        colType = keyChar[0];
        memset(keyChar, 0, 256);
        break;
      }
      continue;
    }
    char theval[2] = {0};
    theval[0] = firstChunk->value[i];
    strcat(keyChar, theval);
  }

  char *val = new char[1024];
  memcpy(val, &firstChunk->value[i + 1], (strlen(firstChunk->value) - i + 1));

  // std::cout << "building  columns ...\n";
  Column *col;
  if (colType == 'I') {
    col = new IntColumn();
    col->as_int()->deserializeChunk(val);
  } else if (colType == 'B') {
    col = new BoolColumn();
    col->as_bool()->deserializeChunk(val);
  } else if (colType == 'F') {
    col = new FloatColumn();
    col->as_float()->deserializeChunk(val);
  } else if (colType == 'S') {
    col = new StringColumn();
    col->as_string()->deserializeChunk(val);
  }
  // std::cout << "deserialized chunk ...\n";

  Key *chunkKey;
  char* chunkKeyText = new char[1024];
  for (int j = 1; j < numChunks; j++) {
    chunkKey = getChunkKey(k, k->nodeIndex, j);
    Value *chunkVal = getChunkVal(j, chunkKey);
    // std::cout << "got value from getChunkVal ...\n";
    if (colType == 'I') {
      col->as_int()->deserializeChunk(chunkVal->value);
    } else if (colType == 'B') {
      col->as_bool()->deserializeChunk(chunkVal->value);
    } else if (colType == 'F') {
      col->as_float()->deserializeChunk(chunkVal->value);
    } else if (colType == 'S') {
      col->as_string()->deserializeChunk(chunkVal->value);
    }
  }

  Schema *s = new Schema();
  DataFrame *retdf = new DataFrame(*s);
  retdf->add_column(col);
  // std::cout << "returning dataframe ...\n";
  return retdf;
}

Value* ChunkStore::getChunkVal(size_t chunkNum, Key *chunkKey) {
  // std::cout << "inside get chunk val ...\n";
  Value *chunkData;
  size_t whichNode = chunkNum % num_nodes;
  if (nodeIndex == whichNode) {
    // std::cout << "waiting for chunkKey " << chunkKey->key << " ...\n";
    while(!waitForKey(chunkKey)) {
      continue;
    }
    chunkData = dynamic_cast<Value*>(store->get(chunkKey));
  } else {
    char* data = new char[1024];
    memset(data, 0, 1024);
    strcat(data, "GET}");
    strcat(data, chunkKey->key);
    strcat(data, "}");
    // std::cout << "sending get msg : " << data << " ...\n";
    data[strlen(data)] = '{';
    client->sendMessage(basePort + whichNode, data);
    char *keyVal = new char[4];
    memset(keyVal, 0, 4);
    strcat(keyVal, "RSP");
    Key *gotKey = new Key(keyVal, nodeIndex);
    std::cout << "waiting for rsp : for data ... " << data << " \n";
    while(!waitForKey(gotKey)) {
      std::cout << "Resending messages for key : " << data << " \n";
      client->sendMessage(basePort + whichNode, data);
      // continue;
    }
    chunkData = dynamic_cast<Value*>(store->get(gotKey));
    store->remove(gotKey);
    // std::cout << "done ...\n";
  }
  return chunkData;
}

bool ChunkStore::waitForKey(Key* k) {
  size_t i = 0;
  while (!store->keyExists(k)) {
    i++;
    usleep(1000);
    if (i >= 500) {
      return 0;
    }
  }
  return 1;
}


// void WordCount::local_count() {
//   sleep(1);
//   std::cout << "before get \n";
//   DataFrame* words = (kv->get(in));
//   std::cout << "Printing words: \n";
//   words->print();
//
//   // DataFrame* words = (kv.waitAndGet(in)); // We need to local implementation
//   // p("Node ").p(nodeIndex).pln(": starting local count...");
//   // std::cout << "Node " << nodeIndex << ": starting local count...\n";
//   SIMap map;
//   //SIMap *map = new SIMap();
//   std::cout << "map size" << map.size_ << "\n";
//   Adder *add = new Adder(map);
//   std::cout << "words->map(*add); \n\n";
//   words->map(*add);
//   // words->local_map(add); // df doesn't know about networking so it is just working with local data
//   //delete words;
//   std::cout << "Printing map \n\n";
//   std::cout << map.items_->keys_->to_string() << "\n\n\n";
//   std::cout << "map size " << map.size_ << "\n";
//
//   sleep(3);
//   Summer* cnt = new Summer(map);
//   char* colType = new char[3];
//   strcat(colType, "SI");
//   Schema *s = new Schema();
//   DataFrame *df = new DataFrame(*s);
//   std::cout << "before from visit with summer \n";
//   std::cout << "In local_count doing fromVisitor with key " << mk_key(nodeIndex)->key << "\n";
//   DataFrame *retDf = df->fromVisitor(mk_key(nodeIndex), kv, colType, cnt);
//   std::cout << "local count df -------------- \n";
//   retDf->print();
//   std::cout << "local count df -------------- \n";
//   delete retDf;
// }
//
// void WordCount::run_() {
//   // std::cout << "DF Beginning run \n";
//   if (nodeIndex == 0) {
//     FileReader *fr = new FileReader(arg);
//     // std::cout << "DF inside run after file reading \n";
//     char* colType = new char[3];
//     strcat(colType, "S");
//     Schema *s = new Schema();
//     DataFrame *df = new DataFrame(*s);
//     // std::cout<<"DF creating fromVisitor\n";
//     DataFrame *retDf = df->fromVisitor(in, kv, colType, fr);
//     // std::cout<<"DF done with fromVisitor\n";
//     delete retDf;
//   }
//   local_count();
//   reduce();
// }
//
// void WordCount::merge(DataFrame* df, SIMap& m) {
//   Adder add(m);
//   df->map(add);
//   delete df;
// }

DataFrame* SOR::sorToDataframe() {
  Schema *s = new Schema();
  DataFrame *ret = new DataFrame(*s);
  for (size_t i = 0; i < len_; i++) {
    ret->add_column(cols_[i]);
  }
  return ret;
}
