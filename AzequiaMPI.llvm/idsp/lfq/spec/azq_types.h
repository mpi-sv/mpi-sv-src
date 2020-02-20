/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _AZQ_TYPES_H_
#define _AZQ_TYPES_H_

typedef int (*fxn)();
typedef int bool;

#ifndef NULL
#define NULL   0
#endif

#ifndef TRUE
#define FALSE ((bool)0)
#define TRUE  ((bool)1)
#endif

/* Error codes for Azequia exported interface */
#define AZQ_SUCCESS       0
#define AZQ_E_EXHAUST    (AZQ_SUCCESS       - 1)
#define AZQ_E_INTEGRITY  (AZQ_E_EXHAUST     - 1)
#define AZQ_E_TIMEOUT    (AZQ_E_INTEGRITY   - 1)
#define AZQ_E_INTERFACE  (AZQ_E_TIMEOUT     - 1)
#define AZQ_E_SYSTEM     (AZQ_E_INTERFACE   - 1)
#define AZQ_E_SIGNALED   (AZQ_E_SYSTEM      - 1)
#define AZQ_E_DEADPART   (AZQ_E_SIGNALED    - 1)
#define AZQ_E_REQUEST    (AZQ_E_DEADPART    - 1)
#define AZQ_E_INTERN     (AZQ_E_REQUEST     - 1)


#endif

