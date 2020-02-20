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

#ifndef P_GROUP_H
#define P_GROUP_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <object.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define GRP_E_OK            0
#define GRP_E_EXHAUST      (GRP_E_OK          - 1)
#define GRP_E_INTEGRITY    (GRP_E_EXHAUST     - 1)
#define GRP_E_TIMEOUT      (GRP_E_INTEGRITY   - 1)
#define GRP_E_INTERFACE    (GRP_E_TIMEOUT     - 1)
#define GRP_E_SYSTEM       (GRP_E_INTERFACE   - 1)
#define GRP_E_DISABLED     (GRP_E_SYSTEM      - 1)

#define GROUP_WORLD           0
#define GROUP_EMPTY           1
#define GROUP_SELF            2

/* Blocks for communication creation */
#define PREALLOC_GROUPS_COUNT           4
#define BLOCK_GROUPS_COUNT             32
#define PREALLOC_MAX_GROUPS_BLOCKS     64

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
/* Rank Group */
struct Mpi_Group {
  int          *Members;
  int           Size;       /* Total group size */
  int           Refs;
  int           LocalRank;  /* Process rank in this group */
  int           Index;
};
typedef struct Mpi_Group  Mpi_P_Group, *Mpi_P_Group_t;

/* Table of groups per rank */
struct GroupTable {
  Object_t        Groups;
  Mpi_P_Group    *World;
  Mpi_P_Group    *Empty;
  Mpi_P_Group    *Self;
};
typedef struct GroupTable GroupTable,  *GroupTable_t;

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
/* Member of the world group are a public global array */
extern  int *Group_World_Members_Ptr;

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
extern int         groupCreateDefault  ();
extern int         groupDeleteDefault  ();
  
extern int         groupAllocTable     (GroupTable_t *grptable);
extern int         groupFreeTable      (GroupTable_t *grptable);

extern int         groupCreate         (GroupTable   *grptable, int *members, int size, Mpi_P_Group_t *newgroup);
extern int         groupDelete         (GroupTable   *grptable, Mpi_P_Group_t *group);

#define            groupGetSize(group)                     (group)->Size
#define            groupGetLocalRank(group)                (group)->LocalRank
#define            groupGetGlobalRank(group, localrank)    (group)->Members[(localrank)]

extern int         groupGetRef         (Mpi_P_Group   *group, Mpi_P_Group_t *newgroup);


#endif
