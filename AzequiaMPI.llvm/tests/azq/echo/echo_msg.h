#ifndef _ECHO_MSG_H_
#define _ECHO_MSG_H_

#define ECHO_PORT          1024

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Declaration of data types                                    *
 *----------------------------------------------------------------*/
typedef enum{ ECHO_ECHO,
              REPLY_ECHO_ECHO,
              REPLY_ECHO_OTHER
            } EchoType;


/*__ ECHO_ECHO _________________________*/
typedef struct EchoEcho {            /* Request */
        }      EchoEcho;

typedef struct ReplyEchoEcho {       /* Replay  */
          int  Reply;
        }      ReplyEchoEcho;



/*__ ECHO_OTHER _________________________*/
typedef struct ReplyEchoOther {
          int  Reply;
        }      ReplyEchoOther;




/* Echo _________________________*/
struct Echo_Msg {
  EchoType Type;
  union EchoBody  {
    EchoEcho              EchoEcho;
    ReplyEchoEcho         ReplyEchoEcho;

    ReplyEchoOther        ReplyEchoOther;
  } Body;
};
typedef struct Echo_Msg Echo_Msg, *Echo_Msg_t;

#endif
