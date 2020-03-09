#pragma once
/*************************************************************************
 * Schema::
 * A schema is a description of the contents of a data frame, the schema
 * knows the number of columns and number of rows, the type of each column,
 * optionally columns and rows can be named by strings.
 * The valid types are represented by the chars 'S', 'B', 'I' and 'F'.
 */
class Schema : public Object {
 public:
   size_t rowSize_; // number of columns in a row or the size of the row
   size_t colSize_; // number of rows in a column or the size of the column
   size_t colCap_;
   char* colType;

  /** Copying constructor */
  Schema(Schema& from) {
    rowSize_ = from.rowSize_;
    colSize_ = from.colSize_;
    colCap_ = from.colCap_;
    colType = from.colType;
  }

  /** Create an empty schema **/
  Schema() {
    rowSize_ = 0;
    colSize_ = 0;
    colCap_ = 2;
    colType = new char[0];
  }

  /** Create a schema from a string of types. A string that contains
    * characters other than those identifying the four type results in
    * undefined behavior. The argument is external, a nullptr argument is
    * undefined. **/
  Schema(const char* types) {
    rowSize_ = strlen(types);
    colSize_ = 0;
    colCap_ = 2;
    char* scmTypes = new char[strlen(types) + 1];
    strcpy(scmTypes, types);
    scmTypes[strlen(types) + 1] = '\0';
    colType = scmTypes;
    // std::cout << "schema constructor using char* " << colType << "\n";
  }

  /** Add a column of the given type and name (can be nullptr), name
    * is external. Names are expectd to be unique, duplicates result
    * in undefined behavior. */
  void add_column(char typ) {
    char *copyType = new char[rowSize_ + 1];
    for (int i = 0; i < rowSize_; i++) {
        copyType[i] = colType[i];
    }
    copyType[rowSize_] = typ;
		rowSize_++;
    delete[] colType;
    colType = copyType;
  }

  /** Add a row with a name (possibly nullptr), name is external.  Names are
   *  expectd to be unique, duplicates result in undefined behavior. */
  void add_row() {
    colSize_++;
  }
/*
  void growColNames() {
    String **copy = new String*[colCap_ * 2];
    for (int i = 0; i < colSize_; i++) {
      copy[i] = rowNames[i];
    }
    colCap_ = colCap_ * 2;
		delete[] rowNames;
		rowNames = copy;
  }*/

  /** Return name of row at idx; nullptr indicates no name. An idx >= width
    * is undefined. */
/*  String* row_name(size_t idx) {
    if (idx >= colSize_) {
      exit(1);
    } else {
      if (rowNames[idx] == nullptr) {
        return nullptr;
      }
      return rowNames[idx];
    }
  }*/

  /** Return name of column at idx; nullptr indicates no name given.
    *  An idx >= width is undefined.*/
/*  String* col_name(size_t idx) {
    if (idx >= rowSize_) {
      exit(1);
    } else {
      if (colNames[idx] == nullptr) {
        return nullptr;
      }
      return colNames[idx];
    }
  }*/

  /** Return type of column at idx. An idx >= width is undefined. */
  char col_type(size_t idx) {
    // // std::cout  << "\n" << colType[idx] << "<-- in scm calltype\n";
    if (idx >= rowSize_) {
      exit(1);
    } else {
      return colType[idx];
    }
  }

  /** Given a column name return its index, or -1. */
  /*int col_idx(const char* name) {
    if (name == nullptr) {
      return -1;
    }
    for (int i = 0; i < rowSize_; i++) {
      if (strcmp(colNames[i]->c_str(), name) == 0) {
        return i;
      }
    }
    return -1;
  }*/

  /** Given a row name return its index, or -1. */
/*  int row_idx(const char* name) {
    if (name == nullptr) {
      return -1;
    }
    for (int i = 0; i < colSize_; i++) {
      if (strcmp(rowNames[i]->c_str(), name) == 0) {
        return i;
      }
    }
    return -1;
  }*/

  /** The number of columns */
  size_t width() {
    return rowSize_;
  }

  /** The number of rows */
  size_t length() {
    return colSize_;
  }
};
