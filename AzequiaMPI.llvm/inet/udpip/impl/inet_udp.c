/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <inet.h>
#include <inet_udp.h>

#if defined(__OSI)
  #include <osi.h>
#else
  #include <limits.h>
  #include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>

#include <routinginit.h>

extern  void panic           (char *where);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#ifndef TRUE
#define FALSE ((int)0)
#define TRUE  ((int)1)
#endif

#define XPN_print(excpn)  \
                          { \
                            if(excpn < 8) \
                              printf("\t\t\t\t>>> Exception %s raised in %s \n", e_names[-(excpn)], where); \
                          }

#define ETH_FRAME_SIZE    1500
#define IP_HDR_SIZE         20
#define TCP_HDR_SIZE         8
#define INET_FRAME_SIZE   (ETH_FRAME_SIZE - IP_HDR_SIZE - 8)

#define INET_HDR_SIZE       sizeof(INET_header)
#define INET_PLD_SIZE       (INET_FRAME_SIZE - INET_HDR_SIZE)

/* LNK Task priority. The maximum priority */
#define INET_PRIORITY        (sched_get_priority_max(SCHED_FIFO))

#define CBUFFER_MAX       64
#define CBUFFER_SIZE      (CBUFFER_MAX * INET_FRAME_SIZE)

#define MAX_NODES         1024

/* Local messages can be routed throught the network o shared memory copied. The most
   efficient is to copy messages sended to the same machine
 */
#define ROUTE_LOCAL_MSG       1

/*----------------------------------------------------------------*
 *   Definition of private data types                             *
 *----------------------------------------------------------------*/
#define UDP_PORT       0x1234
#define DEVICES_NR     1
typedef unsigned short udpPort_t;

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
/* Header used by NET layer */
struct INET_header {
  unsigned int     DstMchn;     /* Destination machine address */
  unsigned int     SrcMchn;     /* Source machine address */
  Protocol_t       Protocol;    /* Protocol for installing different upcalls */
  unsigned short   Mode;        /* Control flags for different types of messages */
  int              FrgmtSize;   /* Fragment size */
};
typedef struct INET_header INET_header, *INET_header_t;

struct DEV {
  pthread_t           Task;
  volatile char       CyclicBuffer[CBUFFER_SIZE];
  char               *InPtr;
  int                 Cnt;
  int                 Nr;
  unsigned int        Addr;
  char               *NextFrame;     /* Next frame to get */
  char               *BufferLimit;   /* Limit of my cyclic buffer. For NextFrame treatment */
  pthread_mutex_t     SendMutex;
  int                 RecvSocket;    /* UDP socket for receiving packets */
  struct sockaddr_in  RecvAddr;
  int                 SendSocket;    /* UDP socket for sending   packets */
  struct sockaddr_in  SendAddr;
};
typedef struct DEV DEV, *DEV_t;

/*----------------------------------------------------------------*
 *   Definition of public data                                    *
 *----------------------------------------------------------------*/
static DEV   DEV_handler[DEVICES_NR];
static int (*deliver)(DEV_iovec *vector, int last_frgmt, int *success);
static int   DEV_initialized = 0;

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static char *e_names[8] = {  /*  0 */ "INET_E_OK",
                             /*  1 */ "INET_E_EXHAUST",
                             /*  2 */ "INET_E_INTEGRITY",
                             /*  3 */ "INET_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "INET_E_INTERFACE",       // with INET_mcbsp.h
                             /*  5 */ "INET_E_SYSTEM",
                             /*  6 */ "INET_E_DISABLED",
                             /*  7 */ "INET_E_DEADPART"
                          };

static int (*deliverUpcall[PROTOCOL_MAX])(INET_iovec *iov, int last_frgmt, int *success);

/* This node address */
static Mchn_t cpuId;

/* Number of nodes */
static int    nodesNr;

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
static int   DEV_config   (int   devNr, int port, int prio);
static void  DEV_unconfig (int   devNr, int port);
static void *DEV_scan     (void *params);

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef __DEBUG
static void printHdr(INET_header_t hdr) {

  printf("NET header |   SrcMchn    DstMchn    Protocol       Mode   FrgmtSize\n");
  printf("           |   -------    -------    --------       ----   ---------\n");
  printf("           |%10d %10d %10x %10d %10d\n", hdr->SrcMchn, hdr->DstMchn, hdr->Protocol, hdr->Mode, hdr->FrgmtSize);

  return;
}
#endif



     /*----------------------------------------------------------*\
    |    DEV_scan                                                  |
    |                                                              |
    |    A task waiting for new packets                            |
    |                                                              |
     \*----------------------------------------------------------*/
static void *DEV_scan(void *devNrPtr) {

  struct sched_param  schparam;
  int                 policy;
  int                 success;
  DEV_iovec           payload;
  int                 bytes;
  int                 excpn      = INET_E_SYSTEM;
  int                 devNr      = *((int *)devNrPtr);
  DEV_t               dev        = &DEV_handler[devNr];
  static char        *where      = "DEV_scan";

#ifdef __DEBUG
  printf("DEV_scan[%d] (LNK task):\n", devNr);
#endif

  if(pthread_getschedparam(pthread_self(), &policy, &schparam))                goto exception;

  while (1) {
#ifdef __DEBUG
    printf("DEV_scan:Receiving (Cnt = %d)...\n", dev->Cnt);
#endif
    /* Wait for an incoming message */
    if(dev->Cnt == CBUFFER_MAX)                                                panic("DEV_scan: Cyclic Buffer Overrun");

    if (0 > (bytes = recvfrom(dev->RecvSocket, dev->InPtr,
                                        INET_FRAME_SIZE, MSG_WAITALL, NULL, NULL)))       goto exception;

#ifdef __DEBUG
    printf("\nDEV_scan:Received %d bytes in a %d fragment. (Cnt = %d)\n", ((INET_header_t)(dev->InPtr))->FrgmtSize, bytes, dev->Cnt);
#endif

    if (bytes == 0)                                                            goto exception;

    /* Encapsulate the message in LNK_iovec structure */
    payload.Begin = dev->InPtr + sizeof(INET_header);
    //payload.Size  = bytes      - sizeof(INET_header);
    payload.Size  = ((INET_header_t)(dev->InPtr))->FrgmtSize;

    /* Deliver the message */
    if (0 > deliver(&payload, ((INET_header_t)(dev->InPtr))->Mode & INET_LAST_FRAGMENT, &success)) {
      fprintf(stdout, ">>> Exception WANDERING MESSAGE raised in DEV_scan\n");
      pthread_exit(NULL);
    }

    /* Increment the pointer */
    if(!success) {
      dev->Cnt += 1;
      dev->InPtr += INET_FRAME_SIZE;
      if (dev->InPtr == (dev->CyclicBuffer + CBUFFER_SIZE))
        dev->InPtr = (char *)dev->CyclicBuffer;
    }
  }

exception:
  close(dev->RecvSocket);
  XPN_print(excpn);
  pthread_exit(NULL);
}


     /*----------------------------------------------------------*\
    |    DEV_config                                                |
    |                                                              |
    |      A communication device configuration                    |
     \*----------------------------------------------------------*/
static int DEV_config(int devNr, int port, int prio) {

  pthread_t           task;
  pthread_attr_t      attrs;
  int                 error;
  int                 excpn     = INET_E_SYSTEM;
  DEV_t               dev       = &DEV_handler[devNr];
  struct sched_param  schparam;

#ifdef __DEBUG
  printf("DEV_config[Dev %d][Port %d][Prio %d]:\n", devNr, port, prio);
#endif

  /* This device type, number and address */
  dev->Nr    = devNr;
  dev->InPtr = (char *)&dev->CyclicBuffer;
  dev->Cnt   = 0;

  /* Mutual exclusion access to this dev */
  if (pthread_mutex_init(&(dev->SendMutex), NULL))                             goto exception;

  /* Device UDP sockets */
  if (0 > (dev->SendSocket  = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)))       goto exception;
  /* Build the (machine,port) address of the remote server socket */
  bzero((char *)&dev->SendAddr, sizeof(dev->SendAddr));
  dev->SendAddr.sin_family  = AF_INET;
  dev->SendAddr.sin_port    = htons(port);

  if (0 > (dev->RecvSocket = socket(AF_INET, SOCK_DGRAM, 0)))                  goto exception;
  bzero((char *)&dev->RecvAddr, sizeof(dev->RecvAddr));
  dev->RecvAddr.sin_family      = AF_INET;
  dev->RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  dev->RecvAddr.sin_port        = htons(port);
  if (0 > bind(dev->RecvSocket, (struct sockaddr *)&dev->RecvAddr,
                                 sizeof(dev->RecvAddr)))                       goto exception;

  /* Attributes of the device internal thread */
  if (pthread_attr_init(&attrs))                                               goto exception;
  if (pthread_attr_setstacksize(&attrs, PTHREAD_STACK_MIN))                    goto exception;
  if (pthread_attr_setinheritsched(&attrs, PTHREAD_EXPLICIT_SCHED))            goto exception;
  if (pthread_attr_setschedpolicy(&attrs, SCHED_FIFO))                         goto exception;
  pthread_attr_getschedparam(&attrs, &schparam);
  schparam.sched_priority = prio;
  if (pthread_attr_setschedparam(&attrs, &schparam))                           goto exception;

  /* Device internal thread */
  if ((error = pthread_create(&dev->Task, &attrs, DEV_scan, &dev->Nr))) {
        if(EPERM == error) {
#ifdef _DEBUG
          printf("DEV_config: Under Linux, setting a scheduling SCHED_FIFO or SCHED_RR policy requires superuser permission.\n\t \
                  The execution will proceed with the default SCHED_OTHER policy\n");
#endif
          pthread_attr_setschedpolicy(&attrs, SCHED_OTHER);
          pthread_attr_getschedparam (&attrs, &schparam);
          schparam.sched_priority = 0;
          pthread_attr_setschedparam (&attrs, &schparam);
          if(pthread_create(&task, &attrs, DEV_scan, &dev->Nr))                goto exception;
        }
        else goto exception;
  }
#ifdef __DEBUG
  printf("DEV_config: End\n");
#endif
  return INET_E_OK;

exception:
  close(dev->RecvSocket);
  close(dev->SendSocket);
  fprintf(stdout, "\t\t\t\t>>> Exception %s raised in %s\n", e_names[-(excpn)], "DEV_config");
  return(excpn);
}


     /*----------------------------------------------------------*\
    |    DEV_unconfig                                              |
    |                                                              |
     \*----------------------------------------------------------*/
static void DEV_unconfig(int devNr, int port) {

  DEV_t dev = &DEV_handler[devNr];

  bzero((char *)&dev->SendAddr, sizeof(dev->SendAddr));
  dev->SendAddr.sin_family      = AF_INET;
  dev->SendAddr.sin_port        = htons(port);
  sendto(dev->SendSocket, NULL, 0, 0, (struct sockaddr *)&dev->SendAddr,
                                      (socklen_t)(sizeof(dev->SendAddr)));
  close(dev->RecvSocket);
  close(dev->SendSocket);
  pthread_kill(dev->Task, 1);
  return;
}


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

     /*----------------------------------------------------------*\
    |*   DEV_init                                                 *|
    |*                                                            *|
    |*                                                            *|
     \*----------------------------------------------------------*/
int DEV_init(int port, int prio) {

  int excpn;

  if(!DEV_initialized) {

#ifdef __DEBUG
    fprintf(stdout, "DEV_init: LNK level using UDP (User Datagram Protocol)\n");
#endif

    if(0 > (excpn = DEV_config(0, port, prio)))                                goto exception;
    DEV_initialized = 1;
  }

  return(INET_E_OK);

exception:
  fprintf(stdout, "\t\t\t\t>>> Exception %s raised in %s\n", e_names[-excpn], "DEV_init");
  return(excpn);
}


     /*----------------------------------------------------------*\
    |*   DEV_terminate                                            *|
    |*                                                            *|
    |*                                                            *|
     \*----------------------------------------------------------*/
void DEV_terminate(int port) {
  DEV_unconfig(0, port);
}


     /*----------------------------------------------------------*\
    |    DEV_install                                               |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
void DEV_install (int (*upcall)(DEV_iovec *iov, int last_frgmt, int *success)) {
  deliver = upcall;
}


     /*----------------------------------------------------------*\
    |*   DEV_send                                                 *|
    |*                                                            *|
    |*                                                            *|
     \*----------------------------------------------------------*/
int DEV_send(int ipAddr, char *frame) {

  struct msghdr msg;
  struct iovec  iov;
  int           bytes;
  int           excpn;
  DEV_t         dev     = &DEV_handler[0];

#ifdef __DEBUG
  printf("DEV_send: to ip Address %x\n", ipAddr);
#endif

  if(pthread_mutex_lock(&dev->SendMutex))                                       {excpn = INET_E_SYSTEM;
                                                                                goto exception;}
  /* Build the (machine,port) address of the remote server socket */
  dev->SendAddr.sin_addr.s_addr = ipAddr;

  iov.iov_base = frame;
  iov.iov_len  = INET_FRAME_SIZE;

  msg.msg_name    = &dev->SendAddr;
  msg.msg_namelen = sizeof(dev->SendAddr);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;

  if (0 > (bytes = sendmsg(dev->SendSocket, &msg, 0)))                          {excpn = INET_E_SYSTEM;
                                                                                goto exception;}

  if(pthread_mutex_unlock(&dev->SendMutex))                                     {excpn = INET_E_SYSTEM;
                                                                                goto exception;}
#ifdef __DEBUG
  printf("DEV_send:End\n");
#endif

  return(INET_E_OK);

exception:
  perror("\t\t\t\t>>> ");
  fprintf(stdout, "\t\t\t\t>>> Exception %s raised in %s\n", e_names[-INET_E_SYSTEM], "DEV_send");
  return(INET_E_SYSTEM);
}


     /*----------------------------------------------------------*\
    |*   DEV_recvStart                                            *|
    |*                                                            *|
    |*                                                            *|
     \*----------------------------------------------------------*/
int DEV_recvStart(int devNr) {
  fprintf(stdout, "\t\t\t\t>>> Exception %s raised in %s\n", e_names[-INET_E_INTERFACE], "LNK_SDB_recvStart");
  return INET_E_INTERFACE;
}


     /*----------------------------------------------------------*\
    |    DEV_recv                                                   |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
void DEV_recv(void) {

  DEV_t   dev = &DEV_handler[0];

  dev->Cnt--;
}


/*----------------------------------------------------------------*
 *   Implementation of public function interface                  *
 *----------------------------------------------------------------*/

     /*----------------------------------------------------------------*\
    |    INET_init                                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o param  (Input)                                                |
    |        Either                                                      |
    |        UDP Port where this virtual processor listen, or            |
    |        IP address of this machine                                  |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |    <  0 : On other case                                            |
    |                                                                    |
     \*----------------------------------------------------------------*/
int INET_init(void *params) {

  udpPort_t   udpPort;
  int         excpn    = INET_E_SYSTEM;
  char       *where    = "INET_init";
  RI_RoutingTableEntry *sendRoute;

#ifdef __DEBUG
  fprintf(stdout, "INET_init: using UDP (User Datagram Protocol). Port: %d\n", UDP_PORT);
#endif

  /* 1. Get configuration setup for the NET:
    - Number of processors in the whole system
		- This Cpu address
		- Routing table for sending message from this node
   */
  if (0 > RI_init())                                                           goto exception;

  nodesNr = RI_getCpuCount();
  cpuId   = RI_getCpuId();
  if ((nodesNr == 0) || (nodesNr > MAX_NODES))                                 goto exception;
  if (NULL == (sendRoute = malloc(nodesNr * sizeof(int))))                     goto exception;
  RI_getRoutingTable(sendRoute);

  /*if (NULL == (MchnTable = (MchnInfo *) malloc (nodesNr * sizeof(MchnInfo))))  goto exception;

  for (i = 0; i < nodesNr; i++) {
    MchnTable[i].NodeAddr = (unsigned int)sendRoute[i];
  }*/

#if (ROUTE_LOCAL_MSG == 0)
    MchnTable[i].NodeAddr = NO_NET;
#ifdef __DEBUG
    fprintf(stdout, "Mchn: 0x%x is the local machine. Using memcpy\n", IPtable[mycpu]);
#endif
#endif

  free(sendRoute);
  RI_destroy();


  udpPort = UDP_PORT;
  if (0 > DEV_init(udpPort, INET_PRIORITY))                                     goto exception;

#ifdef __DEBUG
  printf("INET_init(UDP): Port %d\n", udpPort);
#endif

  return INET_E_OK;

exception:
  XPN_print(excpn);
  return excpn;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_send                                                       |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o iov:                                                        |
    |        IO vector from upper level with 2 positions                 |
    |      o dstMchn:                                                    |
    |        Destination machine                                         |
    |      o protocol:                                                   |
    |        Protocol fro sending the message                            |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |    <  0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_send (INET_iovec *net_iov, Mchn_t dstMchn, Protocol_t protocol) {

  int          size, excpn;
  char        *source;
  char        *destination;
  char         packet[INET_FRAME_SIZE];
  INET_header  hdr  = {INET_PLD_SIZE, 0};

  int dev_nr = 0;

#ifdef __DEBUG
  printf("INET_send: to machine %d\n", dstMchn);
#endif

  size    =  net_iov[2].Size;
  source  = &net_iov[2].Data[0];
  do {
    /* LNK header */
    destination  = packet;
    if ((net_iov[0].Size + net_iov[1].Size  + size) <= INET_PLD_SIZE) {
      hdr.FrgmtSize = net_iov[0].Size + net_iov[1].Size + size;
      hdr.Mode      = INET_LAST_FRAGMENT;
    }

#ifdef __DEBUG
    printHdr(&hdr);
#endif

    /* LNK header */
    memcpy(destination, &hdr, sizeof(INET_header));
    /* NET header*/
    destination += sizeof(INET_header);
    memcpy(destination, &net_iov[0].Data[0], net_iov[0].Size);
    /* AZQ header */
    destination += net_iov[0].Size;
    memcpy(destination, &net_iov[1].Data[0], net_iov[1].Size);
    /* AZQ payload */
    destination += net_iov[1].Size;
    memcpy(destination, source, hdr.FrgmtSize - net_iov[0].Size - net_iov[1].Size);

    source += (hdr.FrgmtSize - net_iov[0].Size - net_iov[1].Size);

    if(0 > (excpn = DEV_send(dev_nr, packet)))                                 goto exception;

    size -= (INET_PLD_SIZE - net_iov[0].Size - net_iov[1].Size);

  } while (size > 0);

#ifdef __DEBUG
  printf("INET_put: End\n");
#endif

  return(INET_E_OK);

exception:
  fprintf(stdout, "\t\t\t\t>>> Exception %s raised in %s\n", e_names[-excpn], "INET_put");
  return(excpn);
}


     /*----------------------------------------------------------*\
    |    INET_install                                               |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
int INET_subscribe (int (*upcall)(INET_iovec *iov, int last_frgmt, int *success), Protocol_t protocol) {

  if((protocol >= PROTOCOL_MAX))    return INET_E_INTERFACE;

  deliverUpcall[protocol] = upcall;

  return INET_E_OK;
}


     /*----------------------------------------------------------*\
    |    INET_finalize                                              |
    |                                                              |
    |    Shutdown the Azequia Link Layer.                          |
    |                                                              |
    |    PARAMETERS:                                               |
    |      o none                                                  |
    |                                                              |
    |    RETURN:                                                   |
    |      o none                                                  |
    |                                                              |
     \*----------------------------------------------------------*/

void INET_finalize (void) {
  DEV_terminate(UDP_PORT);
  return;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_by                                                         |
    |                                                                    |
    |    Do we need to send a message throught the net?                  |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o dstMchn:                                                    |
    |        Destination machine                                         |
    |                                                                    |
    |    RETURN:                                                         |
    |      o TRUE or FALSE                                               |
    |        Yes or not.                                                 |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_by (Mchn_t dstMchn) {
  //if(NO_NET == MchnTable[(int)dstMchn].NodeAddr)  return FALSE;
  //return TRUE;

  return TRUE;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_recv                                                       |
    |                                                                    |
    |    Receive a message from a machine                                |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o srcMchn:                                                    |
    |        Destination machine                                         |
    |                                                                    |
    |    RETURN:                                                         |
    |      o None                                                        |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
void INET_recv(Mchn_t srcMchn) {

  /*unsigned int dev_nr  = MchnTable[(int)srcMchn].NodeAddr;

  MchnInfo_t   dev = &MchnTable[0];
  */
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_getCpuId                                                   |
    |                                                                    |
    |    Returns this node address                                       |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o None                                                        |
    |                                                                    |
    |    RETURN:                                                         |
    |      o Node address                                                |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
Mchn_t INET_getCpuId (void) {
  return cpuId;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_broadcast                                                  |
    |                                                                    |
    |    Broadcast message                                               |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o iov                                                         |
    |      o protocol                                                    |
    |                                                                    |
    |    RETURN:                                                         |
    |      = 0 on success                                                |
    |      < 0 in other case                                             |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_broadcast (INET_iovec *iov, Protocol_t protocol) {

  int           excpn;
  Mchn_t        dstMchn;
  unsigned int  dev_nr;
  INET_header       hdr;
  int           success;


  for (dstMchn = 0; dstMchn < nodesNr; dstMchn++) {
    /*dev_nr = sendRoute[(int)dstMchn];
    setHeader(&hdr, dstMchn, protocol, INET_REMOTE | INET_BROADCAST);
	  if (0 > (excpn = miniSend(&hdr, iov, count + 1, dev_nr)))                  goto exception;*/
  }


  return INET_E_OK;

exception:
  fprintf(stdout, "\n\t\t\t>>> Exception %s raised in %s\r", e_names[-(excpn)], "INET_broadcast");
  return excpn;
}


