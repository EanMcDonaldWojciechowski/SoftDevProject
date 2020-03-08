#include "string.h"
#include "serial.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

class StringArray {
public:
   String* vals_;
   size_t numElements;

   StringArray(String* s1, String* s2, String* s3) {
     vals_ = new String[3];
     vals_[0] = *s1;
     vals_[1] = *s2;
     vals_[2] = *s3;
     numElements = 3;
   }

   StringArray() {
     vals_ = new String[3];
     numElements = 0;
   }

   ~StringArray() {

   }

   void serialize(Serialize &ser) {
     char* data = new char[1024];
     data[1023] = '\0';

     for (int i = 0; i < numElements; i++) {
       strcat(data, vals_[i].c_str());
       strcat(data, "}");
     }

     ser.data = data;
   }

    void deserialize(Serialize &ser) {
      int charSize = strlen(ser.data);
      char* currField = new char[256];
      currField[255] = '\0';
      int setFieldNum = 0;
      for (int i = 0; i < charSize; i++) {
        if (ser.data[i] == '}') {
          String* s = new String(currField);
          vals_[setFieldNum] = *s;
          setFieldNum++;
          numElements++;
          memset(currField, 0, strlen(currField));
          continue;
        }
        char theval[2] = {0};
        theval[0] = ser.data[i];
        strcat(currField, theval);
      }
    }

    void print() {
      for (int i = 0; i < numElements; i++) {
        std::cout << "String at " << i << " is "<< vals_[i].c_str() << "\n";
      }
    }

};



class DoubleArray {
public:
  double* vals_;
  size_t numElements;

  DoubleArray(double d1, double d2, double d3) {
    vals_ = new double[3];
    vals_[0] = d1;
    vals_[1] = d2;
    vals_[2] = d3;
    numElements = 3;
  }

  DoubleArray() {
    vals_ = new double[3];
    numElements = 0;
  }

  ~DoubleArray() {
  }

  void serialize(Serialize &ser) {
    char* data = new char[1024];
    data[1023] = '\0';

    for (int i = 0; i < numElements; i++) {
      char doubleChar[256];
      snprintf(doubleChar,sizeof(vals_[i]), "%f", vals_[i]);
      strcat(data, doubleChar);
      strcat(data, "}");
    }


    ser.data = data;
  }

  void deserialize(Serialize &ser) {
    int charSize = strlen(ser.data);
    char* currField = new char[256];
    currField[255] = '\0';
    int setFieldNum = 0;
    for (int i = 0; i < charSize; i++) {
      if (ser.data[i] == '}') {
        char* pEnd;
        vals_[setFieldNum] = strtod(currField, &pEnd);
        setFieldNum++;
        numElements++;
        memset(currField, 0, strlen(currField));
        continue;
      }
      char theval[2] = {0};
      theval[0] = ser.data[i];
      strcat(currField, theval);
    }
  }

  void print() {
    for (int i = 0; i < numElements; i++) {
      std::cout << "Double at " << i << " is "<< vals_[i] << "\n";
    }
  }

};



enum class MsgKind { Ack, Nack, Put,
                    Reply,  Get, WaitAndGet, Status,
                    Kill,   Register,  Directory };


class Message : public Object {
public:
    MsgKind kind_;  // the message kind
    size_t sender_; // the index of the sender node
    size_t target_; // the index of the receiver node
    size_t id_;     // an id t unique within the node

    Message() {
      kind_ = MsgKind::Ack;
      sender_ = 0;
      target_ = 0;
      id_ = 0;
    }

    Message(MsgKind kind, size_t sender, size_t target, size_t id) {
      kind_ = kind;
      sender_ = sender;
      target_ = target;
      id_ = id;
    }

    ~Message() {
    }

    void serialize(Serialize &ser) {
      char* data = new char[1024];
      data[1023] = '\0';

      char senderChar[256];
      sprintf(senderChar, "%d", sender_);
      strcat(data, senderChar);
      strcat(data, "}");
      char targetChar[256];
      sprintf(targetChar, "%d", target_);
      strcat(data, targetChar);
      strcat(data, "}");
      char idChar[256];
      sprintf(idChar, "%d", id_);
      strcat(data, idChar);
      strcat(data, "}");
      ser.data = data;
    }


    void deserialize(Serialize &ser) {
      int charSize = strlen(ser.data);
      char* currField = new char[256];
      currField[255] = '\0';
      int setFieldNum = 0;
      for (int i = 0; i < charSize; i++) {
        if (ser.data[i] == '}') {
          if (setFieldNum == 0) {
            char* pEnd;
            sender_ = strtol(currField, &pEnd, 10);
            setFieldNum++;
          } else if (setFieldNum == 1) {
            char* pEnd;
            target_ = strtol(currField, &pEnd, 10);
            setFieldNum++;
          } else if (setFieldNum == 2) {
            char* pEnd;
            id_ = strtol(currField, &pEnd, 10);
            setFieldNum++;
          }
          memset(currField, 0, strlen(currField));
          continue;
        }
        char theval[2] = {0};
        theval[0] = ser.data[i];
        strcat(currField, theval);
      }
    }

    void print() {
      if (kind_ == MsgKind::Ack) {
          std::cout << "Kind is Ack" << "\n";
      }
      std::cout << "sender is "<< sender_ << "\n";
      std::cout << "target is "<< target_ << "\n";
      std::cout << "id is "<< id_ << "\n";
    }


};



class Ack : public Message {
  public:
    Ack(MsgKind kind, size_t sender, size_t target, size_t id) {
      Message(kind, sender, target, id);
    }

    ~Ack() {

    }
};



class Status : public Message {
public:
    String* msg_; // owned

   Status(MsgKind kind, size_t sender, size_t target, size_t id, String* msg) {
     kind_ = kind;
     sender_ = sender;
     target_ = target;
     id_ = id;
     msg_ = msg;
     std::cout << msg->c_str() << "\n";
   }

   Status() {
     msg_ = new String("");
     kind_ = MsgKind::Status;
     sender_ = 0;
     target_ = 0;
     id_ = 0;
   }

   ~Status() {

   }


   void serialize(Serialize &ser) {
     char* data = new char[1024];
     data[1023] = '\0';
     strcat(data, "Status}");
     char senderChar[256];
     sprintf(senderChar, "%d", sender_);
     strcat(data, senderChar);
     strcat(data, "}");
     char targetChar[256];
     sprintf(targetChar, "%d", target_);
     strcat(data, targetChar);
     strcat(data, "}");
     char idChar[256];
     sprintf(idChar, "%d", id_);
     strcat(data, idChar);
     strcat(data, "}");
     strcat(data, msg_->c_str());
     strcat(data, "}");
     ser.data = data;
   }

   void deserialize(Serialize &ser) {
     int charSize = strlen(ser.data);
     char* currField = new char[1];
     currField[0] = '\0';
     int setFieldNum = 0;
     for (int i = 0; i < charSize; i++) {
       if (ser.data[i] == '}') {
         if (setFieldNum == 0) {
           MsgKind mk;
           if (strcmp(currField, "Ack") == 0) {
             mk = MsgKind::Ack;
           } else if (strcmp(currField, "Nack") == 0) {
             mk = MsgKind::Nack;
           } else if (strcmp(currField, "Put") == 0) {
             mk = MsgKind::Put;
           } else if (strcmp(currField, "Reply") == 0) {
             mk = MsgKind::Reply;
           } else if (strcmp(currField, "Get") == 0) {
             mk = MsgKind::Get;
           } else if (strcmp(currField, "WaitAndGet") == 0) {
             mk = MsgKind::WaitAndGet;
           } else if (strcmp(currField, "Status") == 0) {
             mk = MsgKind::Status;
           } else if (strcmp(currField, "Kill") == 0) {
             mk = MsgKind::Kill;
           } else if (strcmp(currField, "Register") == 0) {
             mk = MsgKind::Register;
           } else if (strcmp(currField, "Directory") == 0) {
             mk = MsgKind::Directory;
           } else {
             printf("Bad Value for MsgKind.");
             exit(1);
           }
           kind_ = mk;
           setFieldNum++;
         } else if (setFieldNum == 1) {
           char* pEnd;
           sender_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 2) {
           char* pEnd;
           target_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 3) {
           char* pEnd;
           id_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
           std::cout<<"THIS IS CURRFIELD "<< setFieldNum<<"\n";
         } else if (setFieldNum == 4) {
           std::cout<<"THIS IS CURRFIELD "<< currField<<"\n";
           String* s = new String(currField);
           msg_ = s;
           setFieldNum++;
         }
         memset(currField, 0, strlen(currField));
         continue; // LEFT HERE
       }
       char theval[2] = {0};
       theval[0] = ser.data[i];
       strcat(currField, theval);
     }
   }

   void print() {
     if (kind_ == MsgKind::Status) {
         std::cout << "Kind is Ack" << "\n";
     } else {
        std::cout << "WORKING: WRONG TYPE" << "\n";
     }
     std::cout << "sender is "<< sender_ << "\n";
     std::cout << "target is "<< target_ << "\n";
     std::cout << "id is "<< id_ << "\n";
     std::cout << "msg is " << msg_->c_str() << "\n";
   }

};



class Register : public Message {
public:
    sockaddr_in client;
    size_t port;

    Register(MsgKind kind, size_t sender, size_t target, size_t id, sockaddr_in client_, size_t port_) {\
      kind_ = kind;
      sender_ = sender;
      target_ = target;
      id_ = id;
      client = client_;
      port = port_;
    }

    Register() {}

    ~Register() {

    }

    void serialize(Serialize &ser) {
      char* data = new char[1024];
      data[1023] = '\0';

      strcat(data, "Register}");
      char senderChar[256];
      sprintf(senderChar, "%d", sender_);
      strcat(data, senderChar);
      strcat(data, "}");
      char targetChar[256];
      sprintf(targetChar, "%d", target_);
      strcat(data, targetChar);
      strcat(data, "}");
      char idChar[256];
      sprintf(idChar, "%d", id_);
      strcat(data, idChar);
      strcat(data, "}");
      char portChar[256];
      sprintf(portChar, "%d", port);
      strcat(data, portChar);
      strcat(data, "}");

      char* thisaddress = new char[10]; // "127.0.0.1"
      char* actualAddress = inet_ntoa(client.sin_addr);
      strcpy(thisaddress, actualAddress);
      strcat(data, thisaddress);
      strcat(data, "}");
      int thisport = ntohs(client.sin_port);
      char thisportchar[6];
      sprintf(thisportchar, "%d", thisport);
      strcat(data, thisportchar);
      strcat(data, "}");


      ser.data = data;
    }

      void deserialize(Serialize &ser) {
        int charSize = strlen(ser.data);
        char* currField = new char[256];
        currField[255] = '\0';
        int setFieldNum = 0;
        client.sin_family = AF_INET;
        for (int i = 0; i < charSize; i++) {
          if (ser.data[i] == '}') {
            if (setFieldNum == 0) {
              MsgKind mk;
              if (strcmp(currField, "Ack") == 0) {
                mk = MsgKind::Ack;
              } else if (strcmp(currField, "Nack") == 0) {
                mk = MsgKind::Nack;
              } else if (strcmp(currField, "Put") == 0) {
                mk = MsgKind::Put;
              } else if (strcmp(currField, "Reply") == 0) {
                mk = MsgKind::Reply;
              } else if (strcmp(currField, "Get") == 0) {
                mk = MsgKind::Get;
              } else if (strcmp(currField, "WaitAndGet") == 0) {
                mk = MsgKind::WaitAndGet;
              } else if (strcmp(currField, "Status") == 0) {
                mk = MsgKind::Status;
              } else if (strcmp(currField, "Kill") == 0) {
                mk = MsgKind::Kill;
              } else if (strcmp(currField, "Register") == 0) {
                mk = MsgKind::Register;
              } else if (strcmp(currField, "Directory") == 0) {
                mk = MsgKind::Directory;
              } else {
                printf("Bad Value for MsgKind.");
                exit(1);
              }
              kind_ = mk;
              setFieldNum++;
            } else if (setFieldNum == 1) {
              char* pEnd;
              sender_ = strtol(currField, &pEnd, 10);
              setFieldNum++;
            } else if (setFieldNum == 2) {
              char* pEnd;
              target_ = strtol(currField, &pEnd, 10);
              setFieldNum++;
            } else if (setFieldNum == 3) {
              char* pEnd;
              id_ = strtol(currField, &pEnd, 10);
              setFieldNum++;
            } else if (setFieldNum == 4) {
              char* pEnd;
              port = strtol(currField, &pEnd, 10);
              setFieldNum++;
            } else if (setFieldNum == 5) {
              inet_pton(AF_INET, currField, &client.sin_addr);
              setFieldNum++;
            } else if (setFieldNum == 6) {
              char* pEnd;
              int nPort = strtol(currField, &pEnd, 10);
              client.sin_port = htons(nPort);
              setFieldNum++;
            }
            memset(currField, 0, strlen(currField));
            continue;
          }
          char theval[2] = {0};
          theval[0] = ser.data[i];
          strcat(currField, theval);
        }
      }

      void print() {
        if (kind_ == MsgKind::Register) {
            std::cout << "Kind is Directory" << "\n";
        } else {
           std::cout << "WORKING: WRONG TYPE" << "\n";
        }
        std::cout << "sender is "<< sender_ << "\n";
        std::cout << "target is "<< target_ << "\n";
        std::cout << "id is "<< id_ << "\n";
        char* actualAddress = inet_ntoa(client.sin_addr);
        std::cout << "client address is " << actualAddress << "\n";
        int thisport = ntohs(client.sin_port);
        std::cout << "client port is " << thisport << "\n";
        std::cout << "port is " << port << "\n";
      }

};



class Directory : public Message {
public:
   size_t client;
   size_t * ports;  // owned
   String ** addresses;  // owned; strings owned
   size_t portSize;
   size_t addrSize;

   Directory(size_t portSize_, size_t addressSize_) {
     addrSize = addressSize_;
     portSize = portSize_;
     ports = new size_t[portSize_];
     addresses = new String*[addressSize_];
   }

   Directory(MsgKind kind, size_t sender, size_t target, size_t id, size_t client_,
     size_t* ports_, size_t portSize_, String** addresses_, size_t addrSize_) {
     kind_ = kind;
     sender_ = sender;
     target_ = target;
     id_ = id;
     client = client_;
     ports = ports_;
     addresses = addresses_;
     addrSize = addrSize_;
     portSize = portSize_;
   }

   ~Directory() {

   }

   void serialize(Serialize &ser) {
     char* data = new char[1024];
     data[1023] = '\0';
     strcat(data, "Directory}");
     char senderChar[256];
     sprintf(senderChar, "%d", sender_);
     strcat(data, senderChar);
     strcat(data, "}");
     char targetChar[256];
     sprintf(targetChar, "%d", target_);
     strcat(data, targetChar);
     strcat(data, "}");
     char idChar[256];
     sprintf(idChar, "%d", id_);
     strcat(data, idChar);
     strcat(data, "}");
     char clientChar[256];
     sprintf(clientChar, "%d", client);
     strcat(data, clientChar);
     strcat(data, "}");
     char portLenChar[256];
     sprintf(portLenChar, "%d", portSize);
     strcat(data, portLenChar);
     strcat(data, "}");
     for (int i = 0; i < portSize ; i++) {
       char portChar[256];
       sprintf(portChar, "%d", ports[i]);
       strcat(data, portChar);
       strcat(data, "}");
     }
     char portAddLenChar[256];
     sprintf(portAddLenChar, "%d", addrSize);
     strcat(data, portAddLenChar);
     strcat(data, "}");
     for (int i = 0; i < addrSize ; i++) {
       strcat(data, addresses[i]->c_str());
       strcat(data, "}");
     }


     ser.data = data;
   }

   void deserialize(Serialize &ser) {
     int charSize = strlen(ser.data);
     char* currField = new char[1];
     currField[0] = '\0';
     int setFieldNum = 0;
     int loopAtPortNum;
     int loopAtStringNum;
     for (int i = 0; i < charSize; i++) {
       if (ser.data[i] == '}') {
         std::cout<<"This is currField: " << currField << " and setFieldNum:" << setFieldNum << "\n";
         if (setFieldNum == 0) {
           MsgKind mk;
           if (strcmp(currField, "Ack") == 0) {
             mk = MsgKind::Ack;
           } else if (strcmp(currField, "Nack") == 0) {
             mk = MsgKind::Nack;
           } else if (strcmp(currField, "Put") == 0) {
             mk = MsgKind::Put;
           } else if (strcmp(currField, "Reply") == 0) {
             mk = MsgKind::Reply;
           } else if (strcmp(currField, "Get") == 0) {
             mk = MsgKind::Get;
           } else if (strcmp(currField, "WaitAndGet") == 0) {
             mk = MsgKind::WaitAndGet;
           } else if (strcmp(currField, "Status") == 0) {
             mk = MsgKind::Status;
           } else if (strcmp(currField, "Kill") == 0) {
             mk = MsgKind::Kill;
           } else if (strcmp(currField, "Register") == 0) {
             mk = MsgKind::Register;
           } else if (strcmp(currField, "Directory") == 0) {
             mk = MsgKind::Directory;
           } else {
             printf("Bad Value for MsgKind.");
             exit(1);
           }
           kind_ = mk;
           setFieldNum++;
         } else if (setFieldNum == 1) {
           char* pEnd;
           sender_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 2) {
           char* pEnd;
           target_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 3) {
           char* pEnd;
           id_ = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 4) {
           char* pEnd;
           client = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == 5) {
           char* pEnd;
           portSize = strtol(currField, &pEnd, 10);
           loopAtPortNum = setFieldNum;
           setFieldNum++;
         } else if (setFieldNum > 5 && setFieldNum <= 5 + portSize) {
           char* pEnd;
           ports[setFieldNum - loopAtPortNum - 1] = strtol(currField, &pEnd, 10);
           setFieldNum++;
         } else if (setFieldNum == loopAtPortNum + portSize + 1) {
           char* pEnd;
           addrSize = strtol(currField, &pEnd, 10);
           loopAtStringNum = setFieldNum;
           setFieldNum++;
         } else if (setFieldNum > loopAtPortNum + portSize + 1) {
           String* s = new String(currField);
           addresses[setFieldNum - loopAtStringNum - 1] = s;
           setFieldNum++;
         }
         memset(currField, 0, strlen(currField));
         continue;
       }
       char theval[2] = {0};
       theval[0] = ser.data[i];
       strcat(currField, theval);
     }
   }

   void print() {
     if (kind_ == MsgKind::Directory) {
         std::cout << "Kind is Directory" << "\n";
     } else {
        std::cout << "WORKING: WRONG TYPE" << "\n";
     }
     std::cout << "sender is "<< sender_ << "\n";
     std::cout << "target is "<< target_ << "\n";
     std::cout << "id is "<< id_ << "\n";
     std::cout << "client is "<< client << "\n";
     std::cout << "portSize is "<< portSize << "\n";
     for (int i = 0; i < portSize; i++) {
       std::cout << "port "<< i << " is "<< ports[i] << "\n";
     }
     std::cout << "addrSize is "<< addrSize << "\n";
     for (int i = 0; i < addrSize; i++) {
       std::cout << "address "<< i << " is "<< addresses[i]->c_str() << "\n";
     }
   }
};
