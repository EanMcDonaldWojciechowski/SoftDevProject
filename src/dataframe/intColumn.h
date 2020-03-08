#pragma once

/*************************************************************************
 * IntColumn::
 * Holds int values.
 */
class IntColumn : public Column {
 public:
   size_t sizeOfChunk = 4; // number of elements per chunk
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

  IntColumn(size_t height) {
    if (height % sizeOfChunk > 0) {
      numOfChunks_= (height / sizeOfChunk) + 1;
    } else {
      numOfChunks_= height / sizeOfChunk;
    }
    count_ = height;
    elements = new int*[numOfChunks_];
    for (int chunk = 0; chunk < numOfChunks_; chunk++) {
      elements[chunk] = new int[sizeOfChunk];
    }
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
    if (count_ % sizeOfChunk == 0) {
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

  void grow_() {
    int** nElements = new int*[numOfChunks_+ 1];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    nElements[numOfChunks_] = new int[sizeOfChunk];
    numOfChunks_++;
    delete[] elements;
    elements = nElements;
  }

  IntColumn* as_int() {
    return this;
  }

  char get_type() {
    return 'I';
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
};
