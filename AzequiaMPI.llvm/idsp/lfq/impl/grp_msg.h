/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _GRP_MSG_H_
#define _GRP_MSG_H_


/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <thr.h>


/*----------------------------------------------------------------*
 *   Declaration of data types                                    *
 *----------------------------------------------------------------*/
typedef enum{ GRP_CREATE,
              GRP_START,
              GRP_KILL,
              GRP_LEAVE,
              GRP_JOIN,
              GRP_WAIT,
              GRP_WAIT2,
              GRP_DESTROY,
              GRP_SHUTDOWN,

              REPLY_GRP_CREATE,
              REPLY_GRP_START,
              REPLY_GRP_KILL,
              REPLY_GRP_LEAVE,
              REPLY_GRP_JOIN,
              REPLY_GRP_WAIT,
              REPLY_GRP_WAIT2,
              REPLY_GRP_SHUTDOWN,

              REPLY_GRP_OTHER
            } GrpType;


/*__ GRP_CREATE _________________________*/
typedef struct GrpCreate {            /* Request */
          int      Size;
          int      Creator;
          int      Gix;
        }      GrpCreate;

typedef struct ReplyGrpCreate {       /* Replay  */
          int  Reply;
        }      ReplyGrpCreate;



/*__ GRP_DESTROY _________________________*/
typedef struct GrpDestroy {            /* Request */
          int      Size;
          int      Gix;
        }      GrpDestroy;


/*__ GRP_SHUTDOWN _________________________*/
typedef struct GrpShutdown {           /* Request */
          int  Empty;
        }      GrpShutdown;

typedef struct ReplyGrpShutdown {      /* Replay  */
          int  Reply;
        }      ReplyGrpShutdown;


/*__ GRP_START _________________________*/
typedef struct GrpStart {              /* Request */
          int  Gix;
        }      GrpStart;
typedef struct ReplyGrpStart {         /* Replay  */
          int  Reply;
        }      ReplyGrpStart;




/* GRP_KILL _________________________*/
typedef struct GrpKill {             /* Request */
          int  Gix;
        }      GrpKill;

typedef struct ReplyGrpKill {        /* Replay  */
          int  Reply;
        }      ReplyGrpKill;




/* GRP_LEAVE _________________________*/
typedef struct GrpLeave {             /* Request */
          int  ExitCode;
        }      GrpLeave;

typedef struct ReplyGrpLeave {        /* Reply  */
          int  Reply;
        }      ReplyGrpLeave;




/* GRP_WAIT _________________________*/
typedef struct GrpWait {             /* Request */
          int  Gix;
          int  GrpSize;
        }      GrpWait;

typedef struct ReplyGrpWait {        /* Reply  */
          int  Reply;
        }      ReplyGrpWait;




/*__ GRP_JOIN _________________________*/
typedef struct GrpJoin {            /* Request */
          int      Gix;
          int      Rank;
          int      Name;
          CommAttr CommAttr;
        }      GrpJoin;

typedef struct ReplyGrpJoin {       /* Replay  */
          int  Reply;
        }      ReplyGrpJoin;



/*__ GRP_OTHER _________________________*/
typedef struct ReplyGrpOther {
          int  Reply;
        }      ReplyGrpOther;




/* Grp _________________________*/
struct Grp_Msg {
  GrpType Type;
  union GrpBody  {
    GrpCreate             GrpCreate;
    ReplyGrpCreate        ReplyGrpCreate;

    GrpDestroy            GrpDestroy;

    GrpShutdown           GrpShutdown;
    ReplyGrpShutdown      ReplyGrpShutdown;

    GrpJoin               GrpJoin;
    ReplyGrpJoin          ReplyGrpJoin;

    GrpStart              GrpStart;
    ReplyGrpStart         ReplyGrpStart;

    GrpKill               GrpKill;
    ReplyGrpKill          ReplyGrpKill;

    GrpLeave              GrpLeave;
    ReplyGrpLeave         ReplyGrpLeave;

    GrpWait               GrpWait;
    ReplyGrpWait          ReplyGrpWait;

    ReplyGrpOther         ReplyGrpOther;
  } Body;
};
typedef struct Grp_Msg Grp_Msg, *Grp_Msg_t;

#endif
