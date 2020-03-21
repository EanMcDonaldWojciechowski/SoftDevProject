Note: We are including both Milestone 1 and Milestone 2 in this paper. Please remember to add grades for both of those assignments when you give us feedback. Thanks!

Ean Wojciechowski and Hiren Patel
Software Development
03/21/2020

Introduction: where we give a high-level description of the eau2 system.

The eau2 system will be able to store and provide functionality on large data sets. The goal of the eau2 system is to provide a means to store large dataframes (100GB) across a distributed network of nodes. Each node will be responsible for a subset of the large dataset, allowing users of our application to analyze or modify the dataset with speed.

Architecture: where we describe the various part of eau2 at a high-level.

We will have to have an application that is in charge of handling different processes. The application class can be extended to add the desired functionality of the user.

Application Level:

The application level will provide an extensible API that allows users to read and write data from/to the KVStore. The Application is run on each node. Each application instance will have a KVStore that manages storing and retrieving data. From the perspective of the application level there is only one central KVStore.

KVStore Level:

Each node has a KVStore locally. The KVStore is responsible for storing and retrieving data. The KVStore acts as a single unit. Each KVStore has a map of all the keys to their values. When setting values for keys, if the key lives in the KVStore locally, then the KVStore will just set the value in the map for that key. If the key lives in a different node, the KVStore will let the networking level know which node has the key and what values to send over to the node so that it’s KVStore can store that key’s value in its map.

Networking Level:

Each KVStore has a client field. This field is an instance of a client node. A KVStore will construct a client node during instantiation and connect its node to the server. The server will then pass on each client’s information to every other client so that they can talk to each other directly. When a KVStore needs to get or put data in a remote node it will use its client node field to communicate with the required node. The client node will construct the proper message to send through the wire and the receiving client will decode the message and pass it to the KVStore so that the KVStore can get the value for its map for that key and pass that value back to the requesting node.

Sorer:
We want to parse a sor file and create a dataframe. The sorer will go through each field in the file, construct the schema, populate the columns, and create a dataframe at the end.

Implementation: where we describe how the system is built, this can include a description of the classes and their API, but only the class you deem relevant and worth describing. (For example: do not describe the utility classes.)

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
   Hashmap store;
   Client node;
   size_t nodeIndex;

   KVStore(size_t nodeIndex) {
     initizlizeNetwork();
   }

   ~KVStore() {}

   void put(Key k, Value v) {}

   Object* get(Key k) {}

   Object* waitAndGet(Key k) {}

   void initializeNetwork() {}

};

Client and Networking:
The “main” node (node 0) will start the server for all other clients to connect to. The server address will be hardcoded in every application instance, which will allow clients to connect to the server. Once every client (1 client per node) connects to the server, each client will receive the registration information of its neighbors. The clients can use this information to send messages directly to its neighbors to request data from neighbors or serve data requests to neighbors.

From the client’s perspective, some things we will have to do is to allow the clients to listen for messages, process requests, and send out the requested information to the correct node. Right now, we have threads that listen to incoming messages and then the main thread sends out messages. Something new we want to implement is a select feature so that we can take away from the complexities of working with multiple threads.

With the select feature, we would just delegate the listening functionality to the OS and we check with the OS from time to time to see if a message is waiting for us. When we receive a message, our program will retrieve the information the request asked for and then send that information to the correct node.


Serialization:
Currently, we are able to serialize and deserialize information. We will need to use the logic there to serialize and deserialize requests and KV information transfer. This will be a very similar method to what we do right now.


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

What does get() and waitAndGet() return? Is it an Object, Dataframe, Array? Will we need to cast? Example: DataFrame* result = kv.waitAndGet(verify);

Currently, Dataframe::fromArray() takes an array and creates a dataframe, which is stored in the distributed kvstore. The example code given to us demonstrates how to store a single column dataframe. If we create a dataframe from a sor file using sorer, how can this dataframe (which already has values and numerous columns) be stored in the kvstore. Do we need to create a Dataframe::fromDataframe method? How can we store an existing dataframe in the distributed kvstore?

Status: where you describe what has been done and give an estimate of the work that remains.

The conceptual is done. A lot of the detailed technical stuff is done. We now have to get what we have done for previous assignments working for this assignments. This means changing serialize and deserialize to work with requests and key value store data, clients to be able to delegate information to the right clients, the Key Value stores, the columns to store different chunks on different clients, and the application to be able to work with all of that.
