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

  /*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <p_group.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <pthread.h>
#endif

#include <azq_types.h>
#include <com.h>

#include <p_config.h>
#include <env.h>

  /*----------------------------------------------------------------/
 /                  Definition of private data                     /
/----------------------------------------------------------------*/
int *Group_World_Members_Ptr;

  /*-------------------------------------------------------/
 /                 Private interface                      /
/-------------------------------------------------------*/
//PRIVATE PRV_Group *groupAlloc  (PRV_GroupTable *grptable);

#ifdef DEBUG_MODE
#define ALL        (-1)
PRIVATE int        printGroup  (GroupTable *grptable, int index);
#endif

  /*-------------------------------------------------------/
 /    Public interface for group table management         /
/-------------------------------------------------------*/
/**
 *   groupAllocTable
 *     Allocate the table for storing the MPI group objects for a process
 */
int groupAllocTable (GroupTable_t *grptable) {

  Mpi_P_Group  *groups;
  GroupTable   *grptab;
  int           i, error;
//fprintf(stdout, "\ngroupAllocTable (%p): BEGIN\n", self()); fflush(stdout);

  /* 1. Allocate group table structure, groups in the table and members in each group. An special field in
        the group table allows a process to use it into some functions without calling malloc */
  if (posix_memalign((void *)&grptab, CACHE_LINE_SIZE, sizeof(GroupTable)))     goto exception_1;
  
  /* 2. Allocate communicators */
  objInit (&grptab->Groups, PREALLOC_MAX_GROUPS_BLOCKS, 
                            sizeof(Mpi_P_Group), 
                            PREALLOC_GROUPS_COUNT, 
                            BLOCK_GROUPS_COUNT, &error); 
  if(error)	                                                                            goto exception_2;
  
  *grptable = grptab;
//fprintf(stdout, "groupAllocTable (%p): END\n", self()); fflush(stdout);
  return (GRP_E_OK);
  
exception_2:
  free(grptab);
exception_1:
  return GRP_E_EXHAUST;
}


/**
 *   groupFreeTable
 *     Free the table for storing the MPI group objects for an application
 */
int groupFreeTable (GroupTable_t *grptable) {
  
  objFinalize(&(*grptable)->Groups);
  
  free(*grptable);

  *grptable = (GroupTable *)NULL;
  
  return GRP_E_OK;
}

  /*-------------------------------------------------------/
 /        Public interface for group management           /
/-------------------------------------------------------*/
/*
 *  groupCreateDefault()
 *    Per process groups world members. This vector could be very large, so
 *    only one copy is created by process.
 */
int groupCreateDefault () {
  
  int   grpsize;
  int   i;
  
  
  GRP_getSize(getGroup(), &grpsize);
  
  /* World group */
  if (posix_memalign(&Group_World_Members_Ptr, CACHE_LINE_SIZE, grpsize * sizeof(int)))
	return GRP_E_EXHAUST;
  for (i = 0; i < grpsize; i++) Group_World_Members_Ptr[i] = i;
  
  
  return GRP_E_OK;
}
  

/*
 *  groupDeleteDefault()
 */
int groupDeleteDefault () {
  
  int   grpsize;
  int   i;
  
  free(Group_World_Members_Ptr);  
  
  return GRP_E_OK;
}


  
/**
 *   groupCreate
 *
 */
int groupCreate (GroupTable *grptable, int *members, int size, Mpi_P_Group_t *newgroup) {

  Mpi_P_Group *grp;
  int          i;
  int          rank_in_group = getRank(); /* Global rank */
  int          idx;
  
//fprintf(stdout, "\ngroupCreate (%p): BEGIN\n", self()); fflush(stdout);

  /* 1. Get group */
  /*
  if (0 > objAlloc(grptable->Groups, &grp, &idx)) {
    *newgroup = (Mpi_P_Group *)NULL;
    return(GRP_E_EXHAUST);
  }
  */
  objAlloc(grptable->Groups, &grp, &idx);
  grp->Index = idx;
  grp->Refs  = 1;
  grp->Size  = size;

  if (size == 0) {
	grp->Members = NULL;
  
  } else {
	
	if (members == Group_World_Members_Ptr) {
	  
	  grp->Members = Group_World_Members_Ptr;
	  
	  grp->LocalRank = PROC_NULL;
      for (i = 0; i < size; i++) {
        if (rank_in_group == Group_World_Members_Ptr[i])
          grp->LocalRank = i;
      }
	  
	} else {
	  
      if (posix_memalign((void *)&grp->Members, CACHE_LINE_SIZE, size * sizeof(int)))  
        return GRP_E_EXHAUST;
      
	  memcpy(grp->Members, members, size * sizeof(int));
	
      grp->LocalRank = PROC_NULL;
      for (i = 0; i < size; i++) {
        grp->Members[i] = members[i];
        if (rank_in_group == members[i])
          grp->LocalRank = i;
      }
	  
	}
	
  }
  
  *newgroup = grp;

#ifdef DEBUG_MODE
  printGroup(grptable, ALL);
#endif
//fprintf(stdout, "groupCreate (%p): END\n", self()); fflush(stdout);
  return GRP_E_OK;
}


/**
 *  groupDelete
 *    Delete a reference to the group. The group is deallocated if the Refs field reach 0
 */
int groupDelete (GroupTable *grptable, Mpi_P_Group_t *group) {

   (*group)->Refs--;
  
  if ((*group)->Refs == 0) {
	if (((*group)->Members) && ((*group)->Members != Group_World_Members_Ptr)) 
        free((*group)->Members);
	
	objFree(grptable->Groups, group, (*group)->Index);
  }
    
  *group = (Mpi_P_Group *) NULL;
  
#ifdef DEBUG_MODE
  printGroup(grptable, ALL);
#endif

  return GRP_E_OK;
}


/*
 *  groupGetRankInGroup
 *    Return the local rank of a process in a group
 */
int groupGetRankInGroup (Mpi_P_Group *group, int globalrank) {
  
  int i;
  
  for (i = 0; i < group->Size; i++)
    if (group->Members[i] == globalrank)   return i;
  
  return PROC_NULL;
}


/*
 *   groupGetRef
 *     A reference count is maintained for avoid duplication of groups
 */
int groupGetRef (Mpi_P_Group *group, Mpi_P_Group_t *newgroup) {
  
  *newgroup = group;
  group->Refs++;
  
  return GRP_E_OK;
}


  /*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
/-------------------------------------------------------*/
#ifdef DEBUG_MODE

PRIVATE int printGroup (GroupTable *grptable, int index) {

  int i, j;

  fprintf(stdout, " >>>>>> GROUPS <<<<<<  (GroupTable %x)\n", grptable);
  /* TODO */
  /*
   objPrint(object);
   */
  
  return 0;
}

#endif
