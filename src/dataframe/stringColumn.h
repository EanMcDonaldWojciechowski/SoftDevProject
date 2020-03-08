#pragma once
/*************************************************************************
 * StringColumn::
 * Holds String values.
 */
class StringColumn : public Column {
 public:
   size_t sizeOfChunk = 4; // number of elements per chunk
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
        // std::cout << "chunk: " << chunk << "i: " << i << "\n";
        val = va_arg(vl, String*);
        String *f = val;

        elements[chunk][i] = f;
        // std::cout << elements[chunk][i]->c_str() << "\n";
      }
    }
    // std::cout << elements[0][0]->c_str() << "\n";
    va_end(vl);
  }

  StringColumn(size_t height) {
    if (height % sizeOfChunk > 0) {
      numOfChunks_ = (height / sizeOfChunk) + 1;
    } else {
      numOfChunks_ = height / sizeOfChunk;
    }
    count_ = height;
    elements = new String**[numOfChunks_];
    for (int chunk = 0; chunk < numOfChunks_; chunk++) {
      String** newS = new String*[sizeOfChunk];
      for (int i = 0; i < sizeOfChunk; i++) {
        newS[i] = nullptr;
      }
      elements[chunk] = newS;
    }
  }

  ~StringColumn() {}

  void grow_() {
    String*** nElements = new String**[numOfChunks_+ 1];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    nElements[numOfChunks_] = new String*[sizeOfChunk];
    numOfChunks_++;
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
    if (count_ % sizeOfChunk == 0) {
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
};
