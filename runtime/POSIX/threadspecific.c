
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadspecific.h"
#include <bits/libc-lock.h>
#include <bits/local_lim.h>
#include <assert.h>

static struct pthread_key_struct pthread_keys[PTHREAD_KEYS_MAX] =
  { { 0, NULL } };

/* Mutex to protect access to pthread_keys */
static pthread_mutex_t pthread_keys_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Create a new key */

int pthread_key_create(pthread_key_t * key, destr_function destr)
{
  int i;

  pthread_mutex_lock(&pthread_keys_mutex);
  for (i = 0; i < __PTHREAD_KEYS_MAX; i++) {
    if (! pthread_keys[i].in_use) {
      /* Mark key in use */
      pthread_keys[i].in_use = 1;
      pthread_keys[i].destr = destr;
      pthread_mutex_unlock(&pthread_keys_mutex);
      *key = i;
      return 0;
    }
  }
  pthread_mutex_unlock(&pthread_keys_mutex);
  return EAGAIN;
}

/* Reset deleted key's value to NULL in each live thread.
 * NOTE: this executes in the context of the thread manager! */

struct pthread_key_delete_helper_args {
  /* Damn, we need lexical closures in C! ;) */
  unsigned int idx1st, idx2nd;
  pthread_t self;
};

/*static void pthread_key_delete_helper(void *arg, pthread_t th)
{
  struct pthread_key_delete_helper_args *args = arg;
  unsigned int idx1st = args->idx1st;
  unsigned int idx2nd = args->idx2nd;
  //pthread_t self = pthread_self();
  thread_data_t * threaddata = GET_THEAD_DATA_BY_TID(th);
  if ( threaddata && threaddata->terminated) {
    //pthread_exit() may try to free th->p_specific[idx1st] concurrently.
    //pthread_lock(th->p_lock, self);
    if (threaddata->p_specific[idx1st] != NULL)
    	threaddata->p_specific[idx1st][idx2nd] = NULL;
    //_pthread_unlock(th->p_lock);
  }
}*/

/* Delete a key */
int pthread_key_delete(pthread_key_t key)
{
  pthread_t self = pthread_self();
  thread_data_t * threaddata = GET_THEAD_DATA_BY_TID(self);
  assert(threaddata);
  pthread_mutex_lock(&pthread_keys_mutex);
  if (key >= __PTHREAD_KEYS_MAX || !pthread_keys[key].in_use) {
    pthread_mutex_unlock(&pthread_keys_mutex);
    return EINVAL;
  }
  pthread_keys[key].in_use = 0;
  pthread_keys[key].destr = NULL;

  /* Set the value of the key to NULL in all running threads, so
     that if the key is reallocated later by pthread_key_create, its
     associated values will be NULL in all threads.

     If no threads have been created yet, clear it just in the
     current thread.  */

  struct pthread_key_delete_helper_args args;
  args.idx1st = key / __PTHREAD_KEY_2NDLEVEL_SIZE;
  args.idx2nd = key % __PTHREAD_KEY_2NDLEVEL_SIZE;

  if (threaddata->p_specific[args.idx1st] != NULL)
	  threaddata->p_specific[args.idx1st][args.idx2nd] = NULL;


  pthread_mutex_unlock(&pthread_keys_mutex);
  return 0;
}

/* Set the value of a key */

int pthread_setspecific(pthread_key_t key, const void * pointer)
{
  pthread_t self = pthread_self();
  thread_data_t * threaddata = GET_THEAD_DATA_BY_TID(self);
  assert(threaddata);
  unsigned int idx1st, idx2nd;

  if (key >= __PTHREAD_KEYS_MAX || !pthread_keys[key].in_use)
    return EINVAL;
  idx1st = key / __PTHREAD_KEY_2NDLEVEL_SIZE;
  idx2nd = key % __PTHREAD_KEY_2NDLEVEL_SIZE;
  if ((threaddata->p_specific)[idx1st] == NULL) {
    void *newp = calloc(__PTHREAD_KEY_2NDLEVEL_SIZE, sizeof (void *));
    if (newp == NULL)
      return ENOMEM;
    (threaddata->p_specific)[idx1st] = newp;
  }
  (threaddata->p_specific)[idx1st][idx2nd] = (void *) pointer;
//  printf("SET:loc:%d,%d,value:%ld, stored:%ld.\n",idx1st,idx2nd,pointer,(threaddata->p_specific)[idx1st][idx2nd]);
  return 0;
}


/* Get the value of a key */

void * pthread_getspecific(pthread_key_t key)
{
  pthread_t self = pthread_self();
  thread_data_t * threaddata = GET_THEAD_DATA_BY_TID(self);
  assert(threaddata);
  unsigned int idx1st, idx2nd;

  if (key >= PTHREAD_KEYS_MAX)
    return NULL;
  idx1st = key / __PTHREAD_KEY_2NDLEVEL_SIZE;
  idx2nd = key % __PTHREAD_KEY_2NDLEVEL_SIZE;

  if (threaddata->p_specific[idx1st] == NULL
      || !pthread_keys[key].in_use){
	  //printf("GET:get failed!!!!,key:%d,loc:%d,%d,%d,%d\n",key,idx1st,idx2nd,threaddata->p_specific[idx1st] == NULL,!pthread_keys[key].in_use);
	  return NULL;
  }
  //printf("GET:loc:%d,%d,value:%ld\n",idx1st,idx2nd,(threaddata->p_specific)[idx1st][idx2nd]);
  return (threaddata->p_specific)[idx1st][idx2nd];
}

/* Call the destruction routines on all keys */

void pthread_destroy_specifics()
{
  pthread_t self = pthread_self();
  thread_data_t * threaddata = GET_THEAD_DATA_BY_TID(self);
  assert(threaddata);

  int i, j, round, found_nonzero;
  destr_function destr;
  void * data;

  for (round = 0, found_nonzero = 1;
       found_nonzero && round < PTHREAD_DESTRUCTOR_ITERATIONS;
       round++) {
    found_nonzero = 0;
    for (i = 0; i < __PTHREAD_KEY_1STLEVEL_SIZE; i++)
      if (threaddata->p_specific[i] != NULL)
        for (j = 0; j < __PTHREAD_KEY_2NDLEVEL_SIZE; j++) {
          destr = pthread_keys[i * __PTHREAD_KEY_2NDLEVEL_SIZE + j].destr;
          data = threaddata->p_specific[i][j];
          if (destr != NULL && data != NULL) {
        	  (threaddata->p_specific[i])[j] = NULL;
            destr(data);
            found_nonzero = 1;
          }
        }
  }
//  pthread_lock(threaddata->p_lock, self);
  for (i = 0; i < __PTHREAD_KEY_1STLEVEL_SIZE; i++) {
    if (threaddata->p_specific[i] != NULL) {
      free(threaddata->p_specific[i]);
      threaddata->p_specific[i] = NULL;
    }
  }
//pthread_unlock(threaddata->p_lock);
}

