#pragma once
// LANGUAGE: CwC
#include <cstring>
#include <string>
#include <cassert>
#include "object.h"

/** An immutable string class that wraps a character array.
 * The character array is zero terminated. The size() of the
 * String does count the terminator character. Most operations
 * work by copy, but there are exceptions (this is mostly to support
 * large strings and avoid them being copied).
 *  author: vitekj@me.com */
 class String: public Object {
   public:
       char *str_;
       size_t size_;

       String(const char* str) {
           size_ = strlen(str);

           str_ = new char[size_ + 1];
           strcpy(str_, str);
       }

       ~String() {
           delete[] str_;
       }

       size_t length() {
           return size_;
       }

       bool equals(Object *other) {
           if (this == other) {
               return true;
           }

           String *other1 = dynamic_cast<String *>(other);
           if (other1 == nullptr) {
               return false;
           }
           return !strcmp(this->str_, other1->str_); // strcmp returns 0 if equal
       }

       // returns a new concatenated string: this + o
       virtual String* concat(String *o) {
           char *t = new char[size_ + o->length()];
           strcpy(t, str_);
           strcat(t, o->str_);
           String *out = new String(t);
           delete[] t;
           return out;
       }

       virtual void print_info() {
           printf("string: %s, size: %lu\n", str_, size_);
       }

       virtual void print() {
           printf("%s", str_);
       }

       /** Returns 0 if strings are equal, >0 if this string is larger,
        *  <0 otherwise */
       // From Professor Vitek
       virtual int compare(String* tgt) {
           return strcmp(str_, tgt->str_);
       }

       virtual char* get() {
           return str_;
       }

       virtual char* c_str() {
           return str_;
       }

       // From Professor Vitek
       size_t hash_me_() {
           size_t hash = 0;
           for (size_t i = 0; i < size_; ++i) {
               hash = str_[i] + (hash << 6) + (hash << 16) - hash;
           }
           return hash;
       }

       virtual Object* clone() {
         String *newStr = new String(str_);
         return newStr;
       }

       /** This consumes cstr_, the String must be deleted next */
      char * steal() {
          char *res = str_;
          str_ = nullptr;
          return res;
      }
 };

/** A string buffer builds a string from various pieces.
 *  author: jv */
class StrBuff : public Object {
public:
  char *val_; // owned; consumed by get()
  size_t capacity_;
  size_t size_;

  StrBuff() {
      val_ = new char[capacity_ = 10];
      size_ = 0;
  }

  StrBuff(Key* key) {
    val_ = key->key;
    size_ = strlen(key->key);
  }

  void grow_by_(size_t step) {
      if (step + size_ < capacity_) return;
      capacity_ *= 2;
      if (step + size_ >= capacity_) capacity_ += step;
      char* oldV = val_;
      val_ = new char[capacity_];
      memcpy(val_, oldV, size_);
      delete[] oldV;
  }
  StrBuff& c(const char* str) {
      size_t step = strlen(str);
      grow_by_(step);
      memcpy(val_+size_, str, step);
      size_ += step;
      return *this;
  }
  StrBuff& c(String &s) { return c(s.c_str());  }
  StrBuff& c(size_t v) { return c(std::to_string(v).c_str());  } // Cpp

  String* get() {
      assert(val_ != nullptr); // can be called only once
      grow_by_(1);     // ensure space for terminator
      val_[size_] = 0; // terminate
      String *res = new String(val_);
      val_ = nullptr; // val_ was consumed above
      return res;
  }
};
