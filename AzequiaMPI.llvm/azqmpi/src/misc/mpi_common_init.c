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
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <common.h>

#ifdef __OSI
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <time.h>
  #include <pthread.h>
#endif

#include <thr.h>
#include <com.h>
#include <grp.h>

#include <env.h>
#include <errhnd.h>
#include <p_group.h>
#include <p_key.h>

extern struct DefaultAttributes def_attrs;


/**
 *  common_init
 */
int common_init (int *argc, char **argv[], int thread_support) {

  MPI_Group       group_world, group_self, group_empty;
  MPI_Comm        comm_world, comm_empty;
  MPI_Errhandler  errhandlerfatal, errhandlerreturn;

  int             grpsize;
  int            *grp_procs;
  int             self_member;
  int             i;
  int             keyval;
  int             attr_val;


  if (0 > PCS_init(thread_support))                                             return MPI_ERR_INTERN;

  int             rank    = getRank();
  int             grp_nr  = getGroup();

  /* 2. Create default groups */
  /* 2.1. World group */
  GRP_getSize(grp_nr, &grpsize);
  if (0 > PCS_groupCreate(Group_World_Members_Ptr, grpsize, &group_world))      return MPI_ERR_GROUP;
  PCS_groupSet(GROUP_WORLD, group_world);
  /* 2.2. Empty group */ 
  if (0 > PCS_groupCreate(NULL, 0, &group_empty))                               return MPI_ERR_GROUP;
  PCS_groupSet(GROUP_EMPTY, group_empty);
  /* 2.3. Self group. Only for Comm self support */
  self_member = rank;
  if (0 > PCS_groupCreate(&self_member, 1, &group_self))                        return MPI_ERR_GROUP;
  PCS_groupSet(GROUP_SELF, group_self);

  /* 3. Default error handlers */
  if (0 > PCS_errhndCreate((Mpi_P_Handler_function *)FatalErrors,  &errhandlerfatal))
                                                                                return MPI_ERR_OTHER;
  PCS_errhndSet(FATAL_ERROR, errhandlerfatal);
  if (0 > PCS_errhndCreate((Mpi_P_Handler_function *)ReturnErrors, &errhandlerreturn))
                                                                                return MPI_ERR_OTHER;
  PCS_errhndSet(RETURN_ERROR, errhandlerreturn);
  
  /* 4. Create default communicators */
  /* 4.1. WORLD communicator */
  if (0 > comm_create((Mpi_P_Comm *)MPI_COMM_NULL, group_world, NULL, 0, INTRACOMM,
                                      (void *)errhandlerfatal, &comm_world))    return MPI_ERR_COMM;
  PCS_commSet(COMM_WORLD, comm_world);
  /* 4.2. EMPTY communicator */
  if (0 > comm_create((Mpi_P_Comm *)MPI_COMM_NULL, group_self, NULL, 5, INTRACOMM,
                                      (void *)errhandlerfatal, &comm_empty))    return MPI_ERR_COMM;
  PCS_commSet(COMM_SELF, comm_empty);
 
  /* 5. Create default attributes */
  /* 5.1. MPI_TAG_UB, value TAG_UB */
  if (0 > PCS_keyAlloc(&keyval, NULL, MPI_DUP_FN, MPI_NULL_DELETE_FN))          return MPI_ERR_OTHER;
  if (keyval != MPI_TAG_UB)                                                     return MPI_ERR_OTHER;
  if (0 > PCS_keyPutAttr(MPI_COMM_WORLD, MPI_TAG_UB, &def_attrs.TagUb))         return MPI_ERR_OTHER;
   
  /* 5.2. MPI_HOST, value MPI_PROC_NULL */
  if (0 > PCS_keyAlloc(&keyval, NULL, MPI_DUP_FN, MPI_NULL_DELETE_FN))          return MPI_ERR_OTHER;
  if (keyval != MPI_HOST)                                                       return MPI_ERR_OTHER;
  if (0 > PCS_keyPutAttr(MPI_COMM_WORLD, MPI_HOST, &def_attrs.Host))            return MPI_ERR_OTHER;

  /* 5.3. MPI_IO, value actual rank */
  if (0 > PCS_keyAlloc(&keyval, NULL, MPI_DUP_FN, MPI_NULL_DELETE_FN))          return MPI_ERR_OTHER;
  if (keyval != MPI_IO)                                                         return MPI_ERR_OTHER;
  if (0 > PCS_keyPutAttr(MPI_COMM_WORLD, MPI_IO, &def_attrs.Io))                return MPI_ERR_OTHER;

  /* 5.4. MPI_WTIME_IS_GLOBAL, value FALSE */
  if (0 > PCS_keyAlloc(&keyval, NULL, MPI_DUP_FN, MPI_NULL_DELETE_FN))          return MPI_ERR_OTHER;
  if (keyval != MPI_WTIME_IS_GLOBAL)                                            return MPI_ERR_OTHER;
  if (0 > PCS_keyPutAttr(MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &def_attrs.WTimeIsGlobal))
                                                                                return MPI_ERR_OTHER;

  /* 5.5. Implementation specific atribute: MPI_MAX_NODES, value MAX_NODES in Azequia */
  if (0 > PCS_keyAlloc(&keyval, NULL, MPI_DUP_FN, MPI_NULL_DELETE_FN))          return MPI_ERR_OTHER;
  if (keyval != MPI_MAX_NODES)                                                  return MPI_ERR_OTHER;
  if (0 > PCS_keyPutAttr(MPI_COMM_WORLD, MPI_MAX_NODES, &def_attrs.MaxNodes))   return MPI_ERR_OTHER;


  return MPI_SUCCESS;
}

