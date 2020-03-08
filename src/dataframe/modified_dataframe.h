#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include "object.h"
#include "string.h"
#include "column.h"
#include "intColumn.h"
#include "stringColumn.h"
#include "floatColumn.h"
#include "boolColumn.h"
#include "schema.h"
#include "fielder.h"
#include "fielderPrint.h"
#include "row.h"
#include "rower.h"
#include "printRower.h"
#include "add2IntRower.h"
#include <thread>

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
   // Row** rows;
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
  void add_column(Column* col, String* name) {
    // // std::cout << "ADD COL CALLED\n";
    // // std::cout << "add col col type : " << col->get_type() << "\n";
    scm->add_column(col->get_type(), name); // scm width now 5

    Column** temp = new Column*[scm->width() + 1];
    for (int i = 0; i < scm->width() - 1; i++) {
      temp[i] = column[i];
    }
    // // std::cout << "after copying pointers: \n";
    // // std::cout << "values of first COL : " << temp[0]->as_int()->get(0) << "<--- THIS IS COL O O values\n";
    temp[scm->width() - 1] = col;
    // // std::cout << "this is the passed in COL : " << col << "\n";

    delete[] column;
    column = temp;
    // // std::cout << "values of added COL : " << column[scm->width() - 1]->as_float()->get(4) << "<--- THIS IS ADDED COL Oth value\n";

    // // std::cout << "updated cols : " << scm->width() << "\n";
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

  /** Return the offset of the given column name or -1 if no such col. */
  int get_col(String& col) {
    for (int i = 0; i < scm->width(); i++) {
      if (scm->col_name(i) != nullptr) {
        if (col.equals(scm->col_name(i))) {
          return i;
        }

      }
    }
    return -1;
  }

  /** Return the offset of the given row name or -1 if no such row. */
  int get_row(String& col) {
    for (int i = 0; i < scm->length(); i++) {
      if (scm->row_name(i) != nullptr) {
        if (col.equals(scm->row_name(i))) {
          return i;
        }
      }
    }
    return -1;
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
    for (int i = 0; i < scm->width(); i++) {
      //// // std::cout<< "scm width for fillrow in df-->" << scm->width() << "\n";
      //// // std::cout<< "scm width then idx then i for fillrow in df-->" << idx << i << "\n";
      //// // std::cout<< "scm row.col_type(i) for fillrow in df-->" << row.col_type(i) << "\n";
      if (row.col_type(i) == 'I') {
        row.set(i, column[i]->as_int()->get(idx));
      } else if (row.col_type(i) == 'B') {
        // this->printBoolCol(column[i]->as_bool());
        // // // std::cout<< "Testing bool specific->" << column[idx]->as_bool()->get(idx) << "\n";
        row.set(i, column[i]->as_bool()->get(idx));
      } else if (row.col_type(i) == 'F') {
        row.set(i, column[i]->as_float()->get(idx));
      } else if (row.col_type(i) == 'S') {
        row.set(i, column[i]->as_string()->get(idx));
      }
      row.set_idx(idx);
    }
  }

  /** Add a row at the end of this dataframe. The row is expected to have
   *  the right schema and be filled with values, otherwise undedined.  */
  void add_row(Row& row) {
    scm->add_row(nullptr);
    // // std::cout<<row.width()<<"<-row width in addrow in df file\n";
    // // std::cout<<scm->width()<<"<-scm width in addrow in df file\n";
    if (row.width() != scm->width()) {
      exit(1);
    }
    for (int i = 0; i < scm->width(); i++) {
      // // std::cout<< "start add row loop \n";
      // // std::cout<< "printing col type for i=" << i << ": " <<row.col_type(i) << "\n";
      if (row.col_type(i) == 'I') {
        // // std::cout << " BEFORE pushing int onto column i= " << row.get_int(i) << "\n";
        // // std::cout << "not a getint issue" << "\n";
        column[i]->push_back(row.get_int(i));
        // // std::cout << "pushing int onto column i= " << i << "\n";
      } else if (row.col_type(i) == 'B') {
        // // std::cout << " BEFORE pushing bool onto column i= " << row.get_bool(i) << "\n";
        column[i]->push_back(row.get_bool(i));
        // // std::cout << "pushing bool onto column i= " << i << "\n";
      } else if (row.col_type(i) == 'F') {
        column[i]->push_back(row.get_float(i));
      } else if (row.col_type(i) == 'S') {
        column[i]->push_back(row.get_string(i));
      }
      //// // std::cout<< "end add row loop \n";
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
    // // std::cout  << "starting map \n";
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      // // std::cout  << "copying scm at i: "<< i <<  " \n";
      // // std::cout  << "col type i: "<< scm->col_type(i) <<  " \n";
      // // std::cout  << "col name i: "<< scm->col_name(i) <<  " \n";

      s1->add_column(scm->col_type(i), scm->col_name(i));
    }
    // // std::cout  << "copied schema \n";
    Row *currRow = new Row(*s1);
    DataFrame *retDF = new DataFrame(*s1);
    // // std::cout  << "created df \n";
    for (int j = 0; j < scm->length(); j++) {
      this->fill_row(j, *currRow);
      r.accept(*currRow);
      retDF->add_row(*currRow);
    }
    // // std::cout  << "mapped df \n";
    delete scm;
    delete[] column;
    scm = retDF->scm;
    column = retDF->column;
    // // std::cout  << "finished map df \n";
  }

  /** Visit subset of rows in order */
  void map_(Rower& r, size_t start, size_t end) {
    // std::cout  << "In map\n";
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      s1->add_column(scm->col_type(i), scm->col_name(i));
    }

    Row *currRow = new Row(*s1);
    //DataFrame *retDF = new DataFrame(*s1);
    for (int j = start; j < end; j++) {
      this->fill_row(j, *currRow);
      // std::cout  << "copying currRow at j: "<< j <<  " \n";
      r.accept(*currRow);
      // std::cout  << "currRow->get_idx(): "<< currRow->get_idx() <<  " \n";
      // std::cout  << "scm->width(): "<< scm->width() <<  " \n";
      for (int k = 0; k < scm->width(); k++) {
        // std::cout  << "copying currRow at k: "<< k <<  " \n";
        // std::cout  << "currRow->col_type(k): "<< currRow->col_type(k) <<  " \n";
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
    Rower* r_clone = new MyAddRower();
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


    std::cout  << "schema len after pmap: "<< scm->length() <<  " \n";
  }

  /** Create a new dataframe, constructed from rows for which the given Rower
    * returned true from its accept method. */
  DataFrame* filter(Rower& r) {
    // // std::cout  << "starting filter \n";
    Schema* s1 = new Schema();
    for (int i = 0; i < scm->width(); i++) {
      // // std::cout  << "copying scm at i: "<< i <<  " \n";
      // // std::cout  << "col type i: "<< scm->col_type(i) <<  " \n";
      // // std::cout  << "col name i: "<< scm->col_name(i) <<  " \n";
      s1->add_column(scm->col_type(i), scm->col_name(i));
    }
    // // std::cout  << "copied schema \n";
    Row *currRow = new Row(*s1);
    DataFrame *retDF = new DataFrame(*s1);
    // // std::cout  << "created df \n";
    for (int j = 0; j < scm->length(); j++) {
      this->fill_row(j, *currRow);
      if (r.accept(*currRow)) {
        retDF->add_row(*currRow);
      }
    }
    // // std::cout  << "filtered df \n";
    return retDF;
  }

  /** Print the dataframe in SoR format to standard output. */
  void print() {
    Schema s1 = *scm;
    Row *r = new Row(s1);
    for (int cName = 0; cName < scm->width(); cName++) {
      p("   ");
      if (scm->col_name(cName) == nullptr) {
        p("Index:");p(cName);
      } else {
        p(scm->col_name(cName)->c_str());
      }
    }
    pln();
    for (int i = 0; i < scm->length(); i++) {
      this->fill_row(i, *r);
      PrintRower* pr = new PrintRower();
      pr->accept(*r);
    }
  }
};
