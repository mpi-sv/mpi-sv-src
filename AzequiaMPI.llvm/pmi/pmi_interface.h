/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

#ifndef _PMI_INTERFACE_H
#define _PMI_INTERFACE_H


  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/

struct Placement {
  int Binded;
  int Socket;
  int Core;
  int LocalRank;
  int GlobalRank;
};
typedef struct Placement Placement, *Placement_t;


  /*----------------------------------------------------------------/
 /    Definition of public constants                               /
/----------------------------------------------------------------*/

  /*----------------------------------------------------------------/
 /      declaration of public functions                            /
/----------------------------------------------------------------*/

extern int   PMII_init                   ();
extern void  PMII_finalize               ();  
extern int   PMII_getNodeNr              ();
extern int   PMII_spreadNodesOnMachines  (int *mchn, int nodenr);
extern int   PMII_setBindParams          (Placement_t place, int globalrank, int localrank);
extern int   PMII_bindSelf               (Placement_t place);
extern void  PMII_setUnBind              (Placement_t place);


#endif

