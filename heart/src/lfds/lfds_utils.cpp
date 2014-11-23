/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "lfds/lfds.h"
#include "base/hMemory.h"

#define lfds_liblfds_aligned_malloc(s, a)  	Heart::hMalloc(s, a)
#define lfds_liblfds_aligned_free(p)		Heart::hFree(p)

/***** defines *****/
#define RAISED   1
#define LOWERED  0

#define NO_FLAGS 0x0

static LFDS_INLINE lfds_atom_t lfds_abstraction_cas( volatile lfds_atom_t *destination, lfds_atom_t exchange, lfds_atom_t compare );
static LFDS_INLINE unsigned char lfds_abstraction_dcas( volatile lfds_atom_t *destination, lfds_atom_t *exchange, lfds_atom_t *compare );
static LFDS_INLINE lfds_atom_t lfds_abstraction_increment( volatile lfds_atom_t *value );

/****************************************************************************/
#if (defined HEART_PLAT_WINDOWS)

  /* TRD : 64 bit and 32 bit Windows (user-mode or kernel) on any CPU with the Microsoft C compiler

           _WIN32    indicates 64-bit or 32-bit Windows
           _MSC_VER  indicates Microsoft C compiler
  */

  static LFDS_INLINE lfds_atom_t lfds_abstraction_cas( volatile lfds_atom_t *destination, lfds_atom_t exchange, lfds_atom_t compare )
  {
    lfds_atom_t
      rv;

    assert( destination != NULL );
    // TRD : exchange can be any value in its range
    // TRD : compare can be any value in its range

    LFDS_BARRIER_COMPILER_FULL;

    rv = (lfds_atom_t) _InterlockedCompareExchangePointer( (void * volatile *) destination, (void *) exchange, (void *) compare );

    LFDS_BARRIER_COMPILER_FULL;

    return( rv );
  }

#endif





/****************************************************************************/
#if (defined HEART_PLAT_LINUX)

  /* TRD : any OS on any CPU with GCC 4.1.0 or better

           GCC 4.1.0 introduced the __sync_*() atomic intrinsics

           __GNUC__ / __GNUC_MINOR__ / __GNUC_PATCHLEVEL__  indicates GCC and which version
  */

  static LFDS_INLINE lfds_atom_t lfds_abstraction_cas( volatile lfds_atom_t *destination, lfds_atom_t exchange, lfds_atom_t compare )
  {
    lfds_atom_t
      rv;

    assert( destination != NULL );
    // TRD : exchange can be any value in its range
    // TRD : compare can be any value in its range

    // TRD : note the different argument order for the GCC instrinsic to the MSVC instrinsic

    LFDS_BARRIER_COMPILER_FULL;

    rv = (lfds_atom_t) __sync_val_compare_and_swap( destination, compare, exchange );

    LFDS_BARRIER_COMPILER_FULL;

    return( rv );
  }

  static LFDS_INLINE lfds_atom_t lfds_abstraction_increment( volatile lfds_atom_t *value )
  {
    lfds_atom_t
      rv;

    assert( value != NULL );

    // TRD : no need for casting here, GCC has a __sync_add_and_fetch() for all native types

    LFDS_BARRIER_COMPILER_FULL;

    rv = (lfds_atom_t) __sync_add_and_fetch( value, 1 );

    LFDS_BARRIER_COMPILER_FULL;

    return( rv );
  }

#endif

/****************************************************************************/
#if (defined HEART_PLAT_WINDOWS && defined HEART_64BIT)

  /* TRD : 64 bit Windows (user-mode or kernel) on any CPU with the Microsoft C compiler

           _WIN64    indicates 64 bit Windows
           _MSC_VER  indicates Microsoft C compiler
  */

  static LFDS_INLINE unsigned char lfds_abstraction_dcas( volatile lfds_atom_t *destination, lfds_atom_t *exchange, lfds_atom_t *compare )
  {
    unsigned char
      cas_result;

    assert( destination != NULL );
    assert( exchange != NULL );
    assert( compare != NULL );

    LFDS_BARRIER_COMPILER_FULL;

    cas_result = _InterlockedCompareExchange128( (volatile __int64 *) destination, (__int64) *(exchange+1), (__int64) *exchange, (__int64 *) compare );

    LFDS_BARRIER_COMPILER_FULL;

    return( cas_result ) ;
  }

    static LFDS_INLINE lfds_atom_t lfds_abstraction_increment( volatile lfds_atom_t *value )
  {
    lfds_atom_t
      rv;

    assert( value != NULL );

    LFDS_BARRIER_COMPILER_FULL;

    rv = (lfds_atom_t) _InterlockedIncrement64( (__int64 *) value );

    LFDS_BARRIER_COMPILER_FULL;

    return( rv );
  }

#endif





/****************************************************************************/
#if (defined HEART_PLAT_WINDOWS && defined HEART_32BIT)

  /* TRD : 32 bit Windows (user-mode or kernel) on any CPU with the Microsoft C compiler

           (!defined _WIN64 && defined _WIN32)  indicates 32 bit Windows
           _MSC_VER                             indicates Microsoft C compiler
  */

  static LFDS_INLINE unsigned char lfds_abstraction_dcas( volatile lfds_atom_t *destination, lfds_atom_t *exchange, lfds_atom_t *compare )
  {
    __int64
      original_compare;

    assert( destination != NULL );
    assert( exchange != NULL );
    assert( compare != NULL );

    *(__int64 *) &original_compare = *(__int64 *) compare;

    LFDS_BARRIER_COMPILER_FULL;

    *(__int64 *) compare = _InterlockedCompareExchange64( (volatile __int64 *) destination, *(__int64 *) exchange, *(__int64 *) compare );

    LFDS_BARRIER_COMPILER_FULL;

    return( (unsigned char) (*(__int64 *) compare == *(__int64 *) &original_compare) );
  }

  static LFDS_INLINE lfds_atom_t lfds_abstraction_increment( volatile lfds_atom_t *value )
  {
    lfds_atom_t
      rv;

    assert( value != NULL );

    LFDS_BARRIER_COMPILER_FULL;

    rv = (lfds_atom_t) _InterlockedIncrement( (long int *) value );

    LFDS_BARRIER_COMPILER_FULL;

    return( rv );
  }

#endif





/****************************************************************************/
#if (defined HEART_PLAT_LINUX && defined HEART_64BIT)

  /* TRD : any OS on x64 with GCC

           __x86_64__  indicates x64
           __GNUC__    indicates GCC
  */

  static LFDS_INLINE unsigned char lfds_abstraction_dcas( volatile lfds_atom_t *destination, lfds_atom_t *exchange, lfds_atom_t *compare )
  {
    unsigned char
      cas_result;

    assert( destination != NULL );
    assert( exchange != NULL );
    assert( compare != NULL );

    // TRD : __asm__ with "memory" in the clobber list is for GCC a full compiler barrier
    __asm__ __volatile__
    (
      "lock;"           // make cmpxchg16b atomic
      "cmpxchg16b %0;"  // cmpxchg16b sets ZF on success
      "setz       %3;"  // if ZF set, set cas_result to 1

      // output
      : "+m" (*(volatile lfds_atom_t (*)[2]) destination), "+a" (*compare), "+d" (*(compare+1)), "=q" (cas_result)

      // input
      : "b" (*exchange), "c" (*(exchange+1))

      // clobbered
      : "cc", "memory"
    );

    return( cas_result );
  }

#endif





/****************************************************************************/
#if (defined HEART_PLAT_LINUX && defined HEART_32BIT)

  /* TRD : any OS on x86 or ARM with GCC 4.1.0 or better

           GCC 4.1.0 introduced the __sync_*() atomic intrinsics

           __GNUC__ / __GNUC_MINOR__ / __GNUC_PATCHLEVEL__  indicates GCC and which version
  */

  static LFDS_INLINE unsigned char lfds_abstraction_dcas( volatile lfds_atom_t *destination, lfds_atom_t *exchange, lfds_atom_t *compare )
  {
    unsigned char
      cas_result = 0;

    unsigned long long int
      original_destination;

    assert( destination != NULL );
    assert( exchange != NULL );
    assert( compare != NULL );

    LFDS_BARRIER_COMPILER_FULL;

    original_destination = __sync_val_compare_and_swap( (volatile unsigned long long int *) destination, *(unsigned long long int *) compare, *(unsigned long long int *) exchange );

    LFDS_BARRIER_COMPILER_FULL;

    if( original_destination == *(unsigned long long int *) compare )
      cas_result = 1;

    *(unsigned long long int *) compare = original_destination;

    return( cas_result );
  }

#endif