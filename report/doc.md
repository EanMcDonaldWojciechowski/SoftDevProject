Ean Wojciechowski and Hiren Patel
Software Development
04/13/2020

Introduction:

The eau2 system will be able to store and provide functionality on large data sets. The goal of the eau2 system is to provide a means to store large dataframes (100GB) across a distributed network of nodes. Each node will be responsible for a subset of the large dataset, allowing users of our application to analyze or modify the dataset with speed.

Architecture:

Our eau2 application architecture will be described in detail below. We created an extendable application class, which has a KVStore instance created during construction. Each application instance uses their KVStore object to communicate and distribute data across the network of nodes.

Linus Level:

Linus is an extension of the application level with implementation details that use the eau2 distributed kvstore application to calculate the levels of linus.

Word Count Level:

The word count level is an extension of the application level with specific methods and features for counting the words in a document. The word counter uses the virtual run method of application to enable instances of the wordCount class to all perform some function when run.

Application Level:

The application level will provide an extensible API that allows users to read and write data from/to the KVStore. The Application is run on each node. Each application instance will have a KVStore that manages storing and retrieving data. From the perspective of the application level there is only one central KVStore.

KVStore Level:

Each node has a KVStore instance created during construction. The KVStore is responsible for storing and retrieving data (dataframes). Even though there is a KVStore on each node, they act as a single unit. If a user from node 1 puts into the KVStore then another user on node 2 can retrieve that data. The KVStore consists of two layers. First there is the KVStore layer, and beneath that there is the ChunkStore layer. The KVStore is responsible for mapping a Key to a Dataframe. From the application tier / user perspective a key is mapped directly to a dataframe’s serialized value, but underneath the KVStore is the ChunkStore, which is responsible for chunking the dataframe. Each column in a dataframe is used to create a single column dataframe. Each single column dataframe is then stored in the distributed store by ChunkStore. Our column implementation was already designed to be chunkable from a previous assignment, so we wrote a method for serializing and deserializing specific chunks in a column.

ChunkStore Level:

As described in the KVStore section, the ChunkStore is responsible for mapping keys to single column dataframes. ChunkStore’s put method will take a key and single column dataframe, break the column into serialized chunks, and send those chunks to be distributed across the other nodes in the network.

Networking Level:

Each ChunkStore has a client field. This field is an instance of a client node. A ChunkStore will construct a client node during instantiation and connect its node to the server. The server will then pass on each client’s information to every other client so that they can talk to each other directly. When a ChunkStore needs to get or put data in a remote node it will use its client node field to communicate with the required node. The client node will construct the proper message to send through the wire and the receiving client will decode the message and pass it to the ChunkStore so that the ChunkStore can get the value for its map for that key and pass that value back to the requesting node. We added a messaging queue to be able to receive multiple messages and handle them in order without losing packets.

Sorer:
We used another group’s Sorer implementation and made a few small changes to make it compatible with our implementation of Object, String, and Columns.

Implementation:

Linus:
Our linus code is functional, but not consistent. We are able to read in at least 1 GB worth of code from each file and run linus on it. We spent a considerable amount of time going through our memory leaks and fixing them using valgrind, though we were not able to get all of them. We will seem to have segmentation faults if we use a very large dataset.

WordCounter:
We were able to get parts of the given code working but we had a hard time getting different parts of the code working with our implementation. This is because we made some design decisions that disagree with the design of the given code. We made an attempt to change both the given code and our code to work together but we ran into problems with pointers and addresses.

Sorer:
Our original sorer implementation was created in Python, so we chose to use cs4500-a1p1’s sorer. To interface our DataFrame with their sorer we made a few changes to their implementation. We created a new method sorToDataframe, which would convert a read sor file into a dataframe. We refactored their adapter to work with our column implementation, giving us the ability to read data from a sor file and add it column by column to a dataframe.

Application:
The application class is very simple and contains two fields: nodeIndex and kv. Each instance of the application class (which will be run on each node or machine) will have a local kv store and a nodeIndex indicating the index of this node.

class Application : public Object {
 public:
   size_t nodeIndex;
   KVStore kv;

   Application(size_t nodeIndex_) {}
   ~Application() {}

   void run() {}

};


KVStore:
The KVStore class represents the underlying kvstore mechanism that will support the distributed data storing application. The kvstore class has three fields: store (the ChunkStore instance that each KVStore creates upon construction), num_nodes, and nodeIndex. The KVStore uses its chunkstore to store and retrieve data across the network. From the perspective of a user there is only one share KVStore across every node in the system.

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

When it comes to actually taking in received messages, we implemented both a threaded and select model. We have one thread in charge of using the select statement and retrieving the next buffer filled with messages. The thread then parses over that data and splits it into the individual commands. This is because the OS will hold onto all of our message on a socket until we call select and it will just dump all the individual commands into one long char*. Once the buffer has been parsed, we store each command on an array. We then have a different thread in charge of going through all the PUT and GET commands.

Serialization:
Currently, we are able to serialize and deserialize information. To tackle the new changes we made to KVStore and the networking layers we had to change how we serialized columns. To handle chunking the columns of a dataframe we implemented two methods in column: serializechunk and deserializechunk. These methods only serialize one section of a column so that we can store a column across multiple nodes.


Use cases:

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


Two more use cases are WordCounter and Linus. Both applications extend the application class and use the powers of our distributed KVStore to compute / solve a problem.

For WordCounter, this application can be used to distribute words to multiple nodes and each node can perform a work count operation on themselves and hand back the result to the leader node. The leader node can then simply sum up all the results. This leads to much less resource use from just one node performing WordCount from a large dataset.

For Linus, this application can be used to figure out the degrees of separation between individuals. Each node can store and run sub calculations which can help get the final result.

Open questions:

How is a single computer able to handle 8 GB worth of data? Our computers use all of their ram when initially reading the data file.

Can we solve our memory leaks in the future if we complete more valgrind checking?

Status: where you describe what has been done and give an estimate of the work that remains.

We are done with the assignment, but we really struggled this week with getting Linus to work on the entire dataset. We can use a fraction of the dataset and have Linus run successfully pretty reliably. We went through Valgrind and fixed many memory leaks. However, we found that we were not able to go through all of the bugs Valgrind pointed out which made it harder to run our code because now parts of it were better and other parts were still coded based on the old assumptions.
