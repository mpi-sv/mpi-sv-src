/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _PMP_MSG_H_
#define _PMP_MSG_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Declaration of data types                                    *
 *----------------------------------------------------------------*/
typedef enum{ PMP_GET,
              PMP_REGISTER,
              PMP_UNREGISTER,
              PMP_GETLOAD,
              PMP_SHUTDOWN,

              REPLY_PMP_GET,
              REPLY_PMP_REGISTER,
              REPLY_PMP_UNREGISTER,
      			  REPLY_PMP_GETLOAD,
              REPLY_PMP_OTHER
            } PortmapType;


/*__ PMP_GET _________________________*/
typedef struct PortmapGet {
          int  Svc;
        }      PortmapGet;
/*__ REPLY_PMP_GET _________________________*/
typedef struct ReplyPortmapGet {
          int  Reply;
          int  Gix;
        }      ReplyPortmapGet;


/*__ PMP_REGISTER _________________________*/
typedef struct PortmapRegister {
          int  Svc;
          int  Gix;
        }      PortmapRegister;
/*__ REPLY_PMP_REGISTER _________________________*/
typedef struct ReplyPortmapRegister {
          int  Reply;
        }      ReplyPortmapRegister;


/*__ PMP_UNREGISTER _________________________*/
typedef struct PortmapUnregister {
          int  Svc;
        }      PortmapUnregister;
/*__ REPLY_PMP_UNREGISTER _________________________*/
typedef struct ReplyPortmapUnregister {
          int  Reply;
        }      ReplyPortmapUnregister;


/*__ PMP_GETLOAD _________________________*/
typedef struct PortmapGetLoad {
          int  Dummy;
        }      PortmapGetLoad;

/*__ REPLY_PMP_GETLOAD _________________________*/
typedef struct ReplyPortmapGetLoad {
          int  Reply;
          int  CpuLoad;
        }      ReplyPortmapGetLoad;


/*__ REPLY_PMP_OTHER _________________________*/
typedef struct ReplyPortmapOther {
          int  Reply;
        }      ReplyPortmapOther;


/* Portmap _________________________*/
struct Pmp_Msg {
  PortmapType Type;
  union PortmapBody  {
    PortmapGet              PortmapGet;
    ReplyPortmapGet         ReplyPortmapGet;

    PortmapRegister         PortmapRegister;
    ReplyPortmapRegister    ReplyPortmapRegister;

    PortmapUnregister       PortmapUnregister;
    ReplyPortmapUnregister  ReplyPortmapUnregister;

    PortmapGetLoad          PortmapGetLoad;
    ReplyPortmapGetLoad     ReplyPortmapGetLoad;

    ReplyPortmapOther       ReplyPortmapOther;
  } Body;
};
typedef struct Pmp_Msg Pmp_Msg, *Pmp_Msg_t;

#endif
