/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <inet.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of public data                                    *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private data types                             *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static char *e_names[8] = {  /*  0 */ "INET_E_OK",
                             /*  1 */ "INET_E_EXHAUST",
                             /*  2 */ "INET_E_INTEGRITY",
                             /*  3 */ "INET_E_TIMEOUT",    /* This order has to be consistent with net.h */
                             /*  4 */ "INET_E_INTERFACE",          
                             /*  5 */ "INET_E_SYSTEM",
                             /*  6 */ "INET_E_OTHER",
                             /*  7 */ "INET_E_OTHER",
                           };

static int Initialized = 0;

/*----------------------------------------------------------------*
 *   Implementation of public function interface                  *
 *----------------------------------------------------------------*/

int INET_init (void *param) {

  if (Initialized)  return INET_E_OK;
  Initialized = 1;
  
#ifdef _DEBUG
  fprintf(stdout, "NO NET level linked. Only shared memory");
#endif

  return(INET_E_OK);
}

void INET_finalize (void) {
  return;
}


int INET_subscribe (int (*upcall)(INET_iovec *iov, int last_frgmt, int *success), Protocol_t protocol) {
  return INET_E_OK;
}


int INET_send (INET_iovec *iov, Mchn_t dstMchn, Protocol_t protocol) {

  char *where = "INET_NOINET_send";

  return INET_E_INTEGRITY;
}

int INET_broadcast (INET_iovec *iov, Protocol_t protocol) {

  char *where = "INET_NOINET_broadcast";

  return INET_E_INTEGRITY;
}


void INET_recv(char *packet) {

  char *where = "INET_NOINET_recv";

  return ;
}


int INET_by (Mchn_t dstMchn) {

  char *where = "INET_NOINET_INET_by";

  return 0;
}


Mchn_t INET_getCpuId (void) {

  char *where = "INET_NOINET_getcpuid";

  return 0;
}

int INET_getNodes (void) {
  /* Must return 1 node. Using shared memory for communicating threads */
  char *where = "INET_NOINET_getNodes";

  return 1;
}

void INET_start (void) {
	return;
}

