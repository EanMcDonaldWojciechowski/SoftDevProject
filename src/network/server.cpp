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

  Server *s = new Server(ip);
  sleep(18);
  s->terminate();
  return 0;
}
