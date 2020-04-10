// # include <stdio.h>

// class DataFrame;

class Args : public Object {
public:
  char* file = nullptr;   // file to be read
  size_t num_nodes;
  size_t nodeIndex;

  Args() {}

  ~Args() {}

  void parse(int argc, char ** argv) {
    int index = 1; // skipping the first argument
    while (index < argc) {
        if (strcmp(argv[index], "-nodeIndex") == 0) {
            nodeIndex = atoi(argv[index + 1]);
            index+=2;
        }
        else if (strcmp(argv[index], "-nodes") == 0) {
            num_nodes = atoi(argv[index + 1]);
            index+=2;
        }
        else if (strcmp(argv[index], "-file") == 0) {
            file = argv[index + 1];
            index+=2;
        }
        else {
            affirm(false, "Invalid input argument");
        }
    }
  }



};


class KeyBuff : public Object {
  public:
  Key* orig_; // external
  StrBuff buf_;

  // KeyBuff(Key* orig) : orig_(orig), buf_(*orig) {}
  KeyBuff(Key* orig) : orig_(orig), buf_(orig) {}

  ~KeyBuff() {}

  KeyBuff& c(String &s) { buf_.c(s); return *this;  }
  KeyBuff& c(size_t v) {
    buf_.c(v);
    return *this;
  }
  KeyBuff& c(const char* v) { buf_.c(v); return *this; }

  Key* get() {
    String* s = buf_.get();
    // buf_.c(orig_->c_str());
    buf_.c(orig_->key);
    Key* k = new Key(s->steal(), orig_->home());
    delete s;
    return k;
  }
}; // KeyBuff

class FileReader : public Writer {
public:
  Args* arg;
  static const size_t BUFSIZE = 1024;
  char * buf_;
  size_t end_ = 0;
  size_t i_ = 0;
  FILE * file_;

  /** Creates the reader and opens the file for reading.  */
  FileReader(Args *arg_) {
      arg = arg_;
      file_ = fopen(arg->file, "r");
      // if (file_ == nullptr) FATAL_ERROR("Cannot open file " << arg->file);
      if (file_ == nullptr) {
        // std::cout<<"FATAL_ERROR: Cannot open file " << arg->file << "\n";
        exit(1);
      }
      buf_ = new char[BUFSIZE + 1]; //  null terminator
      fillBuffer_();
      skipWhitespace_();
  }

  ~FileReader() {}


  /** Reads next word and stores it in the row. Actually read the word.
      While reading the word, we may have to re-fill the buffer  */
    virtual bool visit(Row & r) override {
      // std::cout << "visit in FR \n";
       assert(i_ < end_);
        assert(! isspace(buf_[i_]));
        size_t wStart = i_;
        // std::cout << "before while true in FR \n";
        while (true) {
            if (i_ == end_) {
                if (feof(file_)) { ++i_;  break; }
                i_ = wStart;
                wStart = 0;
                fillBuffer_();
            }
            if (isspace(buf_[i_]))  break;
            ++i_;
        }
        buf_[i_] = 0;
        // String word(buf_ + wStart, i_ - wStart);
        String *word = new String(buf_ + wStart);
        // std::cout<<"Filereader found word " << word->c_str() << "\n";
        r.set(0, word);
        r.printRow();
        ++i_;
        // std::cout<<"Filereader calling skipWhitespace_()\n";
        skipWhitespace_();
        r.printRow();
        return 1;
    }

    /** Returns true when there are no more words to read.  There is nothing
       more to read if we are at the end of the buffer and the file has
       all been read.     */
    virtual bool done() { return (i_ >= end_) && feof(file_);  }

    /** Reads more data from the file. */
    void fillBuffer_() {
        size_t start = 0;
        // compact unprocessed stream
        if (i_ != end_) {
            start = end_ - i_;
            memcpy(buf_, buf_ + i_, start);
        }
        // read more contents
        end_ = start + fread(buf_+start, sizeof(char), BUFSIZE - start, file_);
        i_ = start;
    }

    /** Skips spaces.  Note that this may need to fill the buffer if the
        last character of the buffer is space itself.  */
    void skipWhitespace_() {
        while (true) {
            if (i_ == end_) {
                if (feof(file_)) return;
                fillBuffer_();
            }
            // std::cout << "char " << buf_[i_] << " is newline?" << (buf_[i_] == '\n') << "\n";
            // if the current character is not whitespace, we are done
            if (!isspace(buf_[i_]))
                return;
            // otherwise skip it
            ++i_;
        }
    }
};


/****************************************************************************/
class Adder : public Reader {
public:
  SIMap& map_;  // String to Num map;  Num holds an int

  Adder(SIMap& map) : map_(map)  {
    std::cout << "map add in adder " << &map_ << "\n";
  }

  ~Adder() {

  }

  virtual bool visit(Row& r) override {
    std::cout << "before num WORD \n";
    // r.printRow();
    std::cout << "before num WORD2 \n";
    String* word = r.get_string(0);
    std::cout << "After getting string\n";
    std::cout << "WORD " << word->c_str() << "\n";
    std::cout << "before num WORD3\n";
    assert(word != nullptr);
    Num* num = map_.contains(*word) ? map_.get(*word) : new Num();
    std::cout << "before set WORD \n";
    assert(num != nullptr);
    num->v++;
    map_.set(word, num);
    std::cout << "SET WORD \n";
    // std::cout << "map size in the visit function" << map_.size_ << "\n";
    // std::cout << "Looking at Keys in address in visit " << &(map_.items_->keys_) << "\n";
    std::cout << map_.items_->keys_->to_string() << "\n";

    return false;
  }
};

/***************************************************************************/
class Summer : public Writer {
public:
  SIMap& map_;
  size_t i = 0;
  size_t j = 0;
  size_t seen = 0;

  Summer(SIMap& map) : map_(map) {
      if (!k()) {
          next();
      }
  }

  ~Summer() {}

  void next() {
      assert(!done());
      if (i == map_.capacity_ ) return;
      j++;
      ++seen;
      if ( j >= map_.items_[i].keys_->size() ) {
          ++i;
          j = 0;
          while( i < map_.capacity_ && map_.items_[i].keys_->size() == 0 ) {
            i++;
          }
      }
  }

  String* k() {
      if (i==map_.capacity_ || j == map_.items_[i].keys_->size()) {
          return nullptr;
      }
      return (String*) (map_.items_[i].keys_->get_(j));
  }

  size_t v() {
      if (i == map_.capacity_ || j == map_.items_[i].keys_->size()) {
          assert(false); return 0;
      }
      return ((Num*)(map_.items_[i].vals_->get_(j)))->v;
  }

  virtual bool visit(Row& r) {
      String & key = *k();
      size_t value = v();
      r.set(0, &key);
      r.set(1, (int) value);
      next();
      return 1;
  }

  bool done() {
        return seen == map_.size();
    }
};

class Application : public Object {
 public:
   size_t nodeIndex;
   size_t num_nodes;
   KVStore *kv;

   Application(size_t nodeIndex_, size_t num_nodes_) {
     nodeIndex = nodeIndex_;
     num_nodes = num_nodes_;
     kv = new KVStore(nodeIndex, num_nodes);
   }

   // Application() {
   //   nodeIndex = 0;
   //   num_nodes = 0;
   //   kv = nullptr;
   // }

   ~Application() {

   }

   virtual void run_() {};


};

/****************************************************************************
 * Calculate a word count for given file:
 *   1) read the data (single node)
 *   2) produce word counts per homed chunks, in parallel
 *   3) combine the results
 **********************************************************author: pmaj ****/
class WordCount: public Application {
public:
  static const size_t BUFSIZE = 1024;
  Key* in;
  KeyBuff* kbuf;
  SIMap all;
  size_t num_nodes;
  Args* arg;

  // WordCount(size_t idx, NetworkIfc & net):
  //   Application(idx, net), in("data"), kbuf(new Key("wc-map-",0)) { }

   // WordCount(size_t idx): Application(idx, arg.num_nodes), in("data"), kbuf(new Key("wc-map-",0)) { }


  WordCount(Args* arg_) : Application(arg_->nodeIndex, arg_->num_nodes) {
    arg = arg_;
    char* data = new char[5];
    strcat(data, "data");
    in = new Key(data);
    kbuf = new KeyBuff(new Key("wc-map-",0));
  }

  ~WordCount() {

  }

  /** The master nodes reads the input, then all of the nodes count. */
  // void run_() override {
  //   if (nodeIndex == 0) {
  //     FileReader fr(arg);
  //     delete DataFrame::fromVisitor(&in, &kv, "S", fr);
  //   }
  //   local_count();
  //   reduce();
  // }
  virtual void run_();

  /** Returns a key for given node.  These keys are homed on master node
   *  which then joins them one by one. */
  Key* mk_key(size_t idx) {
      Key * k = kbuf->c(idx).get();
      // LOG("Created key " << k->c_str());
      return k;
  }

  /** Compute word counts on the local node and build a data frame. */
  // void local_count() {
  //   DataFrame* words = (kv.get(in));
  //   // DataFrame* words = (kv.waitAndGet(in)); // We need to local implementation
  //   // p("Node ").p(nodeIndex).pln(": starting local count...");
  //   std::cout << "Node " << nodeIndex << ": starting local count...\n";
  //   SIMap map;
  //   Adder add(map);
  //   words->map(add);
  //   // words->local_map(add); // df doesn't know about networking so it is just working with local data
  //   delete words;
  //   Summer cnt(map);
  //   delete DataFrame::fromVisitor(mk_key(nodeIndex), &kv, "SI", cnt);
  // }
  void local_count();

  /** Merge the data frames of all nodes */
  void reduce() {
    if (nodeIndex != 0) return;
    std::cout << "Node 0: reducing counts...\n";
    SIMap map;
    Key* own = mk_key(0);
    merge(kv->get(own), map);
    for (size_t i = 1; i < num_nodes; ++i) { // merge other nodes
      Key* ok = mk_key(i);
      merge(kv->waitAndGet(ok), map);
      delete ok;
    }
    // p("Different words: ").pln(map.size());
    std::cout << "Different words: " << map.size() << "\n";
    delete own;
  }

  // void merge(DataFrame* df, SIMap& m) {
  //   Adder add(m);
  //   df->map(add);
  //   delete df;
  // }
  void merge(DataFrame* df, SIMap& m);
}; // WordcountDemo
