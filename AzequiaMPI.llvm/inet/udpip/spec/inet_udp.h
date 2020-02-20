/*
 * Discussion:
 *
 * Assuming the following:
 *
 * o UDP does not guarantee reliability and ordering of datagrams. UDP payload size is 65528 bytes.
 * o TCP/UDP stack is using ETHERNET as link layer. Ethernet payload size is 1500 bytes.
 * o PCs with Linux has a lot of memory, so we are not restricted using large buffers.
 *
 * a) In order to avoid more fragmentation in TCP/IP stack, we will define the FRAGMENT_SIZE constant
 *    as the maximun payload size of an ETHERNET packet, that is, 1500 bytes. This constant indicates
 *    to the Azequia's NET level that messages bigger than 1500 bytes must be fragmented.
 *
 * b) We define a cyclic buffer of 43 chunks of 1500 bytes each one. 43 * 1500 is approximately 64 KB
 *    of total memory used by machine.
 *
 */

#ifndef INET_UDP_H
#define INET_UDP_H

#include <inet.h>

#define SERVER_PORT    6969

/* Maximum fragment size */

int FRAGMENT_SIZE = 1436;           /* 1500 (ethernet mtu) - 64 (azequia message header size) */

struct DEV_iovec {
  char *Begin;
  int   Size;
};
typedef struct DEV_iovec DEV_iovec, *DEV_iovec_t;


#endif
