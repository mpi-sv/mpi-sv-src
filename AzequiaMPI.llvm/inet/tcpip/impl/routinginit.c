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
#include <routinginit.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#else
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <pmi.h>

/* Debug pourpose only */
#ifdef __DEBUG_MALLOC
extern  void  *INET_MALLOC            (unsigned int size);
extern  void   INET_FREE              (void *ptr);
#else
#define INET_MALLOC malloc
#define INET_FREE   free
#endif

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define MAX_CAD               256

/*----------------------------------------------------------------*
 *   Definition of global variables                               *
 *----------------------------------------------------------------*/
static int              mycpu;
static int              mchns_nr;

/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/
/**
 *  RI_init():
 *   Open configuration file and read some information (mycpu and number of nodes)
 */
int RI_init(int listenPort) {

  char   cad [MAX_CAD];
  struct hostent *host;
  struct in_addr ipaddr;
  int    i, spawned, err, name_max, key_maxlen, val_maxlen;
  char  *kvsname = NULL, *key = NULL, *val = NULL;

  /* Initialise PMI *
  if (PMI_SUCCESS != (err = PMI_Init(&spawned))) {
		fprintf(stderr, "PMI_Init failed with error = %d\n", err);
		goto exception;
  } */

  /* Get the total number of nodes */
  /*  if (PMI_SUCCESS != (err = PMI_Get_universe_size(&mchns_nr))) {
   fprintf(stderr, "PMI_Get_size failed with error = %d\n", err);
   goto exception;
   }*/ 
  if (PMI_SUCCESS != (err = PMI_Get_size(&mchns_nr))) {
	fprintf(stderr, "PMI_Get_size failed with error = %d\n", err);
	goto exception;
  } 
  
  /* Get my node identifier */
  if (PMI_SUCCESS != (err = PMI_Get_rank(&mycpu))) {
		fprintf(stderr, "PMI_Get_Rank failed with error = %d\n", err);
		goto exception;
  }

  /* Prepare to store items in the PMI distributed database */
  if (PMI_SUCCESS != (err = PMI_KVS_Get_name_length_max(&name_max))) {
		fprintf(stderr, "PMI_KVS_Get_name_length_max failed with error = %d\n", err);
    goto exception;
  }

  if (NULL == (kvsname = (char *) INET_MALLOC(name_max)))                       goto exception;

  if (PMI_SUCCESS != (err = PMI_KVS_Get_my_name(kvsname, name_max))) {
    fprintf(stderr, "PMI_KVS_Get_my_name failed with error = %d\n", err);
    goto exception;
  }

  if (PMI_SUCCESS != (err = PMI_KVS_Get_key_length_max(&key_maxlen))) {
		fprintf(stderr, "PMI_KVS_Get_key_length_max failed with error = %d\n", err);
    goto exception;
  }

  if (NULL == (key = (char *) INET_MALLOC(key_maxlen)))                         goto exception;

  if (PMI_SUCCESS != (err = PMI_KVS_Get_value_length_max(&val_maxlen))) {
		fprintf(stderr, "PMI_KVS_Get_value_length_max failed with error = %d\n", err);
    goto exception;
  }

  if (NULL == (val = (char *) INET_MALLOC(val_maxlen)))                         goto exception;

  /* Store key #1. Given a machine number, obtain remainder info of node */
  if (0 > gethostname(cad, MAX_CAD))                                            goto exception;
  if (NULL == (host = gethostbyname(cad)))                                      goto exception;
  bcopy(host->h_addr_list[0], (char *)&ipaddr, sizeof(ipaddr));

  sprintf(key, "%s#%d", kvsname, mycpu);
  sprintf(val, "%d#%d", ipaddr, listenPort);

  if (PMI_SUCCESS != (err = PMI_KVS_Put(kvsname, key, val))) {
		fprintf(stderr, "PMI_KVS_Put failed with error = %d\n", err);
    goto exception;
  }

  /* Sincronize all nodes now */
  if (PMI_SUCCESS != (err = PMI_KVS_Commit(kvsname))) {
		fprintf(stderr, "PMI_KVS_Commit failed with error = %d\n", err);
    goto exception;
  }
  
  if (PMI_SUCCESS != (err = PMI_Barrier())) {
		fprintf(stderr, "PMI_Barrier failed with error = %d\n", err);
    goto exception;
  }

	/* Release resources */
  if (val)
		INET_FREE(val);

  if (key)
		INET_FREE(key);

  if (kvsname)
		INET_FREE(kvsname);

  return 0;

exception:
  if (val)
		INET_FREE(val);

  if (key)
		INET_FREE(key);

  if (kvsname)
		INET_FREE(kvsname);

  perror("RI_init");
  return -1;
}


/**
 *  RI_getCpuCount():
 *   Return the number of nodes previously read
 */
int RI_getCpuCount() {

#ifdef __DEBUG
  fprintf(stdout, "Node number: %d \n", mchns_nr);
#endif

  return mchns_nr;
}


/**
 *  RI_getCpuId():
 *   Return my node number
 */
int RI_getCpuId() {

  return mycpu;

#ifdef __DEBUG
  fprintf(stdout, "My CPU id: %d\n", mycpu);
#endif
}

/**
 *  RI_destroy():
 *   Close the routing information library
 */
int RI_destroy() {
  //PMI_Finalize();
  return 0;
}

