//lang:CwC
#pragma once
#include <unistd.h>
// #include "helpers.h"
// #include "../dataframe/modified_dataframe.h"

// The maximum length of a line buffer. No lines over 4095 bytes
// static const int buff_len = 4096;
static const int buff_len = 10000;

// Reads a file and determines the schema on read
class SOR : public Object {
    public:
        Column** cols_;  // owned
        size_t len_;
        size_t cap_;

        SOR() {
            len_ = 0;
            cap_ = 16;
            cols_ = new Column*[cap_];

            for (size_t i = 0; i < cap_; i++) {
                cols_[i] = nullptr;
            }
        }

        ~SOR() {
            for (size_t i = 0; i < len_; i++) {
                if (cols_[i] != nullptr) {
                    delete cols_[i];
                }
            }
            delete[] cols_;
        }

        // What is the type of the column at the given index? i
        // if the index is too big a -1 is returned
        ColumnType get_col_type(size_t index) {
            if (index >= len_) {
                return type_unknown;
            }

            if (cols_[index]->get_type() == 'F') {
              return type_float;
            } else if (cols_[index]->get_type() == 'I') {
              return type_int;
            } else if (cols_[index]->get_type() == 'B') {
              return type_bool;
            } else if (cols_[index]->get_type() == 'S') {
              return type_string;
            }
        }

        // What is the value for the given column index and row index?
        // If the coluumn or row index are too large a nullptr is returned
        char* get_value(size_t col_index, size_t row_index) {
            // printf("get value start \n");
            if (col_index >= len_) {
                // printf("get value ret null \n");
                return nullptr;
            }
            return cols_[col_index]->get_char(row_index);
        }

        // Is there a value at the given column and row index.
        // If the indexes are too large, true is returned.
        bool is_missing(size_t col_index, size_t row_index) {
            return get_value(col_index, row_index) == nullptr;
        }

        // Reads in the data from the file starting at the from byte
        // and reading at most len bytes
        void read(FILE* f, size_t from, size_t len) {
            infer_columns_(f, from, len);
            parse_(f, from, len);
        }

        // reallocates columns array if more space is needed.
        void check_reallocate_() {
            if (len_ >= cap_) {
                cap_*=2;
                Column **temp = new Column*[cap_];
                for (size_t i = 0; i < len_; i++) {
                    temp[i] = cols_[i];
                }
                delete[] cols_;
                cols_ = temp;
            }
        }

        // moves the file pointer to the start of the next line.
        void seek_(FILE* f, size_t from) {
            if (from == 0) {
                fseek(f, from, SEEK_SET);
            } else {
                char buf[buff_len];
                fseek(f, from - 1, SEEK_SET);
                fgets(buf, buff_len, f);
            }
        }

        // infers and creates the column objects
        void infer_columns_(FILE* f, size_t from, size_t len) {
            seek_(f, from);
            char buf[buff_len];

            size_t total_bytes = 0;
            size_t row_count = 0;
            while (fgets(buf, buff_len, f) != nullptr && row_count < 500) {
                row_count++;
                total_bytes += strlen(buf);
                if (total_bytes >= len) {
                    break;
                }
                size_t num_fields;
                char** row = parse_row_(buf, &num_fields);

                for (size_t i = 0; i < num_fields; i++) {
                    check_reallocate_();
                    if (i >= len_) {
                        cols_[i] = new BoolColumn();
                        len_++;
                    }
                    ColumnType inferred_type = infer_type(row[i]);
                    if (inferred_type > cols_[i]->get_enum_type()) {
                        delete cols_[i];
                        switch(inferred_type) {
                            case type_int:
                                cols_[i] = new IntColumn();
                                break;
                            case type_bool:
                                cols_[i] = new BoolColumn();
                                break;
                            case type_float:
                                cols_[i] = new FloatColumn();
                                break;
                            default:
                                cols_[i] = new StringColumn();
                                break;
                        }
                    }
                }
                delete[] row;

            }
            // std::cout << "end infer \n";
        }

        // Find the start of the field value and null terminate it.
        // ASSUMPTION: input field is terminated by '>' char
        // NOTE: will mutate the field value
        // The value of len will be the offset of the null byte
        char* parse_field_(char* field, int* len) {
            char* ret = field;
            int j = 0;
            for (size_t i = 0; field[i] != '>'; i++) {
                switch (field[i]) {
                    case '<':  // Malformed input
                        affirm(false, "Multiple opening <");
                    case ' ': // extra space in front of field
                        ret++;
                        break;
                    case '"': // the start of a String
                        ret++;
                        j = i + 1;
                        while (field[j] != '"') { // add every character until a end quote is met.
                            j++;
                        }
                        field[j] = '\0';
                        *len = j;
                        return ret;
                    default:  // add every ASCII character to field return value.
                        for (j = i; field[j] != '>' && field[j] != ' '; j++);
                        field[j] = '\0';
                        *len = j;
                        return ret;
                }
            }
            *len = 0;
            return nullptr;  // missing value
        }

        // parses a row and returns a list of field values as char*
        // NOTE: will mutate the row value.
        // The value of len will be the number of fields returned
        char** parse_row_(char* row, size_t *len) {
            int cap = 16;
            int l = 0;
            char** output = new char*[cap];

            for (size_t i = 0; row[i] != '\0'; i++) {
                if (row[i] == '<') {
                    int to_increment = 0;
                    if (l >= cap) {
                        cap*=2;
                        char** temp = new char*[cap];
                        for (int k = 0; k < l; k++) {
                            temp[k] = output[k];
                        }
                        delete[] output;
                        output = temp;
                    }

                    output[l++] = parse_field_(&row[i + 1], &to_increment);
                    i = i + to_increment + 1;
                }
            }

            *len = l;
            return output;
        }


        // read the rows from the starting byte up to len bytes into Columns.
        void parse_(FILE* f, size_t from, size_t len) {
            seek_(f, from);
            char buf[buff_len];

            size_t total_bytes = 0;
            while (fgets(buf, buff_len, f) != nullptr) {
                total_bytes += strlen(buf);
                // std::cout << "total bytes read " << total_bytes << " with len " << len << " with bufflen " << buff_len << "\n";
                if (total_bytes >= len) {
                    std::cout << "Breaking now\n";
                    break;
                }

                size_t num_fields;
                // current row could have more columns than infered - parse the frist len_ columns
                // std::cout << "parsing row\n";
                char** row = parse_row_(buf, &num_fields);
                // std::cout << "row 0 is " << row[0] << "\n";
                // std::cout << "row 1 is " << row[1] << "\n";
                // std::cout << "parsing row ended\n";
                // skipping rows with too few fields
                if (num_fields == 0) {
                  // std::cout << "num_fields == 0\n";
                    delete[] row;
                    continue;
                }

                // we skip the row as soon as we find a field that does not match our schema
                bool skip = false;
                for (size_t i = 0; i < len_; i++) {
                    if (!cols_[i]->can_add(row[i])) {
                      // std::cout << "Skipping a /line... TODO FIX OFF BY ONE \n";
                        skip = true;
                        break;
                    }
                }
                if (skip) {
                  // std::cout << "Skipping..\n";
                    delete[] row;
                    continue;
                }

                // add all fields in this row to columns
                // std::cout << "BEGINNING ROW PARSE_______________ \n";
                for (size_t i = 0; i < len_; i++) {
                  // std::cout << "HEREEEEEEEEEEEEEEEEEEEEEEEE\n";
                  // std::cout << "iteration " << i << " with len " << len_ << "\n";
                  // std::cout << "row 1 is " << row[1] << "\n";
                    if (i >= num_fields || row[i] == nullptr) {
                      // std::cout << "cols_[i] = nullptr..\n";
                        cols_[i] = nullptr;
                    } else {
                      // std::cout << "else branch.\n";
                        char type = cols_[i]->get_type();
                        // std::cout << "after get_type.\n";
                        char* pEnd = new char[512];
                        // std::cout << "pEnd = new char[512].\n";
                        if (type == 'B') {
                          // std::cout << "push back B.\n";/
                          bool val = strtol(row[i], &pEnd, 10);
                          cols_[i]->push_back(val);
                        } else if (type == 'I') {
                          // std::cout << "push back I.\n";
                          int val = strtol(row[i], &pEnd, 10);
                          cols_[i]->push_back(val);
                        } else if (type == 'F') {
                          // std::cout << "push back F.\n";
                          float val = strtod(row[i], &pEnd);
                          cols_[i]->push_back(val);
                        } else {
                          // std::cout << "push back S.\n";
                          String *wrap = new String(row[i]);
                          // std::cout << "Found String: " << wrap->c_str() << " in row " << i << "\n";
                          cols_[i]->push_back(wrap);
                        }
                        // std::cout << "before memset row 1 is " << row[1] << "\n";
                        // memset(pEnd, 0, 512);
                        // std::cout << "after memset row 1 is " << row[1] << "\n";
                    }
                }
                // std::cout << "Total bytes read: "<< total_bytes <<"\n";
                delete[] row;
            }
            // std::cout << "printing col 1 ...\n";
            // cols_[0]->printCol();
            // std::cout << "printing col 2 ...\n";
            // cols_[1]->printCol();
            // std::cout << "Finished with parse 2 ...\n";
        }

        // Assumes
        // Converts the columns into a dataframe and returns the dataframe
        DataFrame* sorToDataframe();
};
