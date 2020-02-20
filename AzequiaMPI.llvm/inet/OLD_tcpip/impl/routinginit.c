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
#include <pmi.h>

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

/* Local messages can be routed throught the network o shared memory copied. The most
   efficient is to copy messages sended to the same machine
 */
#define ROUTE_LOCAL_MSG       0

/*----------------------------------------------------------------*
 *   Definition of global variables                               *
 *----------------------------------------------------------------*/
static FILE            *fm;
static int              mycpu;
static int              mchns_nr;
       unsigned int    *IPtable;


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/
/**
 *  RI_init():
 *   Open configuration file and read some information (mycpu and number of nodes)
 */
int RI_init() {

  char   cad [MAX_CAD];
  int    i;
  struct hostent *host;
  struct in_addr ipaddr;
  int    err;
  int    univ_sz, rank;
  int    spawned;
  int    name_max, id_maxlen, key_maxlen, val_maxlen;
  char  *kvsname, *id, *domain_id, *key, *val;


  if (PMI_SUCCESS != (err = PMI_Init(&spawned))) {
	fprintf(stderr, "PMI_Init failed with error = %d\n", err);
	goto exception;
  } 
  
  if (PMI_SUCCESS != (err = PMI_Get_universe_size(&univ_sz))) {
	fprintf(stderr, "PMI_Get_size failed with error = %d\n", err);
	goto exception;
  } 

  if (PMI_SUCCESS != (err = PMI_Get_rank(&rank))) {
	fprintf(stderr, "PMI_Get_Rank failed with error = %d\n", err);
	goto exception;
  }
  
  /* In Azequia, only one process per machine is started, so the rank number is the
     same as the machine number, and size of group is the same as universe size */
  mchns_nr = univ_sz;
  mycpu    = rank;
  
 
  //fm = fopen("./netcfg.azq", "r");
  if (univ_sz == 1) {
  //if (NULL == (fm = fopen("./netcfg.azq", "r")))  {
    /* Not network configuration file exists. Set default values */
    //mchns_nr = 1;
    //mycpu    = 0;
    if (NULL == (IPtable = (unsigned int *) INET_MALLOC (mchns_nr * sizeof(struct in_addr))))
                                                                               goto exception;
    IPtable[0] = 0;
    return 0;
  }
  
#ifdef __DEBUG
  fprintf(stdout, "Reading file %s for network machine addresses ...\n", "./netcfg.azq");
#endif

  //if (NULL == fgets(cad, MAX_CAD, fm))            goto exception_2;
  //if (feof(fm)) goto exception;
  //mchns_nr = atoi(cad);

#ifdef __DEBUG
  fprintf(stdout, "\n\nMachine number: %d\n", mchns_nr);
#endif

  if (0 > gethostname(cad, MAX_CAD)) goto exception;
  if (NULL == (host = gethostbyname(cad))) goto exception;
  bcopy(host->h_addr_list[0], (char *)&ipaddr, sizeof(ipaddr));

#ifdef __DEBUG
  fprintf(stdout, "Machine name: %s\n", cad);
  fprintf(stdout, "\tAddress: %s (0x%x)\n", inet_ntoa(ipaddr), (int)ipaddr.s_addr);
#endif

  /* Set the key/value for hostname in mycpu */
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
  
  /* Host address */
  sprintf(key, "%s_%s_%d", kvsname, "hostaddr", mycpu);
  sprintf(val, "%s", inet_ntoa(ipaddr));
  //sprintf(val, "%s", ipaddr);
  
  if (PMI_SUCCESS != (err = PMI_KVS_Put(kvsname, key, val))) {
	fprintf(stderr, "PMI_KVS_Put failed with error = %d\n", err);
    goto exception;
  }
  
  fprintf(stdout, "PMI_KVS_Put - hostaddr %s / %s \n", key, val);fflush(stdout);

  /*
  if (PMI_SUCCESS != (err = PMI_KVS_Commit(kvsname))) {
	fprintf(stderr, "PMI_KVS_Commit failed with error = %d\n", err);
    goto exception;
  }
  
   
  free(val);
  free(key);
  key = (char *) malloc(key_maxlen);
  val = (char *) malloc(val_maxlen);
*/  
  
  /* Host address */
  sprintf(key, "%s_%s_%s", kvsname, "hostnr", inet_ntoa(ipaddr));
  sprintf(val, "%d", mycpu);
  //sprintf(val, "%s", ipaddr);
  
  if (PMI_SUCCESS != (err = PMI_KVS_Put(kvsname, key, val))) {
	fprintf(stderr, "PMI_KVS_Put failed with error = %d\n", err);
    goto exception;
  }
  
  fprintf(stdout, "PMI_KVS_Put - hostnr %s / %s \n", key, val);fflush(stdout);
  
  /* Port */
//  -Obtener el puerto-
/*  sprintf(key, "%s_%s_%d", kvsname, "port", mycpu);
  sprintf(val, "%s", port);
  
  if (PMI_SUCCESS != (err = PMI_KVS_Put(kvsname, key, val))) {
	fprintf(stderr, "PMI_KVS_Put failed with error = %d\n", err);
    goto exception;
  }*/

  
  if (PMI_SUCCESS != (err = PMI_KVS_Commit(kvsname))) {
	fprintf(stderr, "PMI_KVS_Commit failed with error = %d\n", err);
    goto exception;
  }
  
  if (PMI_SUCCESS != (err = PMI_Barrier())) {
	fprintf(stderr, "PMI_Barrier failed with error = %d\n", err);
    goto exception;
  }
    
  /* PRUEBA 
  sprintf(key, "%s_%s_%d", kvsname, "hostaddr", (rank + 1) % univ_sz);
  if (PMI_SUCCESS != (err = PMI_KVS_Get(kvsname, key, val, val_maxlen))) {
    fprintf(stderr, "PMI_KVS_Get failed with error = %d\n", err);
    goto exception;
  }
  fprintf(stdout, "PRUEBA  --  PMI_KVS_Get(%s) returned %s\n", key, val);
  */
  
  /* Allocate table */
  if (NULL == (IPtable = (unsigned int *) INET_MALLOC (mchns_nr * sizeof(struct in_addr))))
    goto exception_2;

/*
  for (i = 0; i < mchns_nr; i++) {

    if (NULL == fgets(cad, MAX_CAD, fm))          goto exception_2;
    if (feof(fm)) goto exception;

    IPtable[i] = inet_addr(cad);

    if (IPtable[i] == ipaddr.s_addr) {

#if (ROUTE_LOCAL_MSG == 0)
      IPtable[i] = 0;
#ifdef __DEBUG
      fprintf(stdout, "Mchn: %s is the local machine. NO routing local messages\n", inet_ntoa(ipaddr));
#endif
#endif
      //mycpu = i;
    }

  }
  */
  free(val);
  free(key);
  free(kvsname);

  return 0;

exception_2:
  fclose(fm);

exception:
  perror("./netcfg.azq can not be open properly");
  return -1;
}


/**
 *  RI_getCpuCount():
 *   Return the number of nodes previously read
 */
int RI_getCpuCount() {

#ifdef __INET_DEBUG
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

#ifdef __INET_DEBUG
  fprintf(stdout, "My CPU id: %d\n", mycpu);
#endif
}


/**
 *  RI_getRoutingTable():
 *   Read from file the routing information and return them
 */
int RI_getRoutingTable(RI_RoutingTableEntry *RoutingTable) {

  int i;

  for (i = 0; i < mchns_nr; i++)
    RoutingTable[i] = IPtable[i];

  return 0;
}


/**
 *  RI_destroy():
 *   Close the routing information library
 */
int RI_destroy() {
  
  INET_FREE(IPtable);
  //if (fm) fclose(fm);
  
  fprintf(stdout, "RI_destroy:  CPU id: %d\n", mycpu);fflush(stdout);
  
  PMI_Finalize();
  
  return 0;
}

