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

#define TRUE   1
#define FALSE  0
#define PORT 8888


class Server {
  public:
    char* myIP;
    int numRoutes = 0;
    int* routes = new int[3];
    sockaddr_in* actualClientRoutes = new sockaddr_in[3];
    int opt = TRUE;
    int master_socket;
    int addrlen;
    int new_socket;
    int client_socket[3];
    int max_clients = 3;
    int valread;
    int sd;
    int max_sd;
    char buffer[1025]; //data buffer of 1K
    struct sockaddr_in address;
    fd_set readfds; //set of socket descriptors
    bool open;

  Server(char* ip) {
    // initialize();
  }

  ~Server() {
    delete[] routes;
    delete[] actualClientRoutes;
  }

  void initialize() {
    open = true;
    // myIP = ip;
    myIP = "127.0.0.1";
    for (int i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    master_socket = openSocket(0, address);

    waitForConnections();
    informClients();
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
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept the incoming connection
    addrlen = sizeof(address);

    while(numRoutes < max_clients) {

        printf("before new connection , socket fd is %d , ip is : %s , port : %d\n" ,
        new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        if ((new_socket = accept(master_socket,
                (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //inform user of socket number - used in send and receive commands
       printf("New connection , socket fd is %d , ip is : %s , port : %d\n" ,
       new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

       // Reading client information
       int nPort;
       if((valread = read( new_socket , buffer, 1024) > 0)) {
         printf("Got information from Client: %s\n", buffer);
         struct sockaddr_in neighbor;
         neighbor.sin_family = AF_INET;
         char charIP[10];
         memcpy(charIP, &buffer[3],9);
         charIP[10] = '\0';
         char charPort[5];
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
         memset(buffer, 0, 1025);
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

          char* header = new char[4];
          struct sockaddr_in client_addr = actualClientRoutes[j];
          strcpy(header, "IP:");
          char* thisaddress = new char[10]; // "127.0.0.1"
          char* actualAddress = inet_ntoa(client_addr.sin_addr);
          strcpy(thisaddress, actualAddress);
          strcat(header, thisaddress);
          strcat(header, " P:");
          int thisport = ntohs(client_addr.sin_port);
          char thisportchar[6];
          sprintf(thisportchar, "%d", thisport);
          strcat(header, thisportchar);

          sd = client_socket[i];
          send(sd, header, strlen(header), 0);
          std::cout << "Sending to client " << routes[i] << ": " << header << "\n";
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
          if ((valread = read( sd , buffer, 1025)) == 0) {
              //Somebody disconnected , get his details and print
              getpeername(sd , (struct sockaddr*)&address , \
                  (socklen_t*)&addrlen);
              printf("Host disconnected , ip %s , port %d \n" ,
                    inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

              //Close the socket and mark as 0 in list for reuse
              close( sd );
              client_socket[i] = 0;
              int newIndex = 0;
              int* newRoutes = new int[3];
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
  int* sendSockets = new int[2];
  int* recSockets = new int[2];
  sockaddr_in* neighborRoutes = new sockaddr_in[2];
  int myPort;
  int sock = 4;
  int valread;
  struct sockaddr_in serv_addr;
  struct sockaddr_in my_addr;
  char buffer[1025] = {0};
  bool online = TRUE;
  int opt = TRUE;
  int currSocket;
  int numNeighbors = 0;
  Hashmap *store;

  Client(char* ip, int port, Hashmap *store_) {
    if (store_ == nullptr) {
      std::cout << "enter a valid hashmap pointer \n";
      exit(1);
    }
    myIP = ip;
    myPort = port;
    store = store_;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    // if(inet_pton(AF_INET, myIP, &my_addr.sin_addr) <= 0) {
    if(inet_pton(AF_INET, myIP, &my_addr.sin_addr) <= 0) {
      printf("\nInvalid address/ Address not supported \n");
  		exit(1);
    }
    std::cout << "Client IP set to " << inet_ntoa(my_addr.sin_addr) << "\n";

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
    std::cout<<"Done with configureWithServer.\n";
    std::thread* t1 = new std::thread(&Client::acceptPeers, this);
    // sleep(1);
    std::cout<<"Main thread moving to connect to peers.\n";
    connectToPeers();
    std::cout<<"Joining now.\n";
    t1->join();


    std::cout<<"Creating threads to listen to peers.\n";
    // Creating threads to listen to peers.
    /*std::thread* pool[numNeighbors];
    for (int i = 0; i < numNeighbors; i++) {
      int listenSocket = recSockets[i];
      pool[i] = new std::thread(&Client::readPeerMessages, this, listenSocket);
    }*/

    new std::thread(&Client::readPeerMessages, this);

    std::cout<<"Main thread done.\n";
  }

  ~Client() {
    delete[] sendSockets;
    delete[] recSockets;
    // delete[] buffer;
    // neighborRoutes[0].~sockeraddr_in();
    // neighborRoutes[1].~sockeraddr_in();
    delete[] neighborRoutes;
  }

  void sendMessage(int sendTo, const char* msg) {
    std::cout<<"Sending message now.\n";
    // char* header = new char[3];
    // strcpy(header, "S:myPort D:sendTo M:msg");
    // strcpy(header, "S:");
    // char myPortChar[5];
    // sprintf(myPortChar, "%d", myPort);
    // strcat(header, myPortChar);
    // strcat(header, " D:");
    // strcat(header, sendTo);
    // char clientPortChar[5];
    // sprintf(clientPortChar, "%d", sendTo);
    // strcat(header, clientPortChar);
    // strcat(header, " M:");
    // strcat(header, msg);

    // figure out which socket to use
    int clientSock;
    for (int i = 0; i < numNeighbors; i++) {
      if (ntohs(neighborRoutes[i].sin_port) == sendTo) {
        clientSock = sendSockets[i];
      }
    }


    // send(clientSock , header , strlen(header) , 0);
    send(clientSock , msg , strlen(msg) , 0);
    std::cout << "Sending message to socket " << clientSock << " :" << msg << "\n";
  }

  void connectToServer() {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  	{
  		printf("\n Socket creation error \n");
  		exit(1);
  	}
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_port = htons(PORT);

  	// Convert IPv4 and IPv6 addresses from text to binary form
  	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  	{
  		printf("\nInvalid address/ Address not supported \n");
  		exit(1);
  	}

    printf("Server connection in client , socket fd is %d , ip is : %s , port : %d\n" ,
    sock , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port));
  	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  	{
  		printf("\nConnection Failed \n");
      exit(1);
  	}

  }

  void configureWithServer() {

    // Sending my port to server.
    char* header = new char[4];
    strcpy(header, "IP:");
    char* thisaddress = new char[10]; // "127.0.0.1"
    strcpy(thisaddress, myIP);
    strcat(header, thisaddress);
    strcat(header, " P:");
    int thisport = myPort;
    char thisportchar[6];
    sprintf(thisportchar, "%d", thisport);
    strcat(header, thisportchar);
    send(sock, header, strlen(header), 0);
    std::cout << "Sending to server my info: " << header << "\n";

    // Reading first neighbor information
    if((valread = read( sock , buffer, 1024) > 0)) {
      printf("Got information from Server: %s\n", buffer);
      struct sockaddr_in neighbor;
      neighbor.sin_family = AF_INET;
      char charIP[10];
      memcpy(charIP, &buffer[3],9);
      charIP[10] = '\0';
      char charPort[5];
      memcpy(charPort, &buffer[15],4);
      charPort[4] = '\0';
      char* pEnd;
      int nPort = strtol(charPort, &pEnd, 10);
      neighbor.sin_port = htons(nPort);
      std::cout << "n1 IP is " << charIP << " and port is " << nPort << "\n";
      if(inet_pton(AF_INET, charIP, &neighbor.sin_addr)<=0)
    	{
    		printf("\nInvalid address/ Address not supported \n");
    		exit(1);
    	}
      neighborRoutes[numNeighbors] = neighbor;
      numNeighbors++;
      memset(buffer, 0, 1025);
    }

    // Reading second neighbor information
    if((valread = read( sock , buffer, 1024) > 0)) {
      printf("Got information from Server: %s\n", buffer);
      struct sockaddr_in neighbor;
      neighbor.sin_family = AF_INET;
      char charIP[10];
      memcpy(charIP, &buffer[3],9);
      charIP[10] = '\0';
      char charPort[5];
      memcpy(charPort, &buffer[15],4);
      charPort[4] = '\0';
      char* pEnd;
      int nPort = strtol(charPort, &pEnd, 10);
      neighbor.sin_port = htons(nPort);
      std::cout << "n2 IP is " << charIP << " and port is " << nPort << "\n";
      if(inet_pton(AF_INET, charIP, &neighbor.sin_addr)<=0)
    	{
    		printf("\nInvalid address/ Address not supported \n");
    		exit(1);
    	}
      neighborRoutes[numNeighbors] = neighbor;
      numNeighbors++;
      memset(buffer, 0, 1025);
    }

  }

  void acceptPeers() {
    int new_peer = 0;
    listen(currSocket , 2);
    struct sockaddr_in peer_addr;
    int numPeers = 0;
    std::cout<<"Accepting new peers. numPeers is " << numPeers << " and numNeighbors is " << numNeighbors << "\n";
    while((new_peer = accept(currSocket, (struct sockaddr *)&peer_addr, (socklen_t*)&peer_addr)) > 0)
    {
      printf("Peer connection in client , socket fd is %d , ip is : %s , port : %d\n" ,
      sock , inet_ntoa(peer_addr.sin_addr) , ntohs(peer_addr.sin_port));
      recSockets[numPeers] = new_peer;
      numPeers++;
      if (numPeers == numNeighbors) {
        break;
      }
    }
  }


  void connectToPeers() {
    sleep(1);
    for (int i = 0; i < numNeighbors; i++) {
      struct sockaddr_in peer_addr = neighborRoutes[i];
      int peerSock;
      // open socket for peer connection
      if((peerSock = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("peer socket failed");
        exit(EXIT_FAILURE);
      }
      printf("Connecting to peer client , socket fd is %d , ip is : %s , port : %d\n" ,
      peerSock , inet_ntoa(peer_addr.sin_addr) , ntohs(peer_addr.sin_port));
      if (connect(peerSock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0)
      {
        printf("\nConnection Failed \n");
        exit(1);
      }
      sendSockets[i] = peerSock;
    }
  }

  void readPeerMessages() {
    while (TRUE) {
      size_t selectStatus;
      struct timeval tv;
      fd_set fdread;
      size_t max_sd;
      FD_ZERO(&fdread);
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      for (int i = 0; i < numNeighbors; i++) {
        FD_SET(recSockets[i], &fdread);
        max_sd = (max_sd > recSockets[i]) ? max_sd : recSockets[i];
      }
      selectStatus = select(max_sd + 1, &fdread, NULL, NULL, &tv);
      std::cout << "This is selectStatus: " << selectStatus << "\n";
      if (selectStatus < 0)
      {
          printf("select failed\n ");
          return exit(1);
      } else if (selectStatus == 0) {
          std::cout << "Nothing was read\n";
      } else {
        for (int k = 0; k < numNeighbors; k++) {
          int sd = recSockets[k];
          if (FD_ISSET(sd, &fdread)) {
            int ret = recv(sd, (char *)buffer, sizeof(buffer), 0);
            if(ret > 0) {
                printf("Message received from socket %d : %s \n", sd, buffer);
                receivedMessage(buffer, sd);
            }
          }
        }
      }
    }




      // if (selectStatus == recSockets[0] || selectStatus == recSockets[1]) {
      //   readStatus = read_UDP_socket(FD, buffer, sizeof(buffer), &readCount );
      //   printf("Got information from Client with socket %d: %s\n", sock, buffer);
      //   memset(buffer, 0, 1025);
      // }
      // if((valread = read( sock , buffer, 1024) > 0)) {
      //   printf("Got information from Client with socket %d: %s\n", sock, buffer);
      //   memset(buffer, 0, 1025);
      // }
    // } else {
    //   std::cout << "Nothing was read\n";
    // }
  }

  void receivedMessage(char* message, int sd) {
    char* msgType = new char[4];
    msgType[3] = '\0';

    for (int i = 0; i < 3; i++) {
        msgType[i] = message[i];
    }

    if (strcmp(msgType, "PUT") == 0) {
      storeLocal(message);
    } else if (strcmp(msgType, "GET") == 0) {
      retrieveLocal(message, sd);
    }
  }

  void storeLocal(char* message) {
    int i;
    Key *k;
    Value *v;
    char* keyChar = new char[256];
    for (i = 4; i < strlen(message); i++) {
      if (message[i] == '}') {
          k = new Key(keyChar, myPort - 8810);
          break;
      }
      char theval[2] = {0};
      theval[0] = message[i];
      strcat(keyChar, theval);
    }
    char val[1024];
    memcpy(val, &message[i + 1], (strlen(message) - i + 1));
    val[strlen(message) - i + 1] = '\0';
    v = new Value(val);
    std::cout<<"I will store value " << val << " in key " << keyChar << "\n";
    store->put(k, v);

    std::cout << "attempting to print key k's value: " <<  dynamic_cast<Value*>(store->get(k))->value << "\n";
  }

  void retrieveLocal(char* message, int sd) {
    char* keyChar = new char[256];
    Key *k;
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
    strcat(keyVal, "RSP");

    Value *v = dynamic_cast<Value*>(store->get(k));

    Key *tempKey = new Key(keyVal, myPort - 8810);
    char* returnMsg = v->dataToSend(tempKey);
    strcat(returnMsg, v->value);

    send(sd , returnMsg , strlen(returnMsg) , 0);
    std::cout << "Sending message to socket " << sd << " :" << returnMsg << "\n";

  }

  void terminate() {
    this->~Client();
  }

};
