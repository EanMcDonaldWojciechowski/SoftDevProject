Note: We are including both Milestone 1 and Milestone 2 in this paper. Please remember to add grades for both of those assignments when you give us feedback. Thanks!

Ean Wojciechowski and Hiren Patel
Software Development
04/06/2020

Introduction: where we give a high-level description of the eau2 system.

The eau2 system will be able to store and provide functionality on large data sets. The goal of the eau2 system is to provide a means to store large dataframes (100GB) across a distributed network of nodes. Each node will be responsible for a subset of the large dataset, allowing users of our application to analyze or modify the dataset with speed.

Architecture: where we describe the various part of eau2 at a high-level.

We will have to have an application that is in charge of handling different processes. The application class can be extended to add the desired functionality of the user.

Word Count Level:
The word count level is an extension of the application level with specific methods and features for counting the words in a document. The word counter uses the virtual run method of application to enable instances of the wordCount class to all perform some function when run.

Application Level:

The application level will provide an extensible API that allows users to read and write data from/to the KVStore. The Application is run on each node. Each application instance will have a KVStore that manages storing and retrieving data. From the perspective of the application level there is only one central KVStore.

KVStore Level:

Each node has a KVStore locally. The KVStore is responsible for storing and retrieving data. The KVStore acts as a single unit. The KVStore consists of two layers. First there is the KVStore layer, and beneath that there is the ChunkStore layer. The KVStore is responsible for mapping a Key to a Dataframe Value. From the application tier / user perspective a key is mapped directly to a dataframe’s serialized value, but underneath the KVStore is the ChunkStore, which is responsible for chunking and storing the dataframe. Each column in a dataframe is broken into chunks. Our column implementation was already designed this way from a previous assignment, so we wrote a method for serializing and deserializing specific chunks in a column.

Networking Level:

Each KVStore has a client field. This field is an instance of a client node. A KVStore will construct a client node during instantiation and connect its node to the server. The server will then pass on each client’s information to every other client so that they can talk to each other directly. When a KVStore needs to get or put data in a remote node it will use its client node field to communicate with the required node. The client node will construct the proper message to send through the wire and the receiving client will decode the message and pass it to the KVStore so that the KVStore can get the value for its map for that key and pass that value back to the requesting node.

Sorer:
We want to parse a sor file and create a dataframe. The sorer will go through each field in the file, construct the schema, populate the columns, and create a dataframe at the end.

Implementation: where we describe how the system is built, this can include a description of the classes and their API, but only the class you deem relevant and worth describing. (For example: do not describe the utility classes.)

WordCounter:
Our code works from the standpoint of our Dataframe downwards. We can get parts of your code given to us working but we are having a hard time getting different parts of your code working with other parts of your code. This is because you made some design decisions that we may not agree with and may not work with how we structured our APIs. We made an attempt to change both you code and our code to work together but we ran into problems with pointers and addresses since we handle our code differently.

Sorer:
Our original sorer implementation was created in Python, so we chose to use cs4500-a1p1’s sorer. To interface our DataFrame with their sorer we made a few changes to their implementation. We created a new method sorToDataframe, which would convert a read sor file into a dataframe. We refactored their adapter to work with our column implementation, giving us the ability to read data from a sor file and add it column by column to a dataframe.

Application:
Currently, the application class is very simple and contains two fields: nodeIndex and kv. Each instance of the application class (which will be run on each node or machine) will have a local kv store and a nodeIndex indicating the index of this node.

class Application : public Object {
 public:
   size_t nodeIndex;
   KVStore kv;

   Application(size_t nodeIndex_) {}
   ~Application() {}

   void run() {}

};


KVStore:
The KVStore class represents the underlying kvstore mechanism that will support the distributed data storing application. The kvstore class has three fields: store (the local store of data, where key is a string and value is a serialized blob), Node (this KVStore’s client, which allows it to communicate with the other KVStores), and nodeIndex. The KVStore has a local hashmap for storing data, but the client field allows the KVStore to retrieve and store serialized information from other nodes. From a user perspective multiple nodes have KVStores that are all synchronized and consistent, even though the data is physically stored across the whole system.

class KVStore : public Object {
 public:
   ChunkStore *store;
   size_t nodeIndex;

   KVStore(size_t nodeIndex_) {
     nodeIndex = nodeIndex_;
     store = new ChunkStore(nodeIndex);
   }

   ~KVStore() {}

   void put(Key k, Value v) {}

   Object* get(Key k) {}

   Object* waitAndGet(Key k) {}

   void initializeNetwork() {}

};

ChunkStore:
The chunk store is in charge of managing where each chunk of a column gets delegated to and retrieving and building the column back up when the value of a key is requested. We had chunks start at node 0 and go round-robin to each node in order using mod. The chunk store is also in charge of initializing the network layer. If the chunk the chunk store is looking for lives locally, it will just retrieve that value and add onto the dataframe column. When the chunk store needs a chunk that does not live locally, it will request it from one of its neighbors and keep checking its database for a “RSP” key. The other chunk will send a response with the values requested with the key “RSP”. Once the “RSP” key is inserted into the requester’s hashmap, the requester will take the value of the key, parse that data and add those values in a dataframe, and remove the key and value from its hashmap so we are not aggregating data locally and using up our memory. Once one chunk is done, we will look for the next chunk and repeat the process.

class ChunkStore : public Object {
public:
  Hashmap* store;
  Client *client;
  size_t nodeIndex;
  size_t basePort = 8810;


  ChunkStore(size_t nodeIndex_) {}

  ~ChunkStore() {

  }

  void put(Key *k, DataFrame *v);

  DataFrame* get(Key *k);

  void waitForKey(Key* k);

  Value* getChunkVal(size_t chunkNum, Key *ChunkKey);

  void sendInfo(Key *chunkKey, Value *val);
};


Client and Networking:
The “main” node (node 0) will start the server for all other clients to connect to. The server address will be hardcoded in every application instance, which will allow clients to connect to the server. Once every client (1 client per node) connects to the server, each client will receive the registration information of its neighbors. The clients can use this information to send messages directly to its neighbors to request data from neighbors or serve data requests to neighbors.

We changed the way our Clients listen to messages from their peers. We moved from numerous threads to the select method. When we receive a message, our program will retrieve the information the request asked for and then send that information to the correct node.

We created a simple message protocol for our network. Messages can have three letter prefixes that determine the type of message. GET is the code that is sent when a node wants to access a chunk value stored on another node. The message would include GET followed by the key that is being gotten. PUT is the code that is sent when a node wants to tell its neighbor to store a chunk. The PUT command should be followed by the key and data that will be stored.

The final code we have implemented so far is RSP, which is the response code. When a node sends a GET request it must have some way of waiting for a response. To handle get requests we send a PUT message back to the node that originally sent the GET request with a special Key(“RSP”). After the GET request is made the network waits for a response, which comes in the form of PUT RSP followed by the data that was requested. The original node then waits until its local hashmap has a key of RSP (which will contain the GET value it is looking for) and then deserializes the value of the value paired with the key RSP.


Serialization:
Currently, we are able to serialize and deserialize information. To tackle the new changes we made to KVStore and the networking layers we had to change how we serialized columns. To handle chunking the columns of a dataframe we implemented two methods in column: serializechunk and deserializechunk. These methods only serialize one section of a column so that we can store a column across multiple nodes.


Use cases: examples of uses of the system. This could be in the form of code like the one above. It is okay to leave this section mostly empty if there is nothing to say. Maybe just an example of creating a dataframe would be enough.

A use case of this application would be storing a large set of data in the application and a dataframe and performing an operation:

Data.sor: (imagine this sor file is 1x10000 (row x col))

<1>
<2>
<3>
...


Use sorer to real file and create dataframe and add two to every int or float:

class Demo : public Application {
public:
  Key main("main",0);
  Key adder("adder",0);

  Demo(size_t idx): Application(idx) {}

  void run_() override {
    switch(this_node()) {
    case 0:   producer();     break;
    case 1:   adder();          break;
   }
  }

  void producer() {
   FILE *f = fopen("./Data.sor", "r");
   SOR* reader = new SOR();
   reader->read(f, 0, 10000);
   DataFrame *df = reader->sorToDataframe();
   DataFrame::fromArray(&main, &kv, df->length(), df->column[0]);

  }

  void adder() {
    DataFrame* vals = kv.waitAndGet(main);
    //rower that adds two to every int and float
    Rower* myRower = new RowerAddTwo();
    vals->pmap(*myRower);
  }
};




Open questions: where you list things that you are not sure of and would like the answer to.

What are the main differences between get and waitandget? Does wait and get just wait until the key exists? Does this go on forever? What happens if get is called and the key does not exist?

Status: where you describe what has been done and give an estimate of the work that remains.

We are 95% done with the assignment, but we really struggled this week meshing our code with the provided wordCounter. We are mostly certain that our distributed KVStore and data frames work, but we are having issues with pointers and memory copies in SIMap, which uses our Array class under the hood. We ran out of time to fully integrate this wordCounter on top of our Application class.
