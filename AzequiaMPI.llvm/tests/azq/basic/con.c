#include <limits.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <pthread.h>


#define NUM_MSGS    100
#define PORT        4790
#define FRAME_SIZE  1024

struct Header {
  int  NumMsg;
  int  BodySize;
  int  Flags;
};
typedef struct Header Header, *Header_t;

static void *DEV_deliver (void *p) {

  int                 size;
  int                *buftmp;
  int                 sendSocket; 
  struct sockaddr_in  sendAddr;
  int                 sendbuf;
  int                 i = 0;
  Header              hdr;
  int                 r;


  printf("DEV_DELIVER\n");
  usleep(1000);

  if (0 > (sendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))            {printf("S - ERR socket()\n"); goto exception;}
  
  /*sendbuf = 4;
  if (0 > setsockopt(sendSocket, IPPROTO_TCP, TCP_NODELAY, &sendbuf, sizeof(sendbuf)))
    goto exception;
  */
  bzero((char *)&sendAddr, sizeof(sendAddr));
  sendAddr.sin_family       = AF_INET;
  sendAddr.sin_addr.s_addr  = inet_addr("127.0.0.1");
  sendAddr.sin_port         = htons(PORT);

  //if (0 > bind(sendSocket, (struct sockaddr *)&sendAddr, sizeof(struct sockaddr)))    {printf("S - ERR bind()\n"); goto exception;}

  if (0 > connect(sendSocket, (struct sockaddr *)&sendAddr, sizeof(sendAddr))) {printf("S - ERR connect()\n"); goto exception;}

  if (NULL == (buftmp = (int *) malloc (FRAME_SIZE))) goto exception;

  srand(54326);

  while (1) {

    r = rand() % (FRAME_SIZE - sizeof(Header));
    hdr.NumMsg = i;
    hdr.BodySize = r;
    

    i++;
    if (i == 1000) hdr.Flags = 1;
    else           hdr.Flags = 0;

    memcpy(buftmp, (char *)&hdr, sizeof(Header));
    // Faltaría copiar los datos
    printf("Enviando mensaje %d de tamaño %d\n", hdr.NumMsg, hdr.BodySize);

    size = write(sendSocket, (char *)buftmp, hdr.BodySize + sizeof(Header));

    if (i == 1000) break;
  }

  printf("Closing send socket\n");
  shutdown(sendSocket, 0);
  close (sendSocket);

  free(buftmp);

  return;

exception:
  printf("ERROR (DEV_DELIVER)\n");
  shutdown(sendSocket, 0);
  close (sendSocket);
  pthread_exit(NULL);
}


int readpacket (int sockfd, char *buf, int len) {

  int nLeft = len;
  int iPos  = 0;
  int nData = 0;

  do {
    if (0 > (nData = read (sockfd, &buf[iPos], nLeft))) return 0;
    nLeft -= nData;
    iPos  += nData;
  } while(nLeft > 0);

  return iPos;
}


int main (int argc, char *argv[]) {

  pthread_t           taskdeliver;
  int                 recvSocket;
  struct sockaddr_in  recvAddr;
  int                 sendbuf;
  int                 sockfd;
  struct sockaddr_in  peerAddr;
  unsigned int        addrlen;
  int                 buftmp[FRAME_SIZE];
  int                 size;
  int                 i;
  Header              hdr;
  int                 bodysz;


  printf("MAIN\n");

  /* RECV Socket */
  if (0 > (recvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))            {printf("R - ERR socket()\n"); goto exception;}
  bzero((char *)&recvAddr, sizeof(recvAddr));
  recvAddr.sin_family      = AF_INET;
  recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //htonl(INADDR_ANY);
  recvAddr.sin_port        = htons(PORT);
  if (0 > bind(recvSocket, (struct sockaddr *)&recvAddr, sizeof(struct sockaddr)))    {printf("R - ERR bind()\n"); goto exception;}
  listen(recvSocket, 5);

  pthread_create(&taskdeliver, NULL, DEV_deliver, NULL);

  addrlen = sizeof(peerAddr);
  if (0 > (sockfd = accept(recvSocket, (struct sockaddr *)&peerAddr, &addrlen)))
    fprintf(stderr, "DEV_scan: Accepting error (%d)...\n", recvSocket);

  i = 0;
  while (1) {

    //printf("Recibiendo mensaje ...\n");

    //usleep(1000);
    size = readpacket (sockfd, (char *)&hdr, sizeof(Header));
    if (size != sizeof(Header)) {
      fprintf(stderr, "ERROR: Received HEADER of size  %d \n ", size);
      break;
    }

    bodysz = hdr.BodySize;
    size = readpacket (sockfd, (char *)buftmp, bodysz);
    //if (0 > (size = recvfrom(sockfd, (char *)buftmp, FRAME_SIZE, 0, NULL, NULL)))       goto exception;

    if (size != bodysz) {
      fprintf(stderr, "ERROR: Received MESSAGE of size %d\n", size);
      break;
    }

    printf("Recibido mensaje de SIZE: %d  NUM: %d\n", bodysz, hdr.NumMsg);

    if (hdr.Flags != 0) break;
  }
 
  pthread_join(taskdeliver, NULL);

  printf("Closing recv socket\n");
  shutdown(recvSocket, 0);
  close(sockfd);
  close(recvSocket);

  return EXIT_SUCCESS;

exception:
  printf("ERROR MAIN\n");
  return EXIT_FAILURE;
}
