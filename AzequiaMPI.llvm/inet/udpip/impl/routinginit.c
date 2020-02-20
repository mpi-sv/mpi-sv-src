#include <routinginit.h>

#ifdef __OSI
#include <osi.h>
#else
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#endif

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define MAX_CAD     256

/*----------------------------------------------------------------*
 *   Definition of global variables                               *
 *----------------------------------------------------------------*/
static FILE          *fm;
static int            mycpu;
static int            mchns_nr;
       unsigned int  *IPtable;


/*----------------------------------------------------------------*
 *   Definition of public interface                               *
 *----------------------------------------------------------------*/
/**
 *  RI_init():
 *   Open configuration file and read some information (mycpu and number of nodes)
 */
int RI_init() {

//  char  *env;
  char   cad [MAX_CAD];
  int    i;
//  char   file[MAX_CAD];

/*  env = getenv("HOME");
  strcpy(file, env);
  strcat(file, "/.netcfg.azq");
*/

//  strcpy(file, "./netcfg.azq");

#ifdef __DEBUG
//  fprintf(stdout, "Home folder:  %s\n", env);
//  fprintf(stdout, "File to read: %s\n", file);
#endif

  if (NULL == (fm = fopen("./netcfg.azq", "r")))  goto exception;

#ifdef __DEBUG
  fprintf(stdout, "Reading file %s for network machine addresses ...\n", "./netcfg.azq");
#endif

  fgets(cad, MAX_CAD, fm);
  if (feof(fm)) goto exception;
  mchns_nr = atoi(cad);

#ifdef __DEBUG
  fprintf(stdout, "Machine number: %d\n", mchns_nr);
#endif

  if (NULL == (IPtable = (unsigned int *) malloc (mchns_nr * sizeof(unsigned int))))
    goto exception_2;


  for (i = 0; i < mchns_nr; i++) {

    fgets(cad, MAX_CAD, fm);
    if (feof(fm)) goto exception;

    IPtable[i] = inet_addr(cad);

#ifdef __DEBUG
    fprintf(stdout, "Mchn: %s  IP: 0x%x\n", cad, IPtable[i]);
    fprintf(stdout, "Filling table 0x%x \n", IPtable);
#endif

  }

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
 *  RI_getRoutingTable():
 *   Read from file the routing information and return them
 */
int RI_getRoutingTable(RI_RoutingTableEntry *RoutingTable) {

  int i;

  for (i = 0; i < mchns_nr; i++)
    RoutingTable[i] = IPtable[i];

  return 0;

exception:
  perror(".netcfg.azq can not be read properly");
  return -1;
}


/**
 *  RI_destroy():
 *   Close the routing information library
 */
int RI_destroy() {
  free(IPtable);
  fclose(fm);
  return 0;
}

