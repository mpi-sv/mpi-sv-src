/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <azq.h>
#include "echo_msg.h"

#define __ECHO_SERVER_DEBUG


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


     /*----------------------------------------------------------*\
    |    ECHO_echo                                                 |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
static inline int ECHO_echo(char *buff, int buffSize)
{
   fprintf(stdout, "\t\tEchooooooooooooooo\n");
   return 0;
}


     /*----------------------------------------------------------*\
    |    Echo_Echo                                               |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
static void Echo_Echo(Echo_Msg_t rqst_msg, void *buff, int *cnt, Echo_Msg_t rply_msg)
{
  int    excpn;
  char  *where       = "Echo_Echo (skltn)";

#ifdef __ECHO_DEBUG
  fprintf(stdout, "\t\tECHO_ECHO command %x \n", (unsigned)(self()));
#endif
  if(0 > (excpn = rply_msg->Body.ReplyEchoEcho.Reply
                = ECHO_echo ((char *)buff, *cnt)))                              goto exception;
  rply_msg->Type = REPLY_ECHO_ECHO;
  *cnt = 0;
  return;

exception:
  return;
}


     /*----------------------------------------------------------*\
    |    echo_server                                               |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
int echo_server(int *param)
{
  static
  int          buff[256];
  int          bufSize     = param[0] * sizeof(int),
               excpn       = 0,
               clientMchn,
               i,
               gix         = getGroup(),
               myRank      = getRank(),
               ret         = 0;
  Thr_t        me          = THR_self();
  Addr         clientAddr;
  Hdr          hdr;
  Echo_Msg    *rqst_msg       = (Echo_Msg *)&hdr.Store;
  Echo_Msg    *rply_msg       = (Echo_Msg *)&hdr.Store;

  fprintf(stdout, "***** Echo_server(%x), (Gix %x, Rank %d) pth %x\n", (unsigned int)me, gix, myRank, (int)me->Self);
  if(0 > (ret = RPC_register(ECHO_PORT, gix)))                                       goto exception_0;
  while(1) {
    if(0 > (ret = RPC_recv(&hdr, (void *)buff, sizeof(buff))))                       goto exception_1;

#ifdef __ECHO_SERVER_DEBUG
    fprintf(stdout, "\n\t\t^--m#%d---------------------------------------\n", getCpuId());
#endif
    switch(rqst_msg->Type) {
      case ECHO_ECHO:    Echo_Echo   (rqst_msg, buff, &ret, rply_msg);  break;
      default:
        rply_msg->Type = REPLY_ECHO_OTHER;
        rply_msg->Body.ReplyEchoOther.Reply = -1;    break;
    }
#ifdef __ECHO_SERVER_DEBUG
    fprintf(stdout, "\t\tv---------------------------------------------\n");
#endif
    THR_getClient(&clientAddr, &clientMchn);
    if(0 > (ret = RPC_send(&hdr, (void *)buff, ret))) {
      switch(ret) {
        case RPC_E_DEADPART:
          THR_getClient(&clientAddr, &clientMchn);
          fprintf(stdout, "\t\tRPC_E_DEADPART rank %d ----------------------------------------------\n", clientAddr.Rank);
          continue;
          break;
        default:
          goto exception_1;
      }
    }
  }
  fprintf(stdout, "e :::: Echo_server(%x). (Gix %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(0);

exception_1:
  RPC_unregister(ECHO_PORT);
exception_0:
  fprintf(stdout, ":::: FAIL!! Echo_server(%x). (Gix %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(-1);
}
