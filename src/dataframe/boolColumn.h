#pragma once

/*************************************************************************
 * BoolColumn::
 * Holds bool values.
 */
class BoolColumn : public Column {
 public:
   size_t sizeOfChunk = 4; // number of elements per chunk
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

  BoolColumn(size_t height) {
    if (height % sizeOfChunk > 0) {
      numOfChunks_ = (height / sizeOfChunk) + 1;
    } else {
      numOfChunks_ = height / sizeOfChunk;
    }
    count_ = height;
    elements = new bool*[numOfChunks_];
    for (int chunk = 0; chunk < numOfChunks_; chunk++) {
      elements[chunk] = new bool[sizeOfChunk];
    }
  }

  void grow_() {
    bool** nElements = new bool*[numOfChunks_+ 1];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk];
    }
    nElements[numOfChunks_] = new bool[sizeOfChunk];
    numOfChunks_++;
    delete[] elements;
    elements = nElements;
  }

  void push_back(int val) {
    exit(1);
  }

  void push_back(bool val) {
    if (count_ % sizeOfChunk == 0) {
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
};
