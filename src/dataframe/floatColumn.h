#pragma once



/*************************************************************************
 * FloatColumn::
 * Holds float values.
 */
class FloatColumn : public Column {
 public:
   size_t sizeOfChunk = 4; // number of elements per chunk
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

  FloatColumn(size_t height) {
    if (height % sizeOfChunk > 0) {
      numOfChunks_ = (height / sizeOfChunk) + 1;
    } else {
      numOfChunks_ = height / sizeOfChunk;
    }
    count_ = height;
    elements = new float*[numOfChunks_];
    for (int chunk = 0; chunk < numOfChunks_; chunk++) {
      elements[chunk] = new float[sizeOfChunk];
    }
  }

  void push_back(int val) {
    exit(1);
  }

  void push_back(bool val) {
    exit(1);
  }

  void push_back(float val) {
    if (count_ % sizeOfChunk == 0) {
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
    float** nElements = new float*[numOfChunks_+ 1];
    for(size_t chunk = 0; chunk < numOfChunks_; chunk++) {
      nElements[chunk] = elements[chunk]; // set new using pointers to old arrays of ints
    }
    nElements[numOfChunks_] = new float[sizeOfChunk];
    numOfChunks_++;
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
};
