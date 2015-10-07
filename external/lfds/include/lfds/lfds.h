/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

/***** defines *****/
#if (defined PLATFORM_WINDOWS && defined BUILD_64_BIT && !defined WIN_KERNEL_BUILD)
// TRD : 64-bit Windows user-mode with the Microsoft C compiler, any CPU
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <intrin.h>
typedef unsigned __int64                 lfds_atom_t;
#define LFDS_INLINE                   __forceinline
#define LFDS_ALIGN(alignment)         __declspec( align(alignment) )
#define LFDS_ALIGN_SINGLE_POINTER     8
#define LFDS_ALIGN_DOUBLE_POINTER     16
#define LFDS_BARRIER_COMPILER_LOAD    _ReadBarrier()
#define LFDS_BARRIER_COMPILER_STORE   _WriteBarrier()
#define LFDS_BARRIER_COMPILER_FULL    _ReadWriteBarrier()
#define LFDS_BARRIER_PROCESSOR_LOAD   _mm_lfence()
#define LFDS_BARRIER_PROCESSOR_STORE  _mm_sfence()
#define LFDS_BARRIER_PROCESSOR_FULL   _mm_mfence()
#endif

#if (defined PLATFORM_WINDOWS && !defined BUILD_64_BIT && !defined WIN_KERNEL_BUILD)
// TRD : 32-bit Windows user-mode with the Microsoft C compiler, any CPU
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <intrin.h>
typedef unsigned long int                lfds_atom_t;
#define LFDS_INLINE                   __forceinline
#define LFDS_ALIGN(alignment)         __declspec( align(alignment) )
#define LFDS_ALIGN_SINGLE_POINTER     4
#define LFDS_ALIGN_DOUBLE_POINTER     8
#define LFDS_BARRIER_COMPILER_LOAD    _ReadBarrier()
#define LFDS_BARRIER_COMPILER_STORE   _WriteBarrier()
#define LFDS_BARRIER_COMPILER_FULL    _ReadWriteBarrier()
#define LFDS_BARRIER_PROCESSOR_LOAD   _mm_lfence()
#define LFDS_BARRIER_PROCESSOR_STORE  _mm_sfence()
#define LFDS_BARRIER_PROCESSOR_FULL   _mm_mfence()

// TRD : this define is documented but missing in Microsoft Platform SDK v7.0
#define _InterlockedCompareExchangePointer(destination, exchange, compare) _InterlockedCompareExchange((volatile long *) destination, (long) exchange, (long) compare)
#endif

#if (defined PLATFORM_WINDOWS && defined BUILD_64_BIT && defined WIN_KERNEL_BUILD)
// TRD : 64-bit Windows kernel with the Microsoft C compiler, any CPU
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wdm.h>
typedef unsigned __int64                 lfds_atom_t;
#define LFDS_INLINE                   __forceinline
#define LFDS_ALIGN(alignment)         __declspec( align(alignment) )
#define LFDS_ALIGN_SINGLE_POINTER     8
#define LFDS_ALIGN_DOUBLE_POINTER     16
#define LFDS_BARRIER_COMPILER_LOAD    _ReadBarrier()
#define LFDS_BARRIER_COMPILER_STORE   _WriteBarrier()
#define LFDS_BARRIER_COMPILER_FULL    _ReadWriteBarrier()
#define LFDS_BARRIER_PROCESSOR_LOAD   _mm_lfence()
#define LFDS_BARRIER_PROCESSOR_STORE  _mm_sfence()
#define LFDS_BARRIER_PROCESSOR_FULL   _mm_mfence()
#endif

#if (defined PLATFORM_LINUX && !defined BUILD_64_BIT && defined WIN_KERNEL_BUILD)
// TRD : 32-bit Windows kernel with the Microsoft C compiler, any CPU
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wdm.h>
typedef unsigned long int                lfds_atom_t;
#define LFDS_INLINE                   __forceinline
#define LFDS_ALIGN(alignment)         __declspec( align(alignment) )
#define LFDS_ALIGN_SINGLE_POINTER     4
#define LFDS_ALIGN_DOUBLE_POINTER     8
#define LFDS_BARRIER_COMPILER_LOAD    _ReadBarrier()
#define LFDS_BARRIER_COMPILER_STORE   _WriteBarrier()
#define LFDS_BARRIER_COMPILER_FULL    _ReadWriteBarrier()
#define LFDS_BARRIER_PROCESSOR_LOAD   _mm_lfence()
#define LFDS_BARRIER_PROCESSOR_STORE  _mm_sfence()
#define LFDS_BARRIER_PROCESSOR_FULL   _mm_mfence()

// TRD : this define is documented but missing in Microsoft Platform SDK v7.0
#define _InterlockedCompareExchangePointer(destination, exchange, compare) _InterlockedCompareExchange((volatile long *) destination, (long) exchange, (long) compare)
#endif

#if (defined PLATFORM_LINUX && defined BUILD_64_BIT)
// TRD : any UNIX on x64
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
typedef unsigned long long int           lfds_atom_t;
#define LFDS_INLINE                   inline
#define LFDS_ALIGN(alignment)         __attribute__( (aligned(alignment)) )
#define LFDS_ALIGN_SINGLE_POINTER     8
#define LFDS_ALIGN_DOUBLE_POINTER     16
#define LFDS_BARRIER_COMPILER_LOAD    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_STORE   __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_FULL    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_PROCESSOR_LOAD   __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_STORE  __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_FULL   __sync_synchronize()
#endif

#if (defined PLATFORM_LINUX && !defined BUILD_64_BIT)
// TRD : any UNIX with GCC on x86
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
typedef unsigned long int                lfds_atom_t;
#define LFDS_INLINE                   inline
#define LFDS_ALIGN(alignment)         __attribute__( (aligned(alignment)) )
#define LFDS_ALIGN_SINGLE_POINTER     4
#define LFDS_ALIGN_DOUBLE_POINTER     8
#define LFDS_BARRIER_COMPILER_LOAD    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_STORE   __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_FULL    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_PROCESSOR_LOAD   __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_STORE  __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_FULL   __sync_synchronize()
#endif

/*#if (defined __unix__ && defined __arm__ && __GNUC__)
// TRD : any UNIX with GCC on ARM
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
typedef unsigned long int                lfds_atom_t;
#define LFDS_INLINE                   inline
#define LFDS_ALIGN(alignment)         __attribute__( (aligned(alignment)) )
#define LFDS_ALIGN_SINGLE_POINTER     4
#define LFDS_ALIGN_DOUBLE_POINTER     8
#define LFDS_BARRIER_COMPILER_LOAD    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_STORE   __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_COMPILER_FULL    __asm__ __volatile__ ( "" : : : "memory" )
#define LFDS_BARRIER_PROCESSOR_LOAD   __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_STORE  __sync_synchronize()
#define LFDS_BARRIER_PROCESSOR_FULL   __sync_synchronize()
#endif*/

#define LFDS_BARRIER_LOAD   LFDS_BARRIER_COMPILER_LOAD; LFDS_BARRIER_PROCESSOR_LOAD; LFDS_BARRIER_COMPILER_LOAD
#define LFDS_BARRIER_STORE  LFDS_BARRIER_COMPILER_STORE; LFDS_BARRIER_PROCESSOR_STORE; LFDS_BARRIER_COMPILER_STORE
#define LFDS_BARRIER_FULL   LFDS_BARRIER_COMPILER_FULL; LFDS_BARRIER_PROCESSOR_FULL; LFDS_BARRIER_COMPILER_FULL

enum lfds_data_structure_validity {
	LFDS_VALIDITY_VALID,
	LFDS_VALIDITY_INVALID_LOOP,
	LFDS_VALIDITY_INVALID_MISSING_ELEMENTS,
	LFDS_VALIDITY_INVALID_ADDITIONAL_ELEMENTS,
	LFDS_VALIDITY_INVALID_TEST_DATA
};

struct lfds_validation_info {
	lfds_atom_t min_elements, max_elements;
};

/***** lfds_freelist *****/

enum lfds_freelist_query_type {
	LFDS_FREELIST_QUERY_ELEMENT_COUNT,
	LFDS_FREELIST_QUERY_VALIDATE
};

struct lfds_freelist_state;
struct lfds_freelist_element;

/***** public prototypes *****/
int lfds_freelist_new( struct lfds_freelist_state **fs, lfds_atom_t number_elements, int (*user_data_init_function)(void **user_data, void *user_state), void *user_state );
void lfds_freelist_use( struct lfds_freelist_state *fs );
void lfds_freelist_delete( struct lfds_freelist_state *fs, void (*user_data_delete_function)(void *user_data, void *user_state), void *user_state );

lfds_atom_t lfds_freelist_new_elements( struct lfds_freelist_state *fs, lfds_atom_t number_elements );

struct lfds_freelist_element *lfds_freelist_pop( struct lfds_freelist_state *fs, struct lfds_freelist_element **fe );
void lfds_freelist_push( struct lfds_freelist_state *fs, struct lfds_freelist_element *fe );

void *lfds_freelist_get_user_data_from_element( struct lfds_freelist_element *fe, void **user_data );
void lfds_freelist_set_user_data_in_element( struct lfds_freelist_element *fe, void *user_data );

void lfds_freelist_query( struct lfds_freelist_state *fs, enum lfds_freelist_query_type query_type, void *query_input, void *query_output );

/***** lfds_queue *****/

/***** enums *****/
enum lfds_queue_query_type {
	LFDS_QUEUE_QUERY_ELEMENT_COUNT,
};

/***** incomplete types *****/
struct lfds_queue_state;

/***** public prototypes *****/
int lfds_queue_new( struct lfds_queue_state **sq, lfds_atom_t number_elements );
void lfds_queue_use( struct lfds_queue_state *qs );
void lfds_queue_delete( struct lfds_queue_state *qs, void (*user_data_delete_function)(void *user_data, void *user_state), void *user_state );

int lfds_queue_enqueue( struct lfds_queue_state *qs, void *user_data );
int lfds_queue_guaranteed_enqueue( struct lfds_queue_state *qs, void *user_data );
int lfds_queue_dequeue( struct lfds_queue_state *qs, void **user_data );

void lfds_queue_query( struct lfds_queue_state *qs, enum lfds_queue_query_type query_type, void *query_input, void *query_output );
