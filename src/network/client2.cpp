#include "network.h"

int main(int argh , char *argv[]) {
  char* ip;
  for (int i = 1; i < argh; i++) {
    if (strcmp(argv[i], "-ip") == 0) {
      int length = strlen(argv[i + 1]);
      ip = new char[length + 1];
      strcpy(ip, argv[i + 1]);
      ip[length + 1] = '\0';
    }
  }
  Client *c2 = new Client(ip, 8812);
  std::cout<<"Sending message to Client 1\n";
  c2->sendMessage(8811, "Hello 1 from 2, nice to meet you."); // 2 --> 1
  // sleep(3); // 3 -> 1
  // sleep(3); // 1 -> 2
  // sleep(3); // 1 -> 3
  // sleep(3); // 3 -> 2
  c2->sendMessage(8813, "Hello 2 from 3, nice to meet you."); // 2 --> 3
  // c2->terminate();
  return 1;
}
