/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <inet.h>
#include <pmi.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <limits.h>
  #include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
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
#include <sys/signal.h>
#include <time.h>

#include <routinginit.h>
#include <elb.h>
#include <com.h>

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#ifndef TRUE
#define FALSE             ((int)0)
#define TRUE              ((int)1)
#endif

/* Debug messages */
#ifdef __INET_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/* Tries in connecting */
#define  TRIES                 10
#define  TRY_PERIOD       3000000

/* Broadcast destination machine */
#define BCAST_MCHN        (Mchn_t)0xFFFF

/* Exceptions information */
#define XPN_print(excpn)  \
                          { \
                            if(excpn < 8) \
                              fprintf(stderr, "\t\t\t[Node %d] >>> Exception %s\t\t\t\t Raised in %s \n", cpuId, e_names[-(excpn)], where); \
                          }

/* Payload carried by this level */
#define INET_PLD_SIZE     (INET_FRAME_SIZE - sizeof(INET_header))

/* NET Task priority. The maximum priority */
#define INET_PRIORITY     (sched_get_priority_max(SCHED_FIFO))

/* Port to be used by default */
#define TCP_PORT           0x4412

/* INET maximum frame size for fragmenting messages. Calculated to be a multiple of 8 */
#define INET_FRAME_SIZE   ((64 * 1024) + sizeof(INET_header))

/* Incoming cyclic bufer. By now, all receivers share the buffer */
#define CBUFFER_MAX       (512)
#define CBUFFER_SIZE      (CBUFFER_MAX * INET_FRAME_SIZE)

/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* Header used by NET layer */
struct INET_header {
#ifdef __INET_DEBUG
  unsigned short      DstMchn;     /* Destination machine address */
  unsigned short      SrcMchn;     /* Source machine address */
#endif
  Protocol_t          Protocol;    /* Protocol for installing different upcalls */
  unsigned short      Mode;        /* Control flags for different types of messages */
  int                 FrgmtSize;   /* Fragment size */
};
typedef struct INET_header INET_header, *INET_header_t;

/* Machine info stored in a table from 0 to machine_number - 1 */
struct MchnInfo {
  int                 Connected;
  int                 Activated;   /* In use */
  struct in_addr      NodeAddr;    /* IP address of node i */
  int                 RecvSocket;  /* Socket for receiving from node i */
  int                 SendSocket;  /* Socket for sending to node i */
  pthread_mutex_t     SendMutex;   /* Mutex for serializing sends throught send socket */
  pthread_t           ScanTask;    /* Thread reading data from this node */
};
typedef struct MchnInfo MchnInfo, *MchnInfo_t;

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
 /* Error codes */
static char *e_names[8] = {  /*  0 */ "INET_E_OK",
                             /*  1 */ "INET_E_EXHAUST",
                             /*  2 */ "INET_E_INTEGRITY",
                             /*  3 */ "INET_E_TIMEOUT",         // This order has to be consistent
                             /*  4 */ "INET_E_INTERFACE",       // with net.h
                             /*  5 */ "INET_E_SYSTEM",
                             /*  6 */ "INET_E_OTHER",
                             /*  7 */ "INET_E_OTHER",
                          };

/* This node address */
static Mchn_t        cpuId;

/* Number of nodes */
static int           nodesNr;

/* Table of machine info */
static MchnInfo         *MchnTable;
static pthread_mutex_t   MchnTableLock;

/* Type for deliver upcall function into upper levels */
static int         (*deliverUpcall[PROTOCOL_MAX])(INET_iovec *iov, int last_frgmt, int *success);

/* Scan and deliver tasks */
static pthread_t     deliver_task;
static pthread_t     accept_task;

/* Return error from scan and deliver threads */
static int           deliver_ret,
                     accept_ret,
                     scan_ret;

/* Queue for buffering management of incoming messages and deliver */
static ELB_t         elb;

/* Flag for initialization */
static int           initialized = FALSE;
static int           system_ready = 0;

/*----------------------------------------------------------------*
 *   Declaration of private and extern functions                  *
 *----------------------------------------------------------------*/
extern         void   panic        (char *where);

/* Debug pourpose only */
#ifdef __DEBUG_MALLOC
void  *INET_MALLOC                 (unsigned int size);
void   INET_FREE                   (void *ptr);
#else
#define INET_MALLOC malloc
#define INET_FREE   free
#endif

#ifdef __INET_DEBUG
static         void   printHdr     (INET_header_t hdr);
static         void   printSConns  ();
static         void   printRConns  ();
#endif

static inline  int    readpacket   (int sockfd, char *buf, int len);
static inline  int    writevector  (int sockfd, struct iovec *vector, int count);
static         void  *accept_body  (void *params);
static         int    connect_node (MchnInfo_t minfo, int dstMchn);
static         int    launch_task  (pthread_t *task, int prio, void *(*fxn) (void *), void *params);
static         void  *scan         (void *params);
static         void  *inet_deliver (void *params);

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef __DEBUG_MALLOC

static int totsize = 0;

void *INET_MALLOC (unsigned int size) {

  void *ptr;

  ptr = malloc(size);
  totsize += size;
  fprintf(stdout, "[INET_TCP] Reserved %d (acum: %d) bytes at %p\n", size, totsize, ptr);

  return (ptr);
}

void INET_FREE (void *ptr) {
  fprintf(stdout, "[INET_TCP] Freeing memory at %p\n", ptr);
  free(ptr);
}

#endif


#ifdef __INET_DEBUG
static void printHdr(INET_header_t hdr) {

  fprintf(stdout, "NET header |   SrcMchn    DstMchn    Protocol       Mode   FrgmtSize\n");
  fprintf(stdout, "           |   -------    -------    --------       ----   ---------\n");
  fprintf(stdout, "           |%10d %10d %10x %10d %10d\n", hdr->SrcMchn, hdr->DstMchn, hdr->Protocol, hdr->Mode, hdr->FrgmtSize);

  return;
}

static void printSConns () {

  int         i;
  MchnInfo_t  minfo;

  for (i = 0; i < nodesNr; i++) {
    minfo = &MchnTable[i];
    fprintf(stdout, "[%d] To SEND to Machine %d with IP %x use Socket %d\n", cpuId, i, minfo->NodeAddr.s_addr, minfo->SendSocket);
  }

}

static void printRConns () {

  int         i;
  MchnInfo_t  minfo;

  for (i = 0; i < nodesNr; i++) {
    minfo = &MchnTable[i];
    fprintf(stdout, "[%d] To RECEIVE from Machine %d with IP %x use Socket %d\n", cpuId, i, minfo->NodeAddr.s_addr, minfo->RecvSocket);
  }

}

#define setHeader(hdr, dstMchn, protocol, mode)    \
        {                                          \
          (hdr)->DstMchn       = dstMchn;          \
          (hdr)->SrcMchn       = cpuId;            \
          (hdr)->Protocol      = protocol;         \
          (hdr)->Mode          = mode;             \
        };

#endif



#ifndef __INET_DEBUG
/* Version for non debug execution */
#define setHeader(hdr, dstMchn, protocol, mode)    \
        {                                          \
          (hdr)->Protocol      = protocol;         \
          (hdr)->Mode          = mode;             \
        };

#endif



     /*----------------------------------------------------------*\
    |    readpacket                                                |
    |                                                              |
    |    Read a complete packet                                    |
    |                                                              |
     \*----------------------------------------------------------*/
static inline int readpacket (int sockfd, char *buf, int len) {

  register int nLeft = len;
  register int nData;

  while (nLeft > 0) {

    do {
      nData = read (sockfd, buf, nLeft);
    } while ((nData == -1) && (errno == EINTR));

    if (nData == -1) {
      DBG_PRNT((stdout, "[readpacket] Error reading packet (nData: %d  nLeft: %d  errno: %d)\n", nData, nLeft, errno));
      return -1;
    }

    nLeft -= nData;
    buf   += nData;

  }

  return len;
}


     /*----------------------------------------------------------*\
    |    writevector                                               |
    |                                                              |
    |    Write a IOV to a socket                                   |
    |                                                              |
     \*----------------------------------------------------------*/
static inline int writevector(int sockfd, struct iovec *vector, int count) {

           int  len = 0;
           int  i;
  register int  sended;
	register int  iov_offset;

  for (i = 0; i < count; i++)
    len += vector[i].iov_len;
  iov_offset = 0;

  do {
    sended = writev(sockfd, vector, count);
  } while ((sended == -1) && (errno == EINTR));

  if (sended == -1) {
    fprintf(stdout, "[writevector] ERROR. Sended %d errno %d\n", sended, errno);fflush(stdout);
    return -1;
  }

  len -= sended;
  while (len) {

    DBG_PRNT((stdout, "[writevector] Sended %d of %d\n", sended, len + sended));
    fprintf(stdout, "[writevector] Sended %d of %d\n", sended, len + sended);

    while (sended >= vector[iov_offset].iov_len) {
      sended -= vector[iov_offset].iov_len;
      iov_offset++;
    }

    vector[iov_offset].iov_base += sended;
    vector[iov_offset].iov_len  -= sended;

    do {
      sended = writev(sockfd, &vector[iov_offset], count - iov_offset);
      fprintf(stdout, "\t  Sended the rest: %d (vectors: %d)\n", sended, count - iov_offset);
    } while ((sended == -1) && (errno == EINTR));

    if (sended == -1) {
      fprintf(stdout, "[writevector] ERROR. Resended %d errno %d\n", sended, errno);fflush(stdout);
      return -1;
    }

    len -= sended;
  }

  return 0;
}


     /*----------------------------------------------------------*\
    |    accept_body                                               |
    |                                                              |
    |    Task managing all incoming connections                    |
    |                                                              |
     \*----------------------------------------------------------*/
static void *accept_body (void *params) {

  int                 excpn      = INET_E_SYSTEM;
  static char        *where      = "accept_body";
  struct sockaddr_in  peerAddr;
  unsigned int        addrlen;
  int                 sockfd;
  MchnInfo_t          minfo;
  int                 i;
  struct sockaddr_in  recvaddr;
  int                 on = 1;
  int                 sockaux;
  
  int    err;
  int    name_max, id_maxlen, key_maxlen, val_maxlen, spawned;
  char  *kvsname, *id, *domain_id, *key, *val;
  

  DBG_PRNT((stdout, "accept_body(). Init:\n"));
  
  if (pthread_mutex_lock(&MchnTableLock))                                      {excpn = INET_E_SYSTEM;
	goto exception;}
  
   
  /* Get the key/value for hostname */
  if (PMI_SUCCESS != (err = PMI_KVS_Get_name_length_max(&name_max))) {
	fprintf(stderr, "PMI_KVS_Get_name_length_max failed with error = %d\n", err);
	goto exception;
  }
  
  kvsname = (char *) malloc( name_max );
  if (PMI_SUCCESS != (err = PMI_KVS_Get_my_name(kvsname, name_max))) {
	fprintf(stderr, "PMI_KVS_Get_my_name failed with error = %d\n", err);
	goto exception;
  }
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get_key_length_max(&key_maxlen))) {
	fprintf(stderr, "PMI_KVS_Get_key_length_max failed with error = %d\n", err);
	goto exception;
  }
  
  key = (char *) malloc(key_maxlen);
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get_value_length_max(&val_maxlen))) {
	fprintf(stderr, "PMI_KVS_Get_value_length_max failed with error = %d\n", err);
	goto exception;
  }
  
  val = (char *) malloc(val_maxlen);

  if (pthread_mutex_unlock(&MchnTableLock))                                      {excpn = INET_E_SYSTEM;
	goto exception;}
  
  
  /* 1. A socket is created to listen all incoming connections */
  if (0 > (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))                goto exception;

  if (0 > setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))       goto exception;

  bzero((char *)&recvaddr, sizeof(recvaddr));
  recvaddr.sin_family      = AF_INET;
  recvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  recvaddr.sin_port        = htons(TCP_PORT);

  if (0 > bind(sockfd, (struct sockaddr *)&recvaddr, sizeof(recvaddr)))        goto exception;

  listen(sockfd, 10);

  /* 2. Wait for incoming connections */
  while (1) {

    addrlen = sizeof(peerAddr);

    if (0 > (sockaux = accept(sockfd, (struct sockaddr *)&peerAddr, &addrlen)))
                                                                               goto exception;
    /* 2.1. Find the address in the machine table and launch a thread for
            managing communication with the node */
    //for (i = 0; i < nodesNr; i++) {

  
	if (pthread_mutex_lock(&MchnTableLock))                                      {excpn = INET_E_SYSTEM;
	  goto exception;}
	
	  /* Host number */
	  sprintf(key, "%s_%s_%s", kvsname, "hostnr", inet_ntoa(peerAddr.sin_addr));
	  //fprintf(stdout, "PMI_KVS_Get - hostnr: getting  %s\n", key);
	  //sprintf(val, "%s", inet_ntoa(ipaddr));
	  
	  if (PMI_SUCCESS != (err = PMI_KVS_Get(kvsname, key, val, val_maxlen))) {
		fprintf(stderr, "accept _ PMI_KVS_Get failed with error = %d  (%s  %s  %s  %d)\n", err, kvsname, key, val, val_maxlen);
		  goto exception;
	  }
	
	if (pthread_mutex_unlock(&MchnTableLock))                                      {excpn = INET_E_SYSTEM;
	  goto exception;}  
	  
	i = atoi(val);
	  
	  minfo = &MchnTable[i];
	  
      //if (minfo->Activated) continue;
	if (!minfo->Activated) {
	  
      //if (minfo->NodeAddr.s_addr == peerAddr.sin_addr.s_addr) {
	  
	  
	//fprintf(stdout, "PMI_KVS_Get(%s) HOSTNR returned %d \n", key, atoi(val));
	//fflush(stdout);
	  //fprintf(stdout, "Accepted from  %d  in %d\n ", atoi(val), cpuId);
	  fflush(stdout);

	
        minfo->Activated  = TRUE;

        minfo->RecvSocket = sockaux;
        if (0 > setsockopt(minfo->RecvSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
                                                                               goto exception;

        if (0 > launch_task(&minfo->ScanTask, INET_PRIORITY, scan, minfo))     goto exception;

        //break;

      //}

    //}
	}
#ifdef __INET_DEBUG
    printRConns();
#endif

  }

  DBG_PRNT((stdout, "accept_body(). End.\n"));

  free(val);
  free(key);
  free(kvsname);
    
  accept_ret = 0;
  return (&accept_ret);

exception:
  XPN_print(excpn);
  accept_ret = -1;
  return (&accept_ret);
}



     /*----------------------------------------------------------*\
    |    connect_node                                              |
    |                                                              |
    |    Connect to a node                                         |
    |                                                              |
     \*----------------------------------------------------------*/
static int connect_node (MchnInfo_t minfo, int dstMchn) {

  int                  on = 1;
  struct sockaddr_in   sendaddr;
  int                  tries = 0;
  int                  excpn;
  char                *where = "connect_node";

  int    err;
  int    name_max, id_maxlen, key_maxlen, val_maxlen, spawned;
  char  *kvsname, *id, *domain_id, *key, *val;
  

  DBG_PRNT((stdout, "Node %d trying to connect to node with IP Address  0x%x\n", cpuId, minfo->NodeAddr.s_addr));

  if (pthread_mutex_lock(&MchnTableLock))                                      {excpn = INET_E_SYSTEM;
                                                                                goto exception;}
  if (minfo->Connected) {
    if (pthread_mutex_unlock(&MchnTableLock))                                  {excpn = INET_E_SYSTEM;
                                                                                goto exception;}
    return INET_E_OK;
  }

  /* 1. Create the mutex for serializing sends */
  if (0 > pthread_mutex_init(&minfo->SendMutex, NULL))                         {excpn = INET_E_EXHAUST;
                                                                                goto exception;}

  /* 2. Create the socket to send messages to machine i */
  if (0 > (minfo->SendSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))     {excpn = INET_E_EXHAUST;
                                                                                goto exception;}

  if (0 > setsockopt(minfo->SendSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
                                                                               {excpn = INET_E_INTEGRITY;
                                                                                goto exception;}
  on = 1;
  if (0 > setsockopt(minfo->SendSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on)))
                                                                               {excpn = INET_E_INTEGRITY;
                                                                                goto exception;}
  bzero((char *)&sendaddr, sizeof(sendaddr));
  sendaddr.sin_family       = AF_INET;
  
/*  if (PMI_SUCCESS != (err = PMI_Init(&spawned))) {
	fprintf(stderr, "PMI_Init failed with error = %d\n", err);
	goto exception;
  } 
  */
  
  
  
  /* Get the key/value for hostname */
  if (PMI_SUCCESS != (err = PMI_KVS_Get_name_length_max(&name_max))) {
	fprintf(stderr, "PMI_KVS_Get_name_length_max failed with error = %d\n", err);
    goto exception;
  }
  
  if (NULL == (kvsname = (char *) malloc( name_max )))
	goto exception;
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get_my_name(kvsname, name_max))) {
    fprintf(stderr, "connect _ PMI_KVS_Get_my_name failed with error = %d  (%s  %d)\n", err, kvsname, name_max);
	  goto exception;
  }
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get_key_length_max(&key_maxlen))) {
	fprintf(stderr, "PMI_KVS_Get_key_length_max failed with error = %d\n", err);
    goto exception;
  }
  
  key = (char *) malloc(key_maxlen);
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get_value_length_max(&val_maxlen))) {
	fprintf(stderr, "PMI_KVS_Get_value_length_max failed with error = %d\n", err);
    goto exception;
  }
  
  val = (char *) malloc(val_maxlen);
  
  /* Host address */
  sprintf(key, "%s_%s_%d", kvsname, "hostaddr", dstMchn);
  //fprintf(stdout, "PMI_KVS_Get - hostaddr: getting  %s\n", key);
  //sprintf(val, "%s", inet_ntoa(ipaddr));
  
  if (PMI_SUCCESS != (err = PMI_KVS_Get(kvsname, key, val, val_maxlen))) {
    fprintf(stderr, "PMI_KVS_Get failed with error = %d  (%s  %s  %s  %d)\n", err, kvsname, key, val, val_maxlen);
    goto exception;
  }
  
  //fprintf(stdout, "PMI_KVS_Get(%s) HOSTADDR returned %s \n", key, val);
  //fprintf(stdout, "(%s) Trying to connecto to  %s (%d) from %d ... \n", key, val, dstMchn, cpuId);
  DBG_PRNT((stdout, "Node %d trying to connect to node with IP Address  0x%x\n", cpuId, inet_addr(val)));
  fflush(stdout);
  
   
  //sendaddr.sin_addr.s_addr  = minfo->NodeAddr.s_addr;
  sendaddr.sin_addr.s_addr  = inet_addr(val);
  sendaddr.sin_port         = htons(TCP_PORT);

  /* 3. Connect to machine i. Make a number of tries. */
  do {
    if (0 > connect(minfo->SendSocket, (struct sockaddr *)&sendaddr, sizeof(sendaddr))) {
      usleep(TRY_PERIOD);
      DBG_PRNT((stdout, "[INET_TCP] Retry: Node %d trying to connect to node %x\n", cpuId, (unsigned int)sendaddr.sin_addr.s_addr));
      if (tries++ == TRIES) {
        excpn = INET_E_TIMEOUT;
        goto exception;
      }
    } else {
      break;
    }
  } while (1);

  minfo->Connected = TRUE;
  if (pthread_mutex_unlock(&MchnTableLock))                                    {excpn = INET_E_SYSTEM;
                                                                                goto exception;}

  free(val);
  free(key);
  free(kvsname);
    
  return(INET_E_OK);

exception:
  if (-1 != minfo->SendSocket)
    close(minfo->SendSocket);
  pthread_mutex_destroy(&minfo->SendMutex);
  XPN_print(excpn);
  return(excpn);
}



     /*----------------------------------------------------------*\
    |*   launch_task                                                 *|
    |*                                                            *|
    |*                                                            *|
     \*----------------------------------------------------------*/
static int launch_task(pthread_t *task, int prio, void *(*fxn) (void *), void *params) {

  int                 excpn = INET_E_SYSTEM;
  static char        *where = "launch_task";
  pthread_attr_t      attrs;
  struct sched_param  schparam;
  int                 error;

  DBG_PRNT((stdout, "launch_task. Init:\n"));

  if (pthread_attr_init(&attrs))                                               goto exception;
  if (pthread_attr_setstacksize(&attrs, PTHREAD_STACK_MIN * 32))               goto exception;
  if (pthread_attr_setinheritsched(&attrs, PTHREAD_EXPLICIT_SCHED))            goto exception;
  if (pthread_attr_setschedpolicy(&attrs, SCHED_FIFO))                         goto exception;
  pthread_attr_getschedparam(&attrs, &schparam);
  schparam.sched_priority = prio;
  if (pthread_attr_setschedparam(&attrs, &schparam))                           goto exception;

  /* Device internal thread */
  if ((error = pthread_create(task, &attrs, fxn, params))) {
    if(EPERM == error) {

      DBG_PRNT((stdout, "DEV_config: Under Linux, setting a scheduling SCHED_FIFO or SCHED_RR policy requires superuser permission.\n\t \
                         The execution will proceed with the default SCHED_OTHER policy\n"));

      if (pthread_attr_setstacksize(&attrs, PTHREAD_STACK_MIN * 32))           goto exception;
      pthread_attr_setschedpolicy(&attrs, SCHED_OTHER);
      pthread_attr_getschedparam (&attrs, &schparam);
      schparam.sched_priority = 0;
      pthread_attr_setschedparam (&attrs, &schparam);
      if(pthread_create(task, &attrs, fxn, params))                            goto exception;

    } else goto exception;
  }

  DBG_PRNT((stdout, "launch_task. End.\n"));

  return(INET_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}


     /*----------------------------------------------------------*\
    |    scan                                                      |
    |                                                              |
    |    A task waiting for new packets froma node                 |
    |                                                              |
     \*----------------------------------------------------------*/
static void *scan (void *params) {

  int                 excpn    = INET_E_SYSTEM;
  static char        *where    = "scan";
  MchnInfo_t          minfo    = (MchnInfo_t)params;
  int                 size;
  INET_header        *hdr;
  char               *packet;
  Header             *azqHdr;

  DBG_PRNT((stdout, "scan(). Init. Node %d reading from node %x\n", cpuId, minfo->NodeAddr.s_addr));

  /* wait until system had been initialised */
  while (!system_ready) usleep(1000);

  while (1) {

    ELB_newElem(elb, (void *)&packet);

    /* 1. Messages are read in two calls. First retrieve the INET header, and later the data. The
          send is serialized in each machine, so the messages are expected to arrive adjoin */

    /** MODIFICATION -- Read the INET header + the Azequia header -- MODIFICATION **/
    if (0 > (size = readpacket(minfo->RecvSocket, (char *)&packet[0], sizeof(INET_header) + HEADER_NET_SZ)))
                                                                               {excpn = INET_E_INTEGRITY;
                                                                                goto exception;}
    hdr    = (INET_header *)packet;
    azqHdr = (Header *) (packet + sizeof(INET_header));

    if ((!(azqHdr->Mode & MODE_RRV_ACK)) && (azqHdr->RrvDstAddr != NULL)) {
      /* Special case. As we know the destination user buffer address, we can bypass the BC, copying data directly to the user buffer */
      if (0 > (size = readpacket(minfo->RecvSocket, azqHdr->RrvDstAddr, hdr->FrgmtSize - HEADER_NET_SZ)))
                                                                               goto exception;
      /* tell Azequia dont copy payload in RQST_fill */
      azqHdr->Mode |= MODE_NO_COPY;

    } else {
      /* Normal case */
      if (0 > (size = readpacket(minfo->RecvSocket, (char *)&packet[size], hdr->FrgmtSize - HEADER_NET_SZ)))
                                                                               goto exception;
    }

    /* 2. Enqueue the message in buffer */
    ELB_put(elb, packet);
  }

  DBG_PRNT((stdout, "scan(). End.\n"));

  return (NULL);

exception:
  fprintf(stdout, "ERROR (scan): size: %d  errno: %d\n", size, errno);fflush(stdout);
  XPN_print(excpn);
  scan_ret = -1;
  return (&scan_ret);
}



     /*----------------------------------------------------------*\
    |    deliver                                                   |
    |                                                              |
    |    Task doing delivering of message in cyclic buffer         |
    |                                                              |
     \*----------------------------------------------------------*/
static void *inet_deliver (void *params) {

  int                excpn    = INET_E_SYSTEM;
  static char       *where    = "deliver";
  int                success;
  INET_iovec         payload;
  Protocol_t         protocol;
  int                is_last_frgmt;
  INET_header       *packet;

  
  DBG_PRNT((stdout, "INET_deliver().  Init: \n"));

  while (1) {

    /* 1. Get the next message from the cyclic buffer */
	DBG_PRNT((stdout, "[%d] INET_deliver().  Getting..... \n", cpuId));
    ELB_get(elb, (void *)&packet);
    DBG_PRNT((stdout, "[%d] INET_deliver().  Got \n", cpuId));
	
    /* 2. Built a INET iov struct */
    payload.Data  = (char *)packet + sizeof(INET_header);
    payload.Size  = packet->FrgmtSize;

    /* 3. Deliver the message to the correct protocol upper level */
    protocol      = packet->Protocol;
    is_last_frgmt = packet->Mode & INET_LAST_FRAGMENT;

    DBG_PRNT((stdout, "[%d] INET: Delivering MSG from machine  %x  to protocol  %d  \n", cpuId,
                                ((INET_header *)payload.Data)->SrcMchn, protocol));

    /* 4. Deliver the message to upper level */
	if (0 > deliverUpcall[protocol](&payload, is_last_frgmt, &success))        goto exception;

    DBG_PRNT((stdout, "[%d] INET: OK to deliver message. Success: %d\n", cpuId, success));

    /* 5. If upper level copied the message, the packet is freed, else only a reference is in the
          upper level, so INET_recv will free the message later */
    if (success) {
      ELB_freeElem(elb, packet);
    }

  }

  DBG_PRNT((stdout, "INET deliver().  End. \n"));

  deliver_ret = 0;
  return (&deliver_ret);

exception:
  XPN_print(excpn);
  deliver_ret = -1;
  return (&deliver_ret);
}



/*----------------------------------------------------------------*
 *   Implementation of public function interface                  *
 *----------------------------------------------------------------*/

     /*__________________________________________________________________
    /                                                                    \
    |    INET_init                                                       |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |    <  0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_init(void *param) {

  int            excpn    = INET_E_SYSTEM;
  char          *where    = "INET_init";
  int            i;
  RI_RoutingTableEntry
                *sendRoute;


  if (initialized) return INET_E_OK;
  initialized = TRUE;

  DBG_PRNT((stdout, "INET_init: using TCP (Transmission Control Protocol). Port: %d\n", TCP_PORT));
  
#ifdef VERBOSE_MODE
  fprintf(stdout, "-------------------------------------------------------\n");
  fprintf(stdout, "\tAzequia INET (TCP)\n");
  fprintf(stdout, "\tFrame/payload size: %ld bytes / %ld bytes\n", INET_FRAME_SIZE, INET_PLD_SIZE);
  fprintf(stdout, "\tBuffering size: %ld Kbytes (chunks: %d)\n", CBUFFER_SIZE / 1024, CBUFFER_MAX);
  fprintf(stdout, "-------------------------------------------------------\n");
#endif

  /* 1. Get configuration setup for the network:
    - Number of processors in the whole system
		- This Cpu address and number
		- Routing table for sending message from this node
   */
  if (0 > RI_init())                                                           goto exception;

  nodesNr = RI_getCpuCount();
  cpuId   = RI_getCpuId();
  if ((nodesNr == 0) || (nodesNr > MAX_NODES))                                 goto exception;
  //if (NULL == (sendRoute = INET_MALLOC(nodesNr * sizeof(int))))                goto exception;
  //RI_getRoutingTable(sendRoute);

  /* 3. Build the machine info table */
  if (pthread_mutex_init(&MchnTableLock, NULL))                                goto exception;

  if (NULL == (MchnTable = (MchnInfo *) INET_MALLOC (nodesNr * sizeof(MchnInfo))))
                                                                               goto exception;
  for (i = 0; i < nodesNr; i++) {
    MchnTable[i].Connected       = FALSE;
    //MchnTable[i].NodeAddr.s_addr = sendRoute[i];
    MchnTable[i].Activated       = FALSE;
    MchnTable[i].SendSocket      = -1;
    MchnTable[i].RecvSocket      = -1;
  }

  DBG_PRNT((stdout, "CPU number: %d   CPU Address: 0x%x\n", cpuId, MchnTable[cpuId].NodeAddr.s_addr));

  //INET_FREE(sendRoute);
  //RI_destroy();

  /* 4. Create a cyclic queue for incoming message packets */
  if(0 > ELB_create(&elb, INET_FRAME_SIZE, CBUFFER_MAX, 1))                    goto exception2;

  /* 5. Launch deliver task. This thread delivers packet to upper levels */
  if (0 > launch_task(&deliver_task, INET_PRIORITY, inet_deliver, NULL))       goto exception3;

  /* 6. Launch accept task. This thread scans for incoming connections from
        the network, and launch tasks for receiving from each node */
  if (0 > launch_task(&accept_task,  INET_PRIORITY, accept_body, NULL))        goto exception3;

#ifdef __INET_DEBUG
  printSConns();
#endif

  sleep(2);
  
  return INET_E_OK;

exception3:
  ELB_destroy(elb);

exception2:
  INET_FREE(MchnTable);

exception:
  XPN_print(excpn);
  return excpn;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_finalize                                                   |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o None                                                        |
    |                                                                    |
    |    RETURN:                                                         |
    |      o None                                                        |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
void INET_finalize (void) {

  int           i;
  MchnInfo_t    minfo;

  DBG_PRNT((stdout, "INET_finalize: shuting down network in node  %d\n", cpuId));
  fprintf(stdout, "INET_finalize: shuting down network in node  %d\n", cpuId);

  /* 1. Terminate accept and deliver tasks */
  pthread_cancel(accept_task);
  pthread_join(accept_task, NULL);

  pthread_cancel(deliver_task);
  pthread_join(deliver_task, NULL);

  /* 2. Shutdown the machine info entries */
  for (i = 0; i < nodesNr; i++) {

    minfo = &MchnTable[i];

    if (minfo->Activated) {
      pthread_cancel(minfo->ScanTask);
      pthread_join(minfo->ScanTask, NULL);
    }

    if (minfo->RecvSocket != -1) {
      shutdown(minfo->RecvSocket, SHUT_RDWR);
      close(minfo->RecvSocket);
    }

    if (minfo->SendSocket != -1) {
      shutdown(minfo->SendSocket, SHUT_RDWR);
      close(minfo->SendSocket);
      pthread_mutex_destroy(&minfo->SendMutex);
    }

  }

  /* 3. Free machines info */
  if (MchnTable)
    INET_FREE(MchnTable);

  pthread_mutex_destroy(&MchnTableLock);

  /* 4. Free incoming messages queue */
  ELB_destroy(elb);
  
  RI_destroy();


  return;
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

  int            excpn;
  char          *where = "INET_send";
  int            payload_size;
  char          *payload;
  int            upper_header_size;
  INET_header    hdr;
  MchnInfo_t     minfo = &MchnTable[(int)dstMchn];
  struct iovec   iov[3];
  Header        *azqHdr = (Header *) net_iov[0].Data;

  DBG_PRNT((stdout, "[%d] INET_send: to machine number %d (IP Address: 0x%x)\n", cpuId, dstMchn, minfo->NodeAddr.s_addr));
  DBG_PRNT((stdout, "[%d] \t Size: %d  Socket  %d\n", cpuId, net_iov[1].Size, minfo->SendSocket));

  /* 1. If no connection to destination machine, try to do it now */
  if (!(minfo->Connected)) {
    if (0 > connect_node(minfo, (int)dstMchn))                                 {excpn = INET_E_SYSTEM;
                                                                                goto exception;}
  }

  /* 2. Set the fields in header */
  setHeader(&hdr, dstMchn, protocol, INET_REMOTE);

  /* 3. Set the iov struct. The second position is for upper level header. Only is copied ome time */
  upper_header_size = net_iov[0].Size;
  iov[1].iov_base = &net_iov[0].Data[0];
  iov[1].iov_len  = upper_header_size;

  payload_size =  net_iov[1].Size;
  payload      = &net_iov[1].Data[0];

  /* 4. Fragment the message if bigger than a maximum size defined */
  do {

    if ((upper_header_size + payload_size) <= INET_PLD_SIZE) {

      hdr.FrgmtSize  = upper_header_size + payload_size;
      hdr.Mode      |= INET_LAST_FRAGMENT;

    } else {

      hdr.FrgmtSize  =  INET_PLD_SIZE;
      hdr.Mode      &= ~INET_LAST_FRAGMENT;

    }

    /* 4.1. Third position is for upper level payload */
    iov[2].iov_base = payload;
    iov[2].iov_len  = hdr.FrgmtSize - upper_header_size;

    /* 4.2. First position is for INET header */
    iov[0].iov_base = &hdr;
    iov[0].iov_len  = sizeof(INET_header);

#ifdef __INET_DEBUG
    printHdr(&hdr);
#endif

    /* 4.3. Serialize the sends of each fragment */
    if(pthread_mutex_lock(&minfo->SendMutex))                                  {excpn = INET_E_INTEGRITY;
                                                                                goto exception;}

    if (0 > writevector(minfo->SendSocket, iov, 3))                            {excpn = INET_E_SYSTEM;
                                                                                pthread_mutex_unlock(&minfo->SendMutex);
                                                                                goto exception;}

    if(pthread_mutex_unlock(&minfo->SendMutex))                                {excpn = INET_E_INTEGRITY;
                                                                                goto exception;}

    /* 4.4. Update the destination user buffer address only when sending data using RRV protocol */
    if ((!(azqHdr->Mode & MODE_RRV_ACK)) && (azqHdr->RrvDstAddr != NULL))
      azqHdr->RrvDstAddr += hdr.FrgmtSize - upper_header_size;

    payload_size -= (INET_PLD_SIZE - upper_header_size);
    payload      += (hdr.FrgmtSize - upper_header_size);

  } while (payload_size > 0);

  DBG_PRNT((stdout, "INET_send: End\n"));

  return(INET_E_OK);

exception:
  XPN_print(excpn);
  INET_finalize();
  return(excpn);
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

/* Unimplemented by now */
  int            excpn = INET_E_SYSTEM;
  static char   *where = "INET_broadcast";
  Mchn_t         dstMchn;

  for (dstMchn = 0; dstMchn < nodesNr; dstMchn++) {
    goto exception;
  }

  return INET_E_OK;

exception:
  XPN_print(excpn);
  return excpn;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_subscribe                                                  |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o upcall:                                                     |
    |        Upcall function with 3 parameters:                          |
    |          - IO vector for delivering to upper level                 |
    |          - Flag TRUE if the packet is the last fragment of a       |
    |            message                                                 |
    |          - Success: a return parameter set to TRUE if the packet   |
    |            has been copied to upper level                          |
    |      o protocol:                                                   |
    |        Protocol throught it is received this packet                |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |     < 0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_subscribe (int (*upcall)(INET_iovec *iov, int last_frgmt, int *success), Protocol_t protocol) {

  if((protocol >= PROTOCOL_MAX))    return INET_E_INTERFACE;

  deliverUpcall[protocol] = upcall;

  return INET_E_OK;
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
  if (dstMchn != cpuId) return TRUE;
  return FALSE;
}


     /*__________________________________________________________________
    /                                                                    \
    |    INET_recv                                                       |
    |                                                                    |
    |    Receive a message from a machine                                |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o packet:                                                     |
    |        Pointer to packet already received (copied in upper level)  |
    |                                                                    |
    |    RETURN:                                                         |
    |      o None                                                        |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
void INET_recv(char *packet) {
  ELB_freeElem(elb, packet - sizeof(INET_header));
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
    |    INET_getNodes                                                   |
    |                                                                    |
    |    Returns the number of nodes                                     |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |      o None                                                        |
    |                                                                    |
    |    RETURN:                                                         |
    |      o Number of nodes                                             |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int INET_getNodes (void) {
  return nodesNr;
}


void INET_start (void) {
  system_ready = 1;
}
