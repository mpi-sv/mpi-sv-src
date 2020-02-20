/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <azq.h>
#include <malloc.h>
#include <time.h>
#include "echo_msg.h"


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/



     /*----------------------------------------------------------*\
    |    echo_client                                               |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
#define ITERA   (5)
#define SEND_TIMEOUT    1
int echo_client(int *param)
{
  int          bufSize     = param[0] * sizeof(int);
  int          excpn       = 0;
  int          i, j,
               grpSize,
               gix         = getGroup();
  int          myRank      = getRank();
  char        *cadena;
  Thr_t        me          = THR_self();
  struct
  sched_param  schparam;
  int          ret         = 0;
  Hdr          hdr;
  Echo_Msg     *msg        = (Echo_Msg *)&hdr.Store;


  GRP_getSize(gix, &grpSize);
  if((1) != grpSize) {
      fprintf(stdout, "echo_client(%x), (Gix %x, Rank %d) pth %x, Bad Parameters\n", (unsigned int)me, gix, myRank, (int)me->Self);
      return(-1);
  }
  fprintf(stdout, "***** Echo_client(%x), (Gix %x, Rank %d) pth %x\n", (unsigned int)me, gix, myRank, (int)me->Self);

  /* Get the memory for the data to be sent */
  if(NULL == (cadena = (char *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Operator: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }
  strcpy(cadena, "HOLA ECOOOOOOOO");

  for(i = 0; i < ITERA; i++) {
    sleep(1);
#ifdef __ECHO_DEBUG
    fprintf(stdout, "\n\t^------------------------\n");
    fprintf(stdout, "\tECHO_echo (stub): begin %x \n", (unsigned int)(self()));
#endif

  /*  Build and send the GRP_JOIN request to the target machine */
    hdr.Port.Port = ECHO_PORT;
    hdr.Port.Mchn = DFLT_MCHN;
    hdr.Mode      = RPC_FULL;
    msg->Type     = ECHO_ECHO;
    if(0 > (excpn = RPC_trans(&hdr, (void *)cadena, 1+strlen(cadena), 
                                    (void *) NULL,  0)))                        goto exception;
    if(0 > (excpn = msg->Body.ReplyEchoEcho.Reply))                             goto exception;

#ifdef __ECHO_DEBUG
    fprintf(stdout, "\tECHO_echo (stub): end   %x \n", (unsigned int)(self()));
    fprintf(stdout, "\tv-------------------------\n");
#endif
  }
  if(cadena)
    free(cadena);
  fprintf(stdout, "e :::: Echo_client(%x). (Gix %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(0);

exception:
  fprintf(stdout, "e :::: FAIL!! Echo_client(%x). (Gix %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(-1);
}
