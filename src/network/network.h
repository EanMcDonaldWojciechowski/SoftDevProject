#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <iostream>
#include <thread>
#include <queue>

#define TRUE   1
#define FALSE  0
#define PORT 8888


class Server {
  public:
    char* myIP;
    int numRoutes = 0;
    int* routes;
    sockaddr_in* actualClientRoutes;
    int opt = TRUE;
    int master_socket = 0;
    int addrlen;
    int new_socket;
    int* client_socket;
    int max_clients;
    int valread = 0;
    int sd;
    int max_sd;
    char *buffer; //data buffer of 1K
    struct sockaddr_in address;
    fd_set readfds; //set of socket descriptors
    bool open;

  Server(char* ip, size_t num_nodes) {
    max_clients = num_nodes;
    routes = new int[max_clients];
    actualClientRoutes = new sockaddr_in[max_clients];
    client_socket = new int[max_clients];
    myIP = new char[16];
    memset(myIP, 0, 16);
    strcat(myIP, "127.0.0.1");
    buffer = new char[2048];
    std::cout<<"Server constructor done.\n";
  }

  ~Server() {
    delete[] routes;
    delete[] actualClientRoutes;
    delete[] client_socket;
    delete[] buffer;
    delete[] myIP;
  }

  void initialize() {
    std::cout << "server init started \n";
    open = true;

    for (int i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    master_socket = openSocket(0, address);
    std::cout<<"waiting for clients to connect.\n";
    waitForConnections();
    std::cout << "waited for connects \n";
    informClients();
    std::cout << "informed clients \n";
  }

  int openSocket(size_t socketNum, sockaddr_in address) {
    int currSocket;
    if((currSocket = socket(AF_INET , SOCK_STREAM , socketNum)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if(setsockopt(currSocket, SOL_SOCKET, SO_REUSEADDR,
       (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons( PORT );
    if (bind(currSocket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    return currSocket;
  }

  void waitForConnections() {

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, max_clients) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept the incoming connection
    addrlen = sizeof(address);

    while(numRoutes < max_clients) {
        // THESE ARE COMMENTED OUT BECAUSE VALGRIND HAD A PROBLEM WITH AN UNINITIALIZED VALUE
        //printf("before new connection , socket fd is %d , ip is : %s , port : %d\n" ,
        //new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        if ((new_socket = accept(master_socket,
                (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // THESE ARE COMMENTED OUT BECAUSE VALGRIND HAD A PROBLEM WITH AN UNINITIALIZED VALUE
        //inform user of socket number - used in send and receive commands
       //printf("New connection , socket fd is %d , ip is : %s , port : %d\n" ,
       //new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

       // Reading client information
       int nPort;
       valread = read(new_socket, buffer, 2048);
       if(valread > 0) {
         //printf("Got information from Client: %s\n", buffer);
         struct sockaddr_in neighbor;
         neighbor.sin_family = AF_INET;
         char charIP[11];
         memset(charIP, 0, 11);
         memcpy(charIP, &buffer[3],9);
         charIP[10] = '\0';
         char charPort[5];
         memset(charPort, 0, 5);
         memcpy(charPort, &buffer[15],4);
         charPort[4] = '\0';
         char* pEnd;
         nPort = strtol(charPort, &pEnd, 10);
         neighbor.sin_port = htons(nPort);

         std::cout << "This is the port " << charIP << " and this is the port " << nPort << "\n";

         if(inet_pton(AF_INET, charIP, &neighbor.sin_addr)<=0) {
            printf("\nInvalid address/ Address not supported \n");
            exit(1);
         }
         memset(buffer, 0, 2048);
         actualClientRoutes[numRoutes] = neighbor;
       }
       routes[numRoutes] = nPort;
       numRoutes++;

        //add new socket to array of sockets
        for (int i = 0; i < max_clients; i++) {
            //if position is empty
            if( client_socket[i] == 0 ) {
                client_socket[i] = new_socket;
                break;
            }
        }
    }

  }

  bool activityOnThisSocket(int i) {
    sd = client_socket[i];
    if(sd > 0)
        FD_SET( sd , &readfds);
    struct timeval tv = {0, 0};
    bool retval = select(sd, &readfds , NULL , NULL , &tv);
    return retval;
  }

  void informClients() {
    for (int i = 0; i < numRoutes; i++) {
      for (int j = 0; j < numRoutes; j++) {
          if (i == j) {
            continue;
          }

          char* header = new char[256];
          memset(header, 0, 256);
          struct sockaddr_in client_addr = actualClientRoutes[j];
          strcpy(header, "IP:");
          char* thisaddress = new char[10]; // "127.0.0.1"
          memset(thisaddress, 0, 10);
          char* actualAddress = inet_ntoa(client_addr.sin_addr);
          strcpy(thisaddress, actualAddress);
          strcat(header, thisaddress);
          strcat(header, " P:");
          int thisport = ntohs(client_addr.sin_port);
          char* thisportchar = new char[6];
          memset(thisportchar, 0, 6);
          sprintf(thisportchar, "%d", thisport);
          strcat(header, thisportchar);

          sd = client_socket[i];
          send(sd, header, strlen(header), 0);
          std::cout << "Sending to client " << routes[i] << ": " << header << "\n";
          delete[] header;
          delete[] thisaddress;
          delete[] thisportchar;
      }
    }
  }

  void checkConnections() {
    //else its some IO operation on some other socket
    for (int i = 0; i < max_clients; i++) {
      char vOut[7];
      int thisport = routes[i];
      sd = client_socket[i];
      sprintf(vOut, "%d", thisport);
      if (FD_ISSET( sd , &readfds)) {
          //Check if it was for closing , and also read the
          //incoming message
          if ((valread = read( sd , buffer, 2048)) == 0) {
              //Somebody disconnected , get his details and print
              getpeername(sd , (struct sockaddr*)&address , \
                  (socklen_t*)&addrlen);
              printf("Host disconnected , ip %s , port %d \n" ,
                    inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

              //Close the socket and mark as 0 in list for reuse
              close( sd );
              client_socket[i] = 0;
              int newIndex = 0;
              int* newRoutes = new int[max_clients];
              for (int moveRoute = 0; moveRoute < numRoutes; moveRoute++) {
                if (moveRoute == i) {
                  continue;
                } else {
                  newRoutes[newIndex] = routes[moveRoute];
                  newIndex++;
                }
              }
              numRoutes--;
          } else {
              send(sd , vOut , strlen(vOut) , 0 );
          }
      }
    }
  }

  void terminate() {
    this->~Server();
  }

};


class Client {
public:
  char* myIP;
  int* sendSockets;
  int* recSockets;
  sockaddr_in* neighborRoutes;
  int myPort = 0;
  int sock = 4;
  int valread = 0;
  struct sockaddr_in serv_addr = {};
  struct sockaddr_in my_addr = {};
  char buffer[4096] = {0};
  bool online = TRUE;
  int opt = TRUE;
  int currSocket = 0;
  int numNeighbors = 0;
  Hashmap *store;
  size_t num_nodes;
  std::queue <char*> msgsArr;
  std::queue <int> sdArr;
  std::thread* t1;
  std::thread* t2;
  std::thread* t3;

  Client(char* ip, int port, Hashmap *store_, size_t num_nodes_ ) {
    std::cout << "Client constructor starting \n";
    if (store_ == nullptr) {
      std::cout << "enter a valid hashmap pointer \n";
      exit(1);
    }
    num_nodes = num_nodes_;
    myIP = ip;
    myPort = port;
    store = store_;
    std::cout << "\n\n\nOfficial Map Client adr: " << store << "\n\n\n";
    sendSockets = new int[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
      sendSockets[i] = -1;
    }
    recSockets = new int[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
      recSockets[i] = -1;
    }
    neighborRoutes = new sockaddr_in[num_nodes];
    for (int k = 0; k < num_nodes; k++) {
      neighborRoutes[k] = {};
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, myIP, &my_addr.sin_addr) <= 0) {
      printf("\nInvalid address/ Address not supported \n");
  		exit(1);
    }
    if((currSocket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (bind(currSocket, (struct sockaddr *)&my_addr, sizeof(my_addr))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Client conn info: , socket fd is %d , ip is : %s , port : %d\n" ,
    currSocket , inet_ntoa(my_addr.sin_addr) , ntohs(my_addr.sin_port));
    connectToServer();
    configureWithServer();


    t1 = new std::thread(&Client::acceptPeers, this);
    connectToPeers();
    t1->join();
    t2 = new std::thread(&Client::readPeerMessages, this);
    t2->detach();
    t3 = new std::thread(&Client::handleMessageQueue, this);
    t3->detach();
    usleep(1000);
    for (int i = 0; i < numNeighbors; i++) {
      std::cout << "ports: " << ntohs(neighborRoutes[i].sin_port) << "\n";
      std::cout << "sendSockets[i]: " << sendSockets[i] << "\n";
      std::cout << "recSockets[i]: " << recSockets[i] << "\n";
    }
  }

  ~Client() {
    delete[] sendSockets;
    delete[] recSockets;
    delete[] neighborRoutes;
    delete myIP;
    delete store;
    // t1->join();
    t2->join();
    t3->join();
    delete t1;
    delete t2;
    delete t3;
  }

  void sendMessage(int sendTo, const char* msg) {
    // figure out which socket to use
    int clientSock;
    for (int i = 0; i < numNeighbors; i++) {
      if (ntohs(neighborRoutes[i].sin_port) == sendTo) {
        clientSock = sendSockets[i];
      }
    }
    // usleep(1000);
    // msg[strlen(msg)] = '\0';
    usleep(100);
    send(clientSock , msg , strlen(msg), 0);
    //std::cout << "Sending message to socket " << clientSock << " :" << msg << "\n";
  }

  void connectToServer() {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
  		printf("\n Socket creation error \n");
  		exit(1);
  	}
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_port = htons(PORT);
  	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
  		printf("\nInvalid address/ Address not supported \n");
  		exit(1);
  	}
    printf("Server connection in client , socket fd is %d , ip is : %s , port : %d\n" ,
    sock , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));
  	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
  		printf("\nConnection Failed \n");
      exit(1);
  	}

  }

  void configureWithServer() {
    char* header = new char[256];
    memset(header, 0, 256);
    strcpy(header, "IP:");
    char* thisaddress = new char[10];
    strcpy(thisaddress, myIP);
    strcat(header, thisaddress);
    strcat(header, " P:");
    int thisport = myPort;
    char *thisportchar = new char[6];
    memset(thisportchar, 0, 6);
    sprintf(thisportchar, "%d", thisport);
    strcat(header, thisportchar);
    send(sock, header, strlen(header), 0);
    std::cout << "Sending to server my info: " << header << "\n";
    delete[] header;
    delete[] thisaddress;
    delete[] thisportchar;


    for (int k = 0; k < num_nodes - 1; k++) {
      valread = read(sock, buffer, 4096);
      if(valread > 0) {
        //printf("Got information from Server: %s\n", buffer);
        struct sockaddr_in neighbor;
        neighbor.sin_family = AF_INET;
        char charIP[11];
        memset(charIP, 0, 11);
        memcpy(charIP, &buffer[3],9);
        charIP[10] = '\0';
        char charPort[5];
        memcpy(charPort, &buffer[15],4);
        charPort[4] = '\0';
        char* pEnd;
        int nPort = strtol(charPort, &pEnd, 10);
        neighbor.sin_port = htons(nPort);
        std::cout << "n2 IP is " << charIP << " and port is " << nPort << "\n";
        if(inet_pton(AF_INET, charIP, &neighbor.sin_addr)<=0) {
      		printf("\nInvalid address/ Address not supported \n");
      		exit(1);
      	}
        neighborRoutes[numNeighbors] = neighbor;
        numNeighbors++;
        memset(buffer, 0, 4096);
        // delete pEnd;
      }
    }

  }

  void acceptPeers() {
    int new_peer = 0;
    listen(currSocket , 2);
    struct sockaddr_in peer_addr = {};
    int numPeers = 0;
    std::cout<<"Accepting new peers. numPeers is " << numPeers << " and numNeighbors is " << numNeighbors << "\n";
    while((new_peer = accept(currSocket, (struct sockaddr *)&peer_addr, (socklen_t*)&peer_addr)) > 0) {
      printf("Peer connection in client , socket fd is %d , ip is : %s , port : %d\n" ,
      new_peer , inet_ntoa(peer_addr.sin_addr) , ntohs(peer_addr.sin_port));
      valread = read(new_peer , buffer, 4096);
      char *pEnd;
      size_t spotInArray = strtol(buffer, &pEnd, 10);
      std::cout << "INIT other client's port: " << spotInArray << " received on socket " << new_peer << "\n";

      int pos;
      for (int i = 0; i < numNeighbors; i++) {
        if (ntohs(neighborRoutes[i].sin_port) == spotInArray) {
          pos = i;
        }

      }
      recSockets[pos] = new_peer;
      numPeers++;
      if (numPeers == numNeighbors) {
        break;
      }
    }
  }


  void connectToPeers() {
    usleep(10000);
    for (int i = 0; i < numNeighbors; i++) {
      struct sockaddr_in peer_addr = {};
      peer_addr = neighborRoutes[i];
      int peerSock;
      peerSock = socket(AF_INET , SOCK_STREAM , 0);
      if(peerSock == 0) {
        perror("peer socket failed");
        exit(EXIT_FAILURE);
      }
      printf("Connecting to peer client , socket fd is %d , ip is : %s , port : %d\n" ,
      peerSock , inet_ntoa(peer_addr.sin_addr) , ntohs(peer_addr.sin_port));
      if (connect(peerSock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
      }
      char *data = new char[4096];
      memset(data, 0, 4096);
      char *returnChar = new char[256];
      memset(returnChar, 0, 256);
      snprintf(returnChar, 8, "%d", myPort);
      strcat(data, returnChar);
      std::cout << "INIT sending to socket " << peerSock << " with my port " << data << "\n";
      send(peerSock , data , strlen(data) , 0);
      sendSockets[i] = peerSock;
      delete[] returnChar;
      delete[] data;
    }
  }

  void handleMessageQueue() {
    int i = 0;
    while (TRUE) {
      while(!msgsArr.empty())  {
        // std::cout << "\nREADING QUEUE NOW .... \n\n";
        // if (i % 1000 == 0) {
        //     std::cout << i << " msg in array: " << msgsArr.front() << "\n";
        // }
        // std::cout << sdArr.front() << "\n\n";
        receivedMessage(msgsArr.front(), sdArr.front()); // OR Just delete the char* right after this.
        msgsArr.pop();
        sdArr.pop();
        i++;
      }
    }
  }

  void readPeerMessages() {
    char* nextBuffer = new char[4096];
    memset(nextBuffer, 0, 4096);
    bool haveNextBuffer = 0;
    while (TRUE) {
      size_t selectStatus = 0;
      struct timeval tv = {};
      fd_set *fdread = new fd_set;
      FD_ZERO(fdread);
      fd_set *writefds = new fd_set;
      FD_ZERO(writefds);
      fd_set *exceptfds = new fd_set;
      FD_ZERO(exceptfds);
      size_t max_sd = 0;
      tv.tv_sec = 0;
      tv.tv_usec = 1;


      for (int i = 0; i < num_nodes - 1; i++) {
        FD_SET(recSockets[i], fdread);
        max_sd = (max_sd > recSockets[i]) ? max_sd : recSockets[i];
      }

      //std::cout << "max_sd " << max_sd << " fdread " << (fdread == nullptr) << " writefds " << (writefds == nullptr)
      //          << " exceptfds " << (exceptfds == nullptr) << "\n";
      selectStatus = select((max_sd + 1), fdread, writefds, exceptfds, &tv);
      if (selectStatus < 0) {
          printf("select failed\n ");
          return exit(1);
      } else if (selectStatus == 0) {
          // Do nothing
          // while(!msgsArr.empty())  {
          //   // std::cout << "\nREADING QUEUE NOW .... \n\n";
          //   // std::cout << msgsArr.front() << "\n\n";
          //   // std::cout << sdArr.front() << "\n\n";
          //   receivedMessage(msgsArr.front(), sdArr.front()); // OR Just delete the char* right after this.
          //   msgsArr.pop();
          //   sdArr.pop();
          // }
      } else {
        for (int k = 0; k < numNeighbors; k++) {
          int sd = recSockets[k];
          if (FD_ISSET(sd, fdread)) {
            memset(buffer, 0, 4096);
            int ret = recv(sd, (char *)buffer, sizeof(buffer), 0);
            if(ret > 0) {
                //printf("Message received from socket %d : %s \n", sd, buffer);

                int begSeq = 0;
                for (int j = 0; j < strlen(buffer); j++) {
                  // std::cout << buffer[j] << ".";
                  // std::cout << "j = " << j << "\n";
                  if (buffer[j] == '{') { // OR WE CAN JUST LOOK FOR PUT AND GET :/
                    // std::cout << "found end pointer\n";
                    char* msg = new char[4096];
                    memset(msg, 0, 4096);
                    memcpy(msg, &buffer[begSeq], (j) - begSeq); // add the sd array
                    // std::cout << "found a message part: " << msg << "\n";
                    begSeq = j + 1;
                    if (haveNextBuffer) {
                      char* msg3 = new char[4096];
                      memset(msg3, 0, 4096);
                      strcat(msg3, nextBuffer);
                      strcat(msg3, msg);
                      msgsArr.push(msg3);
                      sdArr.push(sd);
                      // std::cout << "AFTER PARTIAL MESSAGE = " << msg3 << "\n";
                      memset(nextBuffer, 0, 4096);
                      // exit(1);
                      haveNextBuffer = 0;
                    } else {
                      msgsArr.push(msg);
                      sdArr.push(sd);
                    }
                  } else {
                    if (j == strlen(buffer) - 1) {
                      // char* msg2 = new char[4096];
                      memset(nextBuffer, 0, 4096);
                      memcpy(nextBuffer, &buffer[begSeq], (j) - begSeq); // add the sd array
                      haveNextBuffer = 1;
                      //std::cout << "\n\n\n\n\n\nFOUND A PARTIAL MESSAGE IN NETOWRK: " << nextBuffer << "\n";
                      // exit(1);
                    }
                  }

                }
            }
          }
        }
      }
      delete fdread;
      delete writefds;
      delete exceptfds;
    }
  }

  void receivedMessage(char* message, int sd) {
    char* msgType = new char[4];
    memset(msgType, 0, 4);
    msgType[3] = '\0';

    for (int i = 0; i < 3; i++) {
        msgType[i] = message[i];
    }


    if (strcmp(msgType, "PUT") == 0) {
      //std::cout << " PUTTING THIS : " << message << "\n";
      storeLocal(message);

    } else if (strcmp(msgType, "GET") == 0) {
      retrieveLocal(message, sd);
    } else {
      std::cout << "NETWORK ERROR: Found an unrecognized message: " << message << "\n";
      exit(1);
    }
    delete[] msgType;
  }

  void storeLocal(char* message) {
    int i;
    Key *k;
    char* keyChar = new char[256];
    memset(keyChar, 0, 256);
    for (i = 4; i < strlen(message); i++) {
      if (message[i] == '}') {
          k = new Key(keyChar, myPort - 8810);
          break;
      }
      char theval[2] = {0};
      theval[0] = message[i];
      strcat(keyChar, theval);
    }
    char* val = new char[4096];
    memset(val, 0, 4096);
    memcpy(val, &message[i + 1], (strlen(message) - i + 1));
    val[strlen(message) - i + 1] = '\0';
    Value *v = new Value(val);
    store->put(k, v);
    // std::cout << "in network after put...\n";
    // store->printall();
    memset(message, 0, 4096); // if we use queue, we would delete the char* here instead of memset. this memset would move up to after we add the copy of buffer to the array.
    // std::cout << "in network done with storelocal...\n";
  }

  void retrieveLocal(char* message, int sd) {
    size_t sendToPort;
    for (int i = 0; i < numNeighbors; i++) {
      // std::cout << "ports: " << ntohs(neighborRoutes[i].sin_port) << "\n";
      // std::cout << "sendSockets[i]: " << sendSockets[i] << "\n";
      // std::cout << "recSockets[i]: " << recSockets[i] << "\n";
      if (recSockets[i] == sd) {
        sendToPort = ntohs(neighborRoutes[i].sin_port);
      }
    }

    char* keyChar = new char[256];
    memset(keyChar, 0 , 256);
    Key *k;
    // std::cout << "Looking for key in message " << message << "\n";
    for (int i = 4; i < strlen(message); i++) {
      if (message[i] == '}') {
          k = new Key(keyChar, myPort - 8810);
          break;
      }
      char theval[2] = {0};
      theval[0] = message[i];
      strcat(keyChar, theval);
    }
    char *keyVal = new char[4];
    memset(keyVal, 0, 4);
    strcat(keyVal, "RSP");
    // store->printall();
    // std::cout << "In network Looking for key " << k->key << "\n";
    Value *v = dynamic_cast<Value*>(store->get(k));
    // std::cout << "Found value for key " << k->key << " : " << v->value << "\n";
    Key *tempKey = new Key(keyVal, myPort - 8810);
    char* returnMsg = v->dataToSend(tempKey);
    // usleep(10000);
    returnMsg[strlen(returnMsg)] = '{';
    sendMessage(sendToPort, returnMsg);
    memset(message, 0, 4096); // if we use queue, we would delete the char* here instead of memset. this memset would move up to after we add the copy of buffer to the array.
  }

  void terminate() {
    this->~Client();
  }

};
