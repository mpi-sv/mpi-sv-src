/*
 * Added by Herman, to support pthread_key_create, pthread_getspecific.
 * pthread_setspecific,etc.
 */
#ifndef THREADSPECIFIC_H_
#define THREADSPECIFIC_H_
#include "multiprocess.h"

#ifdef HAVE_MPI_SUPPORT

#define KEY_UNUSED(p)  (((p) & 1) == 0)

#define KEY_USABLE(p)  (((uintptr_t)(p)) < ((uintptr_t) ((p) + 2)))

#define GET_THEAD_DATA_BY_TID(tid)  ((tid >= 0 && tid < MAX_THREADS) ? (&(__tsync.threads[tid])) : (NULL))

typedef void (*destr_function) (void *);

typedef struct pthread_key_struct{
	int in_use;
	destr_function destr;
} pthread_key_data_t;

#endif

#endif
