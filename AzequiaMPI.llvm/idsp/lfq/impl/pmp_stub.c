/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public functions implemented by this module   *
 *----------------------------------------------------------------*/
#include <pmp.h>

/*----------------------------------------------------------------*
 *   Declaration of types and functions used by this module       *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <unistd.h>
#endif

#include <addr.h>
#include <thr.h>
#include <com.h>
#include <pmp_msg.h>
#include <rpc.h>
#include <xpn.h>

extern int    rpc_send  (const Addr_t dst, char     *buf,
                                           int       cnt,
                                           int       mode);
extern int    rpc_recv  (const Addr_t src, char     *buf,
                                           int       cnt,
                                           Status   *status,
                                           unsigned  timeout);

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#ifdef __PMP_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

#define TRY_PERIOD     1000  /* 1000 microseconds between tries */

#define LOAD_timeout   3000

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static char *e_names[8] = {  /* This order has to be consistent with pmp.h */
                             /*  0 */ "PMP_STUB_E_OK",
                             /*  1 */ "PMP_STUB_E_EXHAUST",
                             /*  2 */ "PMP_STUB_E_INTEGRITY",
                             /*  3 */ "PMP_STUB_E_TIMEOUT",
                             /*  4 */ "PMP_STUB_E_INTERFACE",
                             /*  5 */ "PMP_STUB_E_SYSTEM",
                             /*  6 */ "PMP_STUB_E_SIGNALED",
                             /*  7 */ "PMP_STUB_E_DEADPART"
                           };

/*----------------------------------------------------------------*
 *   Implementation of public functions                           *
 *----------------------------------------------------------------*/

     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |    PMP_get                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_get (int svc, int *gix, int *dstMchn) {

  Addr         addr;
  Pmp_Msg      msg;
  Pmp_Msg      msg_recv;
  int          excpn;
  Status       status;
  char        *where    = "PMP_get (clnt)";

  DBG_PRNT((stdout, "\n\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"));
  DBG_PRNT((stdout, "\t\tPMP_get(%p) (stub):\n", THR_self()));

  addr.Group              = PMP_GIX;
  addr.Rank               = *dstMchn;
  msg.Type                = PMP_GET;
  msg.Body.PortmapGet.Svc = svc;

    addr.Rank               = *dstMchn;

    DBG_PRNT((stdout, "\t\tPMP_get (stub): %p sending to: [G %d  R/M %d]\n", THR_self(), addr.Group, addr.Rank));

    if(0 > (excpn = rpc_send(&addr, (char *)&msg, sizeof(Pmp_Msg), 0)))         goto exception;

    addr.Rank = 0;

    DBG_PRNT((stdout, "\t\tPMP_get (stub): %p receiving from: [G %d  R/M %d]\n", THR_self(), addr.Group, addr.Rank));

    if((0 > (excpn = rpc_recv(&addr, (char *)&msg_recv,
                                     sizeof(Pmp_Msg),
                                     &status,
                                     TRY_PERIOD))) && (excpn != PMP_E_TIMEOUT)) goto exception;

    excpn = msg_recv.Body.ReplyPortmapGet.Reply;

  if(0 > excpn)                                                                 goto exception;

  *gix = msg_recv.Body.ReplyPortmapGet.Gix;
  *dstMchn = status.SrcMchn;

  DBG_PRNT((stdout, "\t\tPMP_get (stub): end %p \n", THR_self()));
  DBG_PRNT((stdout, "\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n"));

  return(PMP_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}

     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |    PMP_shutdown                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int PMP_shutdown (void) {

  Addr         addr;
  Pmp_Msg      msg;
  int          excpn;
  char        *where    = "PMP_shutdown (clnt)";

  DBG_PRNT((stdout, "\n\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"));
  DBG_PRNT((stdout, "\t\tPMP_shutdown(%p) (stub):\n", THR_self()));

  addr.Group              = PMP_GIX;
  addr.Rank               = getCpuId();
  msg.Type                = PMP_SHUTDOWN;

  DBG_PRNT((stdout, "\t\tPMP_shutdown (stub): %p sending to: [G %d  R/M %d]\n", THR_self(), addr.Group, addr.Rank));

  if (0 > (excpn = rpc_send(&addr, (char *)&msg, sizeof(Pmp_Msg), 0)))          goto exception;

  DBG_PRNT((stdout, "\t\tPMP_shutdown (stub): end %p \n", THR_self()));
  DBG_PRNT((stdout, "\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n"));

  return PMP_E_OK;

exception:
  XPN_print(excpn);
  return excpn;
}

