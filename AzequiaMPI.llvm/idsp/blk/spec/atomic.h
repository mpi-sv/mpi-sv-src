#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <config.h>

/* Must be consistent with azqmpi/include/arch.h */
#define  AMD64   0x00000001
#define  IA32    0x00000002
#define  IA64    0x00000004

#if (AZQMPI_ARCH == IA32)

#define ATOMIC_BOOL_CMPXCHG(ptr, old_value, new_value) __sync_bool_compare_and_swap((volatile long * const) (ptr), (const long) (old_value), (const long) (new_value))
#define ATOMIC_VAL_CMPXCHG(ptr, old_value, new_value) __sync_val_compare_and_swap((volatile long * const) (ptr), (const long) (old_value), (const long) (new_value))
#define ATOMIC_XCHG(ptr, value) __sync_lock_test_and_set((volatile long * const) (ptr), (const long) (value))
#define ATOMIC_ADD(ptr, value) __sync_fetch_and_add((volatile long * const) (ptr), (const long) (value))

#elif (AZQMPI_ARCH == IA64 || AZQMPI_ARCH == AMD64)

#define ATOMIC_BOOL_CMPXCHG(ptr, old_value, new_value) __sync_bool_compare_and_swap((volatile long long * const) (ptr), (const long long) (old_value), (const long long) (new_value))
#define ATOMIC_VAL_CMPXCHG(ptr, old_value, new_value) __sync_val_compare_and_swap((volatile long long * const) (ptr), (const long long) (old_value), (const long long) (new_value))
#define ATOMIC_XCHG(ptr, value) __sync_lock_test_and_set((volatile long long * const) (ptr), (const long long) (value))
#define ATOMIC_ADD(ptr, value) __sync_fetch_and_add((volatile long long * const) (ptr), (const long long) (value))

#else

#error "Architecture not supported!!"

#endif	/* AZQMPI_ARCH */

#endif	/* _ATOMIC_H_ */
