#pragma once
/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 */
class Row : public Object {
 public:
   Schema* schema;
   size_t idx;
   // THE ARRAY OF COLUMNS: is an array of columns of height 1!
   // this is to allow us to keep typed values in our row.
   Column** cols;


   /** Build a row following a schema. */
   Row(Schema& scm) {
     schema = &scm;
     cols = new Column*[scm.rowSize_];
   }

  /** Setters: set the given column with the given value. Setting a column with
    * a value of the wrong type is undefined. */
  void set(size_t col, int val) {
    if (schema->col_type(col) != 'I') {
      exit(1);
    }
    cols[col] = new IntColumn(1, val);
  }

  void set(size_t col, float val) {
    if (schema->col_type(col) != 'F') {
      exit(1);
    }
    cols[col] = new FloatColumn(1, val);
  }

  void set(size_t col, bool val) {
    if (schema->col_type(col) != 'B') {
      exit(1);
    }
    cols[col] = new BoolColumn(1, val);
  }

  /** The string is external. */
  void set(size_t col, String* val) {
    if (schema->col_type(col) != 'S') {
      exit(1);
    }
    cols[col] = new StringColumn(1, val);
  }

  /** Set/get the index of this row (ie. its position in the dataframe. This is
   *  only used for informational purposes, unused otherwise */
  void set_idx(size_t idx) {
    this->idx = idx;
  }
  size_t get_idx() {
    return idx;
  }

  /** Getters: get the value at the given column. If the column is not
    * of the requested type, the result is undefined. */
  int get_int(size_t col) {
    if (schema->col_type(col) != 'I') {
      exit(1);
    }
    return cols[col]->as_int()->get(0);
  }

  bool get_bool(size_t col) {
    if (schema->col_type(col) != 'B') {
      exit(1);
    }
    return cols[col]->as_bool()->get(0);
  }

  float get_float(size_t col) {
    if (schema->col_type(col) != 'F') {
      exit(1);
    }
    return cols[col]->as_float()->get(0);
  }

  String* get_string(size_t col) {
    if (schema->col_type(col) != 'S') {
      exit(1);
    }
    return cols[col]->as_string()->get(0);
  }

  void printRow() {
    for (int i = 0; i < schema->width(); i++) {
      if (schema->col_type(i) == 'I') {
        std:: cout << "int val in row: " << cols[i]->as_int()->get(i) << "\n";
      } else if (schema->col_type(i) == 'B') {
        std:: cout << "bool val in row: " << cols[i]->as_bool()->get(i) << "\n";
      } else if (schema->col_type(i) == 'F') {
        std:: cout << "float val in row: " << cols[i]->as_float()->get(i) << "\n";
      } else if (schema->col_type(i) == 'S') {
        std:: cout << "String val in row: " << cols[i]->as_string()->get(i)->c_str() << "\n";
      }
    }
  }

  /** Number of fields in the row. */
  size_t width() {
    return schema->rowSize_;
  }

   /** Type of the field at the given position. An idx >= width is  undefined. */
  char col_type(size_t idx) {
    if ((idx >= schema->rowSize_) || (idx < 0)) { // TODO: DO THIS FOR OTHER EXIT STATMENTS... ADD idx<0
      exit(1);
    }
    return schema->col_type(idx);
  }

  /** Given a Fielder, visit every field of this row. The first argument is
    * index of the row in the dataframe.
    * Calling this method before the row's fields have been set is undefined. */
  void visit(size_t idx, Fielder& f) {

  }

};
