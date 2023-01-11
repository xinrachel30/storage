#include "pipe_networking.h"

static void sighandler(int signo) {
  if ( signo == SIGINT ){
    printf("SIGINT received.\n");

    remove(WKP);

    exit(0);
  }
}

/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  signal(SIGINT, sighandler);
  int b, from_client, to_client;
  char buffer[HANDSHAKE_BUFFER_SIZE];

  //creates WKP
  // printf("[server] handshake: making wkp\n");
  b = mkfifo(WKP, 0600);
  if ( b == -1 ) {
    printf("mkfifo error %d: %s\n", errno, strerror(errno));
    exit(-1);
  }

  //opens WKP, waiting for a connection
  from_client = open(WKP, O_RDONLY, 0644);

  //creates subserver
  if (from_client != -1) {
    int role = fork();
    if (role == 0) {
      to_client = server_connect(from_client);
      b = read(from_client, buffer, sizeof(buffer));
      if (b == -1) {
        printf("communication error %d: %s\n", errno, strerror(errno));
        exit(1);
      }
      // printf("[subserver] received from client: -%s-\n", buffer);
      while (1) {
        int r = rand() % HANDSHAKE_BUFFER_SIZE;
        sprintf(buffer, "%d", r);
        sleep(1);
        b = write(to_client, buffer, sizeof(buffer));
        if (b == -1) {
          printf("communication error %d: %s\n", errno, strerror(errno));
          exit(1);
        }
      }
    }
  }
  remove(WKP);
  // printf("[server] removes WKP\n");
  server_setup();
  return from_client;
}

/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  int b, to_client;
  char buffer[HANDSHAKE_BUFFER_SIZE];

  b = read(from_client, buffer, sizeof(buffer));
  // printf("[subserver] handshake received: -%s-\n", buffer);

  to_client = open(buffer, O_WRONLY, 0);
  //create SYN_ACK message
  srand(time(NULL));
  int r = rand() % HANDSHAKE_BUFFER_SIZE;
  sprintf(buffer, "%d", r);
  write(to_client, buffer, sizeof(buffer));

  //read and check ACK
  read(from_client, buffer, sizeof(buffer));
  int ra = atoi(buffer);
  if (ra != r+1) {
    printf("[subserver] handshake received bad ACK: -%s-\n", buffer);
    exit(0);
  }//bad response
  // printf("[subserver] handshake received: -%s-\n", buffer);

  return to_client;
}

/*=========================
  client_handshake
  args: int * to_server
  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.
  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[HANDSHAKE_BUFFER_SIZE];
  char ppname[HANDSHAKE_BUFFER_SIZE];

  //make private pipe
  // printf("[client] handshake: making pp\n");
  sprintf(ppname, "%d", getpid() );
  mkfifo(ppname, 0600);

  //send pp name to server
  // printf("[client] handshake: connecting to wkp\n");
  *to_server = open( WKP, O_WRONLY, 0);
  if ( *to_server == -1 ) {
    printf("open error %d: %s\n", errno, strerror(errno));
    exit(1);
  }

  write(*to_server, ppname, sizeof(buffer));
  //open and wait for connection
  from_server = open(ppname, O_RDONLY, 0);

  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  // printf("[client] handshake: received -%s-\n", buffer);

  //remove pp
  remove(ppname);
  // printf("[client] handshake: removed pp\n");

  //send ACK to server
  int r = atoi(buffer) + 1;
  sprintf(buffer, "%d", r);
  write(*to_server, buffer, sizeof(buffer));

  return from_server;
}
