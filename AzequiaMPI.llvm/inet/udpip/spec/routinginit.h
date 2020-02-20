#ifndef __ROUTING_INIT
#define __ROUTING_INIT

 


/*

   This module implements a protocol to find out the routing 
   table for current network configuration.

   Calling RI_getNetConfig() causes execution to wait until the information is 
   available. The information can be retrieved from the flash memory if available.
   If no flash is available, the information will be received by the network from 
   another node having flash, or having received the information previously.
   If none of the nodes have flash memory, then the host must send the information
   to the root node, who will pass it to the rest (the host is then assumed to be
   running the other side of protocol).

   ================================================================================
   AT LEAST 128KB OF HEAP AVAILABLE ARE NEEDED IN ORDER TO READ NET INFO FROM FLASH
   ================================================================================

   It is assumed that every node of the network call RI_getNetConfig() on starting
   its execution.

   RI_getNetConfig() return the next information:

   <cpuCount> is the number of DSPs in the system.
   <thisCpu>  is the number of cuurent DSP in the system (unique ID), from 0 to 
                    <cpuCount>-1, where 0 is the root and has direct connection
                    to host.

   <routingTable> is a vector of <cpuCount> entries, containing the number of 
                    communication device to uso in order to send messages to
                    other DSPs (index <i> of vector is the device to arrive to 
                    DSP <i>).

   IMPORTANT NOTES:
   
   - routingTable[thisCpu] == -1 if there is no loopback connection for local.

   - Device numbers are numbered as follows:
             0 .. CP_MAX-1          <-------->   commports 0 .. CP_MAX-1
        CP_MAX .. CP_MAX+SDB_MAX-1  <-------->   SDBs      0 .. SDB_MAX-1

     where CP_MAX and SDB_MAX are defined in <sundace.h>

   - When RI_getNetConfig() returns, all nodes with direct connection with local DSP 
     are prepared to receive communications, so local node can start normal execution.
     (but any of the rest of nodes on the system can still be uninitialized).

*/




/******************************************************************************/
/* DECLARATION OF PUBLIC TYPES                                                */
/******************************************************************************/

typedef int RI_RoutingTableEntry;


/******************************************************************************/
/* DECLARATION OF PUBLIC INTERFACE                                            */
/******************************************************************************/


// call init to execute complete protocol of retrieving routing information 
//      and propagating to rest of modules. Then use getX functions

int RI_init();
int RI_getCpuCount();
int RI_getCpuId();
int RI_getRoutingTable(RI_RoutingTableEntry *RoutingTable);
int RI_destroy();


#endif





