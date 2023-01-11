#include "pipe_networking.h"


int main() {

  int b;
  int to_server;
  int from_server;

  from_server = client_handshake( &to_server );


  char buffer[BUFFER_SIZE];
  char initialMsg[] = "init";


  b = write(to_server, initialMsg, sizeof(initialMsg));
  if (b == -1) {
    printf("communication error %d: %s\n", errno, strerror(errno));
    exit(1);
  }

  b = read(from_server, buffer, sizeof(buffer));

  while ( 1 ) {
    printf("[client] response from subserver: %s\n", buffer);
    sleep(1);
    b = read(from_server, buffer, sizeof(buffer));
    if (b == 0) {
      return 0;
    }
  }

}
