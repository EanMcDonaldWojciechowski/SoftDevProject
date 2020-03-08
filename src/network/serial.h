class Serialize {
  public:
    char* data;

  Serialize() {
    data = new char[256];
    data[255] = '\0';
  }

  Serialize(char* bytes) {
    data = bytes;
  }

  ~Serialize() {

  }

  char* getBytes() {
    return data;
  }

  void getSer(char* bytes) {
    data = bytes;
  }

};
