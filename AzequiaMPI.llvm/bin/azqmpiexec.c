/*  _______________________________________________________________________
   |                                                                       |
   |  Azequia Message Passing Interface   ( AzequiaMPI )                   |
   |                                                                       |
   |  Authors: GIM. Media Engineering Group                                |
   |           http://gim.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           juancarl@unex.es                                            |
   |                                                                       |
   |  Date:    July 06, 2010                                               |
   |                                                                       |
   |  Description:  $ azqmpiexec -n 16 ./executable                        |
   |                Wrapper to mpiexec                                     |
   |_______________________________________________________________________| */


  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <limits.h>
#include <sched.h>
#include <unistd.h>
#include <hwloc.h>
#include <sys/time.h> 

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define __DEBUG


      /*________________________________________________________________
     /                                                                  \
    |    get_puPerHost                                                   |
    |                                                                    |
    |    Get the number of logical processors in each host of the ring   |
    |    An homgeneous cluster is assumed                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int get_puPerHost()
{
  int               depth;
  int               puPerHostNr;
  hwloc_topology_t  topology;
  
  hwloc_topology_init(&topology);
  hwloc_topology_load(topology);
  depth       = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_CORE);
  puPerHostNr = hwloc_get_nbobjs_by_depth    (topology, depth);
  hwloc_topology_destroy(topology);
  return puPerHostNr;
}


      /*________________________________________________________________
     /                                                                  \
    |    get_mpiNodeNr                                                   |
    |                                                                    |
    |    Get the number of MPI nodes, the <-n nodes> parameter           |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int get_mpiNodeNr(int argc, char *argv[])
{
  int  i;
 
  for (i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-n")) {
      if (argv[i + 1] == NULL)
        return -1;
      return atoi(argv[i + 1]); 
    }
  }
  return -1;
}

 
      /*________________________________________________________________
     /                                                                  \
    |    get_perHostParam                                                |
    |                                                                    |
    |    Get the parameter <-perhost P> of the command line              |
    |                                                                    |
    |    Returns 0 if not present                                        |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int get_perHostParam(int argc, char *argv[])
{
  int i;
 
  for (i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-perhost")) {
      if (argv[i + 1] == NULL)
        return -1;
      return atoi(argv[i + 1]); 
    }
  }
  return 0;
}


      /*________________________________________________________________
     /                                                                  \
    |    get_hostsInRing                                                 |
    |                                                                    |
    |    Get the number of hosts that participate in the MPD ring        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o hostsInRing      (Output)                                     |
    |        Number ofo hosts in the ring                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int get_hostsInRing(int *hostsInRing)
{
  struct timeval  time;
  int             randomize;
  char            mandato[64];
  char            fichero[16];
  FILE           *fd;
  int             ringHostNr = 0;

  strcpy(mandato, "mpdtrace | wc -l >> ");
  gettimeofday(&time, 0);
  srand((int)time.tv_usec);
  randomize=rand()%1000000;
  sprintf(fichero, "%d", randomize); 
  strcat(mandato, fichero);  
  if (0 > system(mandato))                                                      goto exception;
  if(NULL == (fd = fopen(fichero, "r")))                                        goto exception;
  if(0 >  fscanf(fd, "%d", &ringHostNr))                                        goto exception;
  fclose(fd); 
  unlink(fichero);
  if(ringHostNr <= 0)                                                           goto exception;
#ifdef __DEBUG
  printf("%s:main>> %d hosts in the ring\n", __FILE__, ringHostNr);    
#endif
  *hostsInRing = ringHostNr;
  return 0;
exception:
  return -1;
}



      /*________________________________________________________________
     /                                                                  \
    |    main                                                            |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define AZQ_LEN 64
int main(int argc, char *argv[]) {
  
  int    i, inew, incr;
  int    perHostParam;
  int    mpiNodeNr, mpiNodeNrIdx = -1, perHostNrIdx = -1;
  char **newArgv;
  int    ringHostNr = 0;


#ifdef __DEBUG
  {
  int i;
  printf("%s:main>> Import the environment: \n", __FILE__);    
  for (i = 0; i < argc; i++)
    printf("%s:main>> argv[%d] = %s\n", __FILE__, i, argv[i]); 
  }
#endif

  /* 1. Get the number of hosts in the MPD ring */
  if(0 > get_hostsInRing(&ringHostNr))                                          goto exception;

  /* 2. Get the -perhost parameter from command line */
  if(0 > (perHostParam = get_perHostParam(argc, argv)))                         goto exception;
  if(perHostParam == 0) perHostParam = get_puPerHost();
  
  /* 3. Build the entries of the new argv to pass to mpiexec */
  incr = (perHostParam ? 5 : 0);
  if(NULL == (newArgv = (char **)malloc((1+argc+6+incr) * sizeof(char *))))     goto exception;

  /* 3.1 Change "azqmpiexec" by "mpiexec" */
  if(NULL == (newArgv[0] = (char *)malloc(1+strlen("mpiexec"))))                goto exception;
  strcpy(newArgv[0], "mpiexec");

  /* 3.2 Insert the -genv parameters */
  /*     "AZQMPI_NODES" environ variable */
  if(NULL == (newArgv[1] = (char *)malloc(1+strlen("-genv"))))                  goto exception;
  strcpy(newArgv[1], "-genv");
  if(NULL == (newArgv[2] = (char *)malloc(1+strlen("AZQMPI_NODES"))))           goto exception;
  strcpy(newArgv[2], "AZQMPI_NODES");
  if(NULL == (newArgv[3] = (char *)malloc(AZQ_LEN)))                            goto exception;
  if(0 > (mpiNodeNr   = get_mpiNodeNr(argc, argv)))                             goto exception;
  sprintf(newArgv[3], "%d", mpiNodeNr); 


  /*     "AZQMPI_HOSTS" environ variable */
  if(NULL == (newArgv[4] = (char *)malloc(1+strlen("-genv"))))                  goto exception;
  strcpy(newArgv[4], "-genv");
  if(NULL == (newArgv[5] = (char *)malloc(1+strlen("AZQMPI_HOSTS"))))           goto exception;
  strcpy(newArgv[5], "AZQMPI_HOSTS");
  if(NULL == (newArgv[6] = (char *)malloc(AZQ_LEN)))                            goto exception;
  sprintf(newArgv[6], "%d", ringHostNr); 

  /*     "AZQMPI_PERHOST" environ variable */
  if(NULL == (newArgv[7] = (char *)malloc(1+strlen("-genv"))))                  goto exception;
  strcpy(newArgv[7], "-genv");
  if(NULL == (newArgv[8] = (char *)malloc(1+strlen("AZQMPI_PERHOST"))))         goto exception;
  strcpy(newArgv[8], "AZQMPI_PERHOST");
  if(NULL == (newArgv[9] = (char *)malloc(AZQ_LEN)))                            goto exception;
  sprintf(newArgv[9], "%d", perHostParam); 
  
  /*     -perhost must be always 1, because -n is number of machine */
  if(NULL == (newArgv[10] = (char *)malloc(AZQ_LEN)))                           goto exception;
  sprintf(newArgv[10], "%s", "-perhost"); 
  if(NULL == (newArgv[11] = (char *)malloc(AZQ_LEN)))                           goto exception;
  sprintf(newArgv[11], "%d", 1);
	
  
  /* 3.3 Run through the old argv and populate the new one */
  for (i = 1, inew = 1+6+incr; i < argc; i++, inew++) {
    if(NULL == (newArgv[inew] = (char *)malloc(1+strlen(argv[i]))))             goto exception;
    strcpy(newArgv[inew], argv[i]);

    /* Do not include the -perhost parameter */
    if (!strcmp(argv[i], "-perhost"))
      perHostNrIdx = i + 1;
    if(i == perHostNrIdx)
      inew -= 2;

    /* Change number of MPI nodes by number of AZEQUIA container processes */
    if(!strcmp(argv[i], "-n")) 
      mpiNodeNrIdx = i + 1;
    if(i == mpiNodeNrIdx)
      sprintf(newArgv[inew], "%d", ringHostNr); 
  }
  newArgv[inew] = NULL;

#ifdef __DEBUG
  {
  int i;
  printf("%s:main>> Export the environment: \n", __FILE__);    
  for (i = 0; i < inew; i++)
    printf("%s:main>> newArgv[%d] = %s\n", __FILE__, i, newArgv[i]); 
  }
#endif

#ifdef VERBOSE_MORE
  for (i = 0; i < inew; i++)
    fprintf(stdout, "%s ", newArgv[i]); 
  fprintf(stdout, "\n");
  fflush(stdout);
#endif  

  /* 4. Call mpiexec */
  if(0 > (execvp(newArgv[0], newArgv)))                                         goto exception;
  return 0;

exception:
  printf("%s : main : Exception \n", __FILE__);
  perror("");
  return (1);
}



