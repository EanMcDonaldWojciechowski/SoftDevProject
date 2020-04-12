// #include <iostream>
// #include <cstdlib>
using namespace std;

// define default capacity of the queue
// #define SIZE 10

// Class for queue
class Queue {
	char **arr;		// array to store queue elements
	int capacity;	// maximum capacity of the queue
	int front;		// front points to front element in the queue (if any)
	int size;		// current size of the queue

public:
	Queue() {
    capacity = 128;
    arr = new char*[capacity];
    int front = 0;
    int size = 0;
  } 	// constructor

  Queue(int capacity_) {
    capacity = capacity_;
    arr = new char*[capacity];
    int front = 0;
    int size = 0;
  } 	// constructor

	~Queue() {
    delete[] arr;
  }   				// destructor

  // Utility function to remove front element from the queue
  char* pop() {
  	// check for queue underflow
  	if (isEmpty()) 	{
  		cout << "UnderFlow\nProgram Terminated\n";
  		exit(EXIT_FAILURE);
  	}
    int retFront = front;
    cout << "Removing " << arr[retFront] << '\n';
  	front = front + 1;
  	size--;
    return arr[retFront];
  }

  // Utility function to add an item to the queue
  void push(char* message) {
  	// check for queue overflow
  	if (isFull())	{
  		cout << "OverFlow\nProgram Terminated\n";
  		exit(EXIT_FAILURE);
  	}

    if (isFull()) {
      expand();
    }

    char* cpyMsg = new char[strlen(message)];
    strcpy(cpyMsg, message);

  	cout << "Inserting " << cpyMsg << '\n';
    arr[front + size] = cpyMsg;
    size++;
  }

  // Double the capacity of queue when needed
  void expand() {
    Queue *copy = new Queue(capacity_ * 2);
    for (int i = 0; i < capacity; i++) {
      if (!(arr[i] == nullptr)) {
        copy->arr[i] = arr[i];
        copy->size++; // This may not even matter and we may not even need it.
      }
    }
    delete[] arr;
    arr = copy->arr;
    capacity = capacity * 2;
  }

  // Utility function to return front element in the queue
  char* Queue::peek()  {
  	if (isEmpty())	{
  		cout << "UnderFlow\nProgram Terminated\n";
  		exit(EXIT_FAILURE);
  	}
  	return arr[front];
  }

  // Utility function to return the size of the queue
  int Queue::size()  {
  	return count;
  }

  // Utility function to check if the queue is empty or not
  bool Queue::isEmpty()  {
  	return (size() == 0);
  }

  // Utility function to check if the queue is full or not
  bool Queue::isFull()  {
  	return (size() == capacity);
  }
};
