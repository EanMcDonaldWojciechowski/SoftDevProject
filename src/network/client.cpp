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
  Client *c1 = new Client(ip, 8811);
  // sleep(3); // 2 -> 1
  // sleep(3); // 3 -> 1
  c1->sendMessage(8812, "Hello 2 from 1, nice to meet you."); // 1 --> 2
  c1->sendMessage(8813, "Hello 3 from 1, nice to meet you."); // 1 --> 3
  // sleep(3); // 3 -> 2
  // sleep(3); // 2 -> 3
  // c1->terminate();
  return 1;
}
