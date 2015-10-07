/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "lfds/lfds.h"
#include "lfds/lfds_utils.cpp"

#define LFDS_FREELIST_POINTER 0
#define LFDS_FREELIST_COUNTER 1
#define LFDS_FREELIST_PAC_SIZE 2

#pragma pack( push, LFDS_ALIGN_DOUBLE_POINTER )

struct lfds_freelist_state
{
  struct lfds_freelist_element
    *volatile top[LFDS_FREELIST_PAC_SIZE];

  int
    (*user_data_init_function)( void **user_data, void *user_state );

  void
    *user_state;

  lfds_atom_t
    aba_counter,
    element_count;
};

struct lfds_freelist_element
{
  struct lfds_freelist_element
    *next[LFDS_FREELIST_PAC_SIZE];

  void
    *user_data;
};

#pragma pack( pop )

/****************************************************************************/
int lfds_freelist_new( struct lfds_freelist_state **fs, lfds_atom_t number_elements, int (*user_data_init_function)(void **user_data, void *user_state), void *user_state )
{
  int
    rv = 0;

  lfds_atom_t
    element_count;

  assert( fs != NULL );
  // TRD : number_elements can be any value in its range
  // TRD : user_data_init_function can be NULL

  *fs = (struct lfds_freelist_state *) lfds_liblfds_aligned_malloc( sizeof(struct lfds_freelist_state), LFDS_ALIGN_DOUBLE_POINTER );

  if( (*fs) != NULL )
  {
    (*fs)->top[LFDS_FREELIST_POINTER] = NULL;
    (*fs)->top[LFDS_FREELIST_COUNTER] = 0;
    (*fs)->user_data_init_function = user_data_init_function;
    (*fs)->user_state = user_state;
    (*fs)->aba_counter = 0;
    (*fs)->element_count = 0;

    element_count = lfds_freelist_new_elements( *fs, number_elements );

    if( element_count == number_elements )
      rv = 1;

    if( element_count != number_elements )
    {
      lfds_liblfds_aligned_free( (*fs) );
      *fs = NULL;
    }
  }

  LFDS_BARRIER_STORE;

  return( rv );
}

lfds_atom_t lfds_freelist_internal_new_element( struct lfds_freelist_state *fs, struct lfds_freelist_element **fe )
{
  lfds_atom_t
    rv = 0;

  assert( fs != NULL );
  assert( fe != NULL );

  /* TRD : basically, does what you'd expect;
           allocates an element
           calls the user init function
           if anything fails, cleans up,
           sets *fe to NULL
           and returns 0
  */

  *fe = (struct lfds_freelist_element *) lfds_liblfds_aligned_malloc( sizeof(struct lfds_freelist_element), LFDS_ALIGN_DOUBLE_POINTER );

  if( *fe != NULL )
  {
    if( fs->user_data_init_function == NULL )
    {
      (*fe)->user_data = NULL;
      rv = 1;
    }

    if( fs->user_data_init_function != NULL )
    {
      rv = fs->user_data_init_function( &(*fe)->user_data, fs->user_state );

      if( rv == 0 )
      {
        lfds_liblfds_aligned_free( *fe );
        *fe = NULL;
      }
    }
  }

  if( rv == 1 )
    lfds_abstraction_increment( (lfds_atom_t *) &fs->element_count );

  return( rv );
}

lfds_atom_t lfds_freelist_new_elements(struct lfds_freelist_state *fs, lfds_atom_t number_elements)
{
	struct lfds_freelist_element
		*fe;

	lfds_atom_t
		loop,
		count = 0;

	assert(fs != NULL);
	// TRD : number_elements can be any value in its range
	// TRD : user_data_init_function can be NULL

	for (loop = 0; loop < number_elements; loop++)
		if (lfds_freelist_internal_new_element(fs, &fe))
		{
			lfds_freelist_push(fs, fe);
			count++;
		}

	return(count);
}

/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds_freelist_use( struct lfds_freelist_state *fs )
{
  assert( fs != NULL );

  LFDS_BARRIER_LOAD;

  return;
}

#pragma warning( default : 4100 )

/****************************************************************************/
struct lfds_freelist_element *lfds_freelist_pop( struct lfds_freelist_state *fs, struct lfds_freelist_element **fe )
{
  LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_freelist_element
    *fe_local[LFDS_FREELIST_PAC_SIZE];

  assert( fs != NULL );
  assert( fe != NULL );

  LFDS_BARRIER_LOAD;

  fe_local[LFDS_FREELIST_COUNTER] = fs->top[LFDS_FREELIST_COUNTER];
  fe_local[LFDS_FREELIST_POINTER] = fs->top[LFDS_FREELIST_POINTER];

  /* TRD : note that lfds_abstraction_dcas loads the original value of the destination (fs->top) into the compare (fe_local)
           (this happens of course after the CAS itself has occurred inside lfds_abstraction_dcas)
  */

  do
  {
    if( fe_local[LFDS_FREELIST_POINTER] == NULL )
    {
      *fe = NULL;
      return( *fe );
    }
  }
  while( 0 == lfds_abstraction_dcas((volatile lfds_atom_t *) fs->top, (lfds_atom_t *) fe_local[LFDS_FREELIST_POINTER]->next, (lfds_atom_t *) fe_local) );

  *fe = (struct lfds_freelist_element *) fe_local[LFDS_FREELIST_POINTER];

  return( *fe );
}

/****************************************************************************/
void lfds_freelist_push( struct lfds_freelist_state *fs, struct lfds_freelist_element *fe )
{
  LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_freelist_element
    *fe_local[LFDS_FREELIST_PAC_SIZE],
    *original_fe_next[LFDS_FREELIST_PAC_SIZE];

  assert( fs != NULL );
  assert( fe != NULL );

  LFDS_BARRIER_LOAD;

  fe_local[LFDS_FREELIST_POINTER] = fe;
  fe_local[LFDS_FREELIST_COUNTER] = (struct lfds_freelist_element *) lfds_abstraction_increment( (lfds_atom_t *) &fs->aba_counter );

  original_fe_next[LFDS_FREELIST_POINTER] = fs->top[LFDS_FREELIST_POINTER];
  original_fe_next[LFDS_FREELIST_COUNTER] = fs->top[LFDS_FREELIST_COUNTER];

  /* TRD : note that lfds_abstraction_dcas loads the original value of the destination (fs->top) into the compare (original_fe_next)
           (this happens of course after the CAS itself has occurred inside lfds_abstraction_dcas)
           this then causes us in our loop, should we repeat it, to update fe_local->next to a more
           up-to-date version of the head of the lfds_freelist
  */

  do
  {
    fe_local[LFDS_FREELIST_POINTER]->next[LFDS_FREELIST_POINTER] = original_fe_next[LFDS_FREELIST_POINTER];
    fe_local[LFDS_FREELIST_POINTER]->next[LFDS_FREELIST_COUNTER] = original_fe_next[LFDS_FREELIST_COUNTER];
  }
  while( 0 == lfds_abstraction_dcas((volatile lfds_atom_t *) fs->top, (lfds_atom_t *) fe_local, (lfds_atom_t *) original_fe_next) );

  return;
}

/****************************************************************************/
void *lfds_freelist_get_user_data_from_element( struct lfds_freelist_element *fe, void **user_data )
{
  assert( fe != NULL );
  // TRD : user_data can be NULL

  LFDS_BARRIER_LOAD;

  if( user_data != NULL )
    *user_data = fe->user_data;

  return( fe->user_data );
}

/****************************************************************************/
void lfds_freelist_set_user_data_in_element( struct lfds_freelist_element *fe, void *user_data )
{
  assert( fe != NULL );
  // TRD : user_data can be NULL

  fe->user_data = user_data;

  LFDS_BARRIER_STORE;

  return;
}

/****************************************************************************/
void lfds_freelist_delete( struct lfds_freelist_state *fs, void (*user_data_delete_function)(void *user_data, void *user_state), void *user_state )
{
  struct lfds_freelist_element
    *fe;

  void
    *user_data;

  assert( fs != NULL );
  // TRD : user_data_delete_function can be NULL
  // TRD : user_state can be NULL

  // TRD : leading load barrier not required as it will be performed by the pop

  while( lfds_freelist_pop(fs, &fe) )
  {
    if( user_data_delete_function != NULL )
    {
      lfds_freelist_get_user_data_from_element( fe, &user_data );
      user_data_delete_function( user_data, user_state );
    }

    lfds_liblfds_aligned_free( fe );
  }

  lfds_liblfds_aligned_free( fs );

  return;
}

void lfds_freelist_internal_validate( struct lfds_freelist_state *fs, struct lfds_validation_info *vi, enum lfds_data_structure_validity *lfds_freelist_validity )
{
  struct lfds_freelist_element
    *fe,
    *fe_slow,
    *fe_fast;

  lfds_atom_t
    element_count = 0;

  assert( fs != NULL );
  // TRD : vi can be NULL
  assert( lfds_freelist_validity != NULL );

  *lfds_freelist_validity = LFDS_VALIDITY_VALID;

  fe_slow = fe_fast = (struct lfds_freelist_element *) fs->top[LFDS_FREELIST_POINTER];

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the top of the lfds_freelist
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the lfds_freelist)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop
  */

  if( fe_slow != NULL )
    do
    {
      fe_slow = fe_slow->next[LFDS_FREELIST_POINTER];

      if( fe_fast != NULL )
        fe_fast = fe_fast->next[LFDS_FREELIST_POINTER];

      if( fe_fast != NULL )
        fe_fast = fe_fast->next[LFDS_FREELIST_POINTER];
    }
    while( fe_slow != NULL && fe_fast != fe_slow );

  if( fe_fast != NULL && fe_slow != NULL && fe_fast == fe_slow )
    *lfds_freelist_validity = LFDS_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds_freelist_validity == LFDS_VALIDITY_VALID && vi != NULL )
  {
    fe = (struct lfds_freelist_element *) fs->top[LFDS_FREELIST_POINTER];

    while( fe != NULL )
    {
      element_count++;
      fe = (struct lfds_freelist_element *) fe->next[LFDS_FREELIST_POINTER];
    }

    if( element_count < vi->min_elements )
      *lfds_freelist_validity = LFDS_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( element_count > vi->max_elements )
      *lfds_freelist_validity = LFDS_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

void lfds_freelist_query( struct lfds_freelist_state *fs, enum lfds_freelist_query_type query_type, void *query_input, void *query_output )
{
  assert( fs != NULL );
  // TRD : query type can be any value in its range
  // TRD : query_input can be NULL in some cases
  assert( query_output != NULL );

  LFDS_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS_FREELIST_QUERY_ELEMENT_COUNT:
      assert( query_input == NULL );

      *(lfds_atom_t *) query_output = fs->element_count;
    break;

    case LFDS_FREELIST_QUERY_VALIDATE:
      // TRD : query_input can be NULL

      lfds_freelist_internal_validate( fs, (struct lfds_validation_info *) query_input, (enum lfds_data_structure_validity *) query_output );
    break;
  }

  return;
}