#pragma once
#include "object.h"
#include "string.h"
#include "helpers.h"
#include <stdarg.h>


class IntColumn;
class BoolColumn;
class FloatColumn;
class StringColumn;

enum ColumnType {
    type_unknown = -1,
    type_bool = 0,
    type_int = 1,
    type_float = 2,
    type_string = 3,
};

// returns the inferred typing of the char*
ColumnType infer_type(char *c) {
    // missing values
    if (c == nullptr) {
        return type_bool;
    }
    // check boolean
    if (strlen(c) == 1) {
        if ((*c == '0') || (*c == '1')) {
            return type_bool;
        }
    }
    // check int
    if (is_int(c)) {
        return type_int;
    }
    // check float
    if (is_float(c)) {
        return type_float;
    }
    return type_string;
}


/* Column ::
 * Represents one column of a data frame which holds values of a single type.
 * This abstract class defines methods overriden in subclasses. There is
 * one subclass per element type. Columns are mutable, equality is pointer
 * equality. */
class Column : public Object {
 public:

   Column() {}

   ~Column() {}


  /** Type converters: Return same column under its actual type, or
   *  nullptr if of the wrong type.*/
   virtual IntColumn* as_int() {return nullptr;}
   virtual BoolColumn*  as_bool() {return nullptr;}
   virtual FloatColumn* as_float() {return nullptr;}
   virtual StringColumn* as_string() {return nullptr;}

  /** Type appropriate push_back methods. Calling the wrong method is
    * undefined behavior. **/
  virtual void push_back(int val) {}
  virtual void push_back(bool val) {}
  virtual void push_back(float val) {}
  virtual void push_back(String* val) {}

 /** Returns the number of elements in the column. */
  virtual size_t size() {return 0;}

  /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. **/
  virtual char get_type() { return 'N';}
  // get string rep of element at ith index
  virtual char* get_char(size_t i) {return nullptr;}
  virtual bool can_add(char* c) {return 0;}
  virtual char get_enum_type() { return -1;}
};


/*************************************************************************
 * BoolColumn::
 * Holds bool values.
 */
class BoolColumn : public Column {
 public:
   size_t sizeOfChunk = 50; // number of elements per chunk
   bool** elements; // An array that holds arrays of chunks
   size_t numOfChunks_;
	 size_t count_; // number of elements

  BoolColumn() {
    numOfChunks_ = 1;
    count_ = 0;
    elements = new bool*[numOfChunks_];
    elements[0] = new bool[sizeOfChunk];
  }

  BoolColumn(int n, ...) {
    if (n == 0) {
      numOfChunks_ = 1;
      count_ = 0;
      elements = new bool*[numOfChunks_];
      elements[0] = new bool[sizeOfChunk];
    } else {
      int val;
      va_list vl;
      va_start(vl, n);
      if (n % sizeOfChunk > 0) {
        numOfChunks_ = (n / sizeOfChunk) + 1;
      } else {
        numOfChunks_ = n / sizeOfChunk;
      }
      count_ = n;
      elements = new bool*[numOfChunks_];
      for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
        elements[chunk] = new bool[sizeOfChunk];
        for (int i = 0; (i < sizeOfChunk) && ((chunk * sizeOfChunk + i) < n); i++) {
          val = va_arg(vl, int);
          bool f = static_cast<bool>(val);
          elements[chunk][i] = f;
          // std::cout << elements[chunk][i] << "\n";
        }
      }
      va_end(vl);
    }

  }

  void grow_() {
    bool** nElements = new bool*[numOfChunks_*2];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk];
    }
    for (size_t chunk2 = numOfChunks_; chunk2 < (numOfChunks_ * 2); chunk2++) {
      nElements[chunk2] = new bool[sizeOfChunk];
    }
    numOfChunks_ = numOfChunks_*2;
    delete[] elements;
    elements = nElements;
  }

  void push_back(int val) {
    exit(1);
  }

  void push_back(bool val) {
    if ((count_ / sizeOfChunk) == numOfChunks_) {
      grow_();
    }
    size_t chunkLoc = count_ / sizeOfChunk;
    size_t idxInChunk = count_ % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
    count_++;
  }

  void push_back(float val) {
    exit(1);
  }

  void push_back(String* val) {
    exit(1);
  }

  char get_type() {
    return 'B';
  }

  char get_enum_type() {
    return type_bool;
  }

  bool get(size_t idx) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    //std::cout << "get ChunkLoc: " << chunkLoc << " cIDX: " << idxInChunk << "\n";
    return elements[chunkLoc][idxInChunk];
  }

  BoolColumn* as_bool() {
    return this;
  }

  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, bool val) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
  }

  size_t size() {
    return count_;
  }

  // checks if the value represented by the char* can be added to this column
  virtual bool can_add(char *c) {
    if (c == nullptr || *c == '\0') {
        return true;
    }
    return infer_type(c) == 0; // enum type
  }

  // get string rep of element at ith index
  char* get_char(size_t i) {
      char* ret = new char[512];
      bool values_ = get(i);
      sprintf(ret, "%f", values_);
      return ret;
  }
};


/*************************************************************************
 * FloatColumn::
 * Holds float values.
 */
class FloatColumn : public Column {
 public:
   size_t sizeOfChunk = 50; // number of elements per chunk
   float** elements; // An array that holds arrays of chunks
   size_t numOfChunks_; // number of chunks.
	 size_t count_; // number of elements

  FloatColumn() {
    numOfChunks_ = 1;
    count_ = 0;
    elements = new float*[numOfChunks_];
    elements[0] = new float[sizeOfChunk];
  }

  FloatColumn(int n, ...) {
    if (n == 0) {
      numOfChunks_ = 1;
      count_ = 0;
      elements = new float*[numOfChunks_];
      elements[0] = new float[sizeOfChunk];
    } else {
      double val;
      va_list vl;
      va_start(vl, n);
      if (n % sizeOfChunk > 0) {
        numOfChunks_ = (n / sizeOfChunk) + 1;
      } else {
        numOfChunks_ = n / sizeOfChunk;
      }
      count_ = n;
      elements = new float*[numOfChunks_];
      for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
        elements[chunk] = new float[sizeOfChunk];
        for (int i = 0; (i < sizeOfChunk) && ((chunk * sizeOfChunk + i) < n); i++) {
          val = va_arg(vl, double);
          float f = static_cast<float>(val);

          elements[chunk][i] = f;
          // std::cout << elements[chunk][i] << "\n";
        }
      }
      va_end(vl);
    }
  }

  void push_back(int val) {
    exit(1);
  }

  void push_back(bool val) {
    exit(1);
  }

  void push_back(float val) {
    if ((count_ / sizeOfChunk) == numOfChunks_) {
      grow_();
    }
    size_t chunkLoc = count_ / sizeOfChunk;
    size_t idxInChunk = count_ % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
    count_++;
  }

  void push_back(String* val) {
    exit(1);
  }

  void grow_() {
    float** nElements = new float*[numOfChunks_*2];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    for (size_t chunk2 = numOfChunks_; chunk2 < (numOfChunks_ * 2); chunk2++) {
      nElements[chunk2] = new float[sizeOfChunk];
    }
    numOfChunks_ = numOfChunks_*2;
    delete[] elements;
    elements = nElements;
  }

  float get(size_t idx) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    //std::cout << "get ChunkLoc: " << chunkLoc << " cIDX: " << idxInChunk << "\n";
    return elements[chunkLoc][idxInChunk];
  }

  FloatColumn* as_float() {
    return this;
  }

  char get_type() {
    return 'F';
  }

  char get_enum_type() {
    return type_float;
  }

  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, float val) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
  }

  size_t size() {
    return count_;
  }

  // checks if the value represented by the char* can be added to this column
  virtual bool can_add(char *c) {
    if (c == nullptr || *c == '\0') {
        return true;
    }
    return infer_type(c) == 2; // enum type
  }

  // get string rep of element at ith index
  char* get_char(size_t i) {
      char* ret = new char[512];
      float values_ = get(i);
      sprintf(ret, "%f", values_);
      return ret;
  }
};


/*************************************************************************
 * IntColumn::
 * Holds int values.
 */
class IntColumn : public Column {
 public:
   size_t sizeOfChunk = 50; // number of elements per chunk
   int** elements; // An array that holds arrays of chunks
   size_t numOfChunks_; // number of chunks.
   size_t count_; // number of elements

  IntColumn() {
    numOfChunks_ = 1;
    count_ = 0;
    elements = new int*[numOfChunks_];
    elements[0] = new int[sizeOfChunk];
  }

  IntColumn(int n, ...) {
    if (n == 0) {
      numOfChunks_ = 1;
      count_ = 0;
      elements = new int*[numOfChunks_];
      elements[0] = new int[sizeOfChunk];
    }
    else {
      int val;
      va_list vl;
      va_start(vl, n);
      if (n % sizeOfChunk > 0) {
        numOfChunks_ = (n / sizeOfChunk) + 1;
      } else {
        numOfChunks_ = n / sizeOfChunk;
      }
      count_ = n;
      elements = new int*[numOfChunks_];
      for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
        elements[chunk] = new int[sizeOfChunk];
        for (int i = 0; (i < sizeOfChunk) && ((chunk * sizeOfChunk + i) < n); i++) {
          val = va_arg(vl, int);
          int f = static_cast<int>(val);
          elements[chunk][i] = f;
          // std::cout << elements[chunk][i] << "\n";
        }
      }
      va_end(vl);
    }
  }

  ~IntColumn() {
    for (int i = 0; i < numOfChunks_; i++) {
      delete[] elements[i];
    }
    delete[] elements;
  }

  int get(size_t idx) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    return elements[chunkLoc][idxInChunk];
  }

  void push_back(int val) {
    if ((count_ / sizeOfChunk) == numOfChunks_) {
      grow_();
    }
    size_t chunkLoc = count_ / sizeOfChunk;
    size_t idxInChunk = count_ % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
    count_++;
  }

  void push_back(bool val) {
    exit(1);
  }

  void push_back(float val) {
    exit(1);
  }

  void push_back(String* val) {
    exit(1);
  }
/*
  void grow_() {
    int** nElements = new int*[numOfChunks_+ 1];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    nElements[numOfChunks_] = new int[sizeOfChunk];
    numOfChunks_++;
    delete[] elements;
    elements = nElements;
  }*/


  void grow_() {
    int** nElements = new int*[numOfChunks_*2];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    for (size_t chunk2 = numOfChunks_; chunk2 < (numOfChunks_ * 2); chunk2++) {
      nElements[chunk2] = new int[sizeOfChunk];
    }
    numOfChunks_ = numOfChunks_*2;
    delete[] elements;
    elements = nElements;
  }


  IntColumn* as_int() {
    return this;
  }

  char get_type() {
    return 'I';
  }

  char get_enum_type() {
    return type_int;
  }

  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, int val) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
  }

  size_t size() {
    return count_;
  }

  // checks if the value represented by the char* can be added to this column
  virtual bool can_add(char *c) {
    if (c == nullptr || *c == '\0') {
        return true;
    }
    return infer_type(c) == 1; // enum type
  }

  // get string rep of element at ith index
  char* get_char(size_t i) {
      char* ret = new char[512];
      int values_ = get(i);
      sprintf(ret, "%f", values_);
      return ret;
  }
};


/*************************************************************************
 * StringColumn::
 * Holds String values.
 */
class StringColumn : public Column {
 public:
   size_t sizeOfChunk = 50; // number of elements per chunk
   String*** elements; // An array that holds arrays of chunks
   size_t numOfChunks_; // number of chunks.
   size_t count_; // number of elements

  StringColumn() {
    numOfChunks_ = 1;
    count_ = 0;
    elements = new String**[numOfChunks_];
    elements[0] = new String*[sizeOfChunk];
  }

  StringColumn(int n, ...) {
    if (n == 0) {
      numOfChunks_ = 1;
      count_ = 0;
      elements = new String**[numOfChunks_];
      elements[0] = new String*[sizeOfChunk];
    } else {
      String* val;
      va_list vl;
      va_start(vl, n);
      if (n % sizeOfChunk > 0) {
        numOfChunks_ = (n / sizeOfChunk) + 1;
      } else {
        numOfChunks_ = n / sizeOfChunk;
      }
      count_ = n;
      elements = new String**[numOfChunks_];
      for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
        elements[chunk] = new String*[sizeOfChunk];
        for (int i = 0; (i < sizeOfChunk) && ((chunk * sizeOfChunk + i) < n); i++) {
          val = va_arg(vl, String*);
          String *f = val;

          elements[chunk][i] = f;
        }
      }
      va_end(vl);
    }
  }

  ~StringColumn() {}

  void grow_() {
    String*** nElements = new String**[numOfChunks_*2];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    for (size_t chunk2 = numOfChunks_; chunk2 < (numOfChunks_ * 2); chunk2++) {
      nElements[chunk2] = new String*[sizeOfChunk];
    }
    numOfChunks_ = numOfChunks_*2;
    delete[] elements;
    elements = nElements;
  }

  void push_back(int val) {
    exit(1);
  }

  void push_back(bool val) {
    exit(1);
  }

  void push_back(float val) {
    exit(1);
  }

  void push_back(String* val) {
    if ((count_ / sizeOfChunk) == numOfChunks_) {
      grow_();
    }
    size_t chunkLoc = count_ / sizeOfChunk;
    size_t idxInChunk = count_ % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
    count_++;
  }

  String* get(size_t idx) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    // std::cout << "get count: " << count_ << "\n";
    // std::cout << "get ChunkLoc: " << chunkLoc << " cIDX: " << idxInChunk << "\n";
    // std::cout << "actual 0 0: " << elements[0][0]->c_str() <<"\n";
    elements[0][0]->c_str();
    return elements[chunkLoc][idxInChunk];
  }

  StringColumn* as_string() {
    return this;
  }

  char get_type() {
    return 'S';
  }

  char get_enum_type() {
    return type_string;
  }

  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, String *val) {
    if (idx >= count_) {
      exit(1);
    }
    size_t chunkLoc = idx / sizeOfChunk;
    size_t idxInChunk = idx % sizeOfChunk;
    elements[chunkLoc][idxInChunk] = val;
  }

  virtual size_t size() {
    return count_;
  }

  // checks if the value represented by the char* can be added to this column
  virtual bool can_add(char *c) {
    if (c == nullptr || *c == '\0') {
        return true;
    }
    return infer_type(c) == 2; // enum type
  }
  // get string rep of element at ith index
  char* get_char(size_t i) {
      String* values_ = get(i);
      return values_->c_str();
  }
};