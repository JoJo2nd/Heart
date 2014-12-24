/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "lfds/lfds.h"
#include "lfds/lfds_utils.cpp"

#define LFDS_QUEUE_STATE_UNKNOWN               -1
#define LFDS_QUEUE_STATE_EMPTY                  0
#define LFDS_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE   1
#define LFDS_QUEUE_STATE_ATTEMPT_DELFDS_QUEUE        2

#define LFDS_QUEUE_POINTER 0
#define LFDS_QUEUE_COUNTER 1
#define LFDS_QUEUE_PAC_SIZE 2

#pragma pack( push, LFDS_ALIGN_DOUBLE_POINTER )

struct lfds_queue_state
{
  struct lfds_queue_element
    *volatile enqueue[LFDS_QUEUE_PAC_SIZE],
    *volatile dequeue[LFDS_QUEUE_PAC_SIZE];

  lfds_atom_t
    aba_counter;

  struct lfds_freelist_state
    *fs;
};

struct lfds_queue_element
{
  // TRD : next in a lfds_queue requires volatile as it is target of CAS
  struct lfds_queue_element
    *volatile next[LFDS_QUEUE_PAC_SIZE];

  struct lfds_freelist_element
    *fe;

  void
    *user_data;
};

#pragma pack( pop )


int lfds_queue_internal_freelist_init_function( void **user_data, void *user_state );
void lfds_queue_internal_freelist_delete_function( void *user_data, void *user_state );
void lfds_queue_internal_new_element_from_freelist( struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE], void *user_data );
void lfds_queue_internal_guaranteed_new_element_from_freelist( struct lfds_queue_state *qs, struct lfds_queue_element * qe[LFDS_QUEUE_PAC_SIZE], void *user_data );
void lfds_queue_internal_init_element( struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE], struct lfds_freelist_element *fe, void *user_data );
void lfds_queue_internal_queue( struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE] );

/****************************************************************************/
int lfds_queue_new( struct lfds_queue_state **qs, lfds_atom_t number_elements )
{
  int
    rv = 0;

  struct lfds_queue_element
    *qe[LFDS_QUEUE_PAC_SIZE];

  assert( qs != NULL );
  // TRD : number_elements can be any value in its range

  *qs = (struct lfds_queue_state *) lfds_liblfds_aligned_malloc( sizeof(struct lfds_queue_state), LFDS_ALIGN_DOUBLE_POINTER );

  if( *qs != NULL )
  {
    // TRD : the size of the lfds_freelist is the size of the lfds_queue (+1 for the leading dummy element, which is hidden from the caller)
    lfds_freelist_new( &(*qs)->fs, number_elements+1, lfds_queue_internal_freelist_init_function, NULL );

    if( (*qs)->fs != NULL )
    {
      lfds_queue_internal_new_element_from_freelist( *qs, qe, NULL );
      (*qs)->enqueue[LFDS_QUEUE_POINTER] = (*qs)->dequeue[LFDS_QUEUE_POINTER] = qe[LFDS_QUEUE_POINTER];
      (*qs)->enqueue[LFDS_QUEUE_COUNTER] = (*qs)->dequeue[LFDS_QUEUE_COUNTER] = 0;
      (*qs)->aba_counter = 0;
      rv = 1;
    }

    if( (*qs)->fs == NULL )
    {
      lfds_liblfds_aligned_free( *qs );
      *qs = NULL;
    }
  }

  LFDS_BARRIER_STORE;

  return( rv );
}

/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds_queue_use( struct lfds_queue_state *qs )
{
  assert( qs != NULL );

  LFDS_BARRIER_LOAD;

  return;
}

#pragma warning( default : 4100 )

/****************************************************************************/
#pragma warning( disable : 4100 )

int lfds_queue_internal_freelist_init_function( void **user_data, void *user_state )
{
  int
    rv = 0;

  assert( user_data != NULL );
  assert( user_state == NULL );

  *user_data = lfds_liblfds_aligned_malloc( sizeof(struct lfds_queue_element), LFDS_ALIGN_DOUBLE_POINTER );

  if( *user_data != NULL )
    rv = 1;

  return( rv );
}

#pragma warning( default : 4100 )

/****************************************************************************/
void lfds_queue_internal_new_element_from_freelist( struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE], void *user_data )
{
  struct lfds_freelist_element
    *fe;

  assert( qs != NULL );
  assert( qe != NULL );
  // TRD : user_data can be any value in its range

  qe[LFDS_QUEUE_POINTER] = NULL;

  lfds_freelist_pop( qs->fs, &fe );

  if( fe != NULL )
    lfds_queue_internal_init_element( qs, qe, fe, user_data );

  return;
}

/****************************************************************************/
void lfds_queue_delete( struct lfds_queue_state *qs, void (*user_data_delete_function)(void *user_data, void *user_state), void *user_state )
{
  void
    *user_data;

  assert( qs != NULL );
  // TRD : user_data_delete_function can be NULL
  // TRD : user_state can be NULL

  // TRD : leading load barrier not required as it will be performed by the dequeue

  while( lfds_queue_dequeue(qs, &user_data) )
    if( user_data_delete_function != NULL )
      user_data_delete_function( user_data, user_state );

  /* TRD : fully dequeuing will leave us
           with a single dummy element
           which both qs->enqueue and qs->dequeue point at
           we push this back onto the lfds_freelist
           before we delete the lfds_freelist
  */

  lfds_freelist_push( qs->fs, qs->enqueue[LFDS_QUEUE_POINTER]->fe );

  lfds_freelist_delete( qs->fs, lfds_queue_internal_freelist_delete_function, NULL );

  lfds_liblfds_aligned_free( qs );

  return;
}

/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds_queue_internal_freelist_delete_function( void *user_data, void *user_state )
{
  assert( user_data != NULL );
  assert( user_state == NULL );

  lfds_liblfds_aligned_free( user_data );

  return;
}

#pragma warning( default : 4100 )

/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds_queue_query( struct lfds_queue_state *qs, enum lfds_queue_query_type query_type, void *query_input, void *query_output )
{
  assert( qs != NULL );
  // TRD : query_type can be any value in its range
  // TRD : query_input can be NULL
  assert( query_output != NULL );

  switch( query_type )
  {
    case LFDS_QUEUE_QUERY_ELEMENT_COUNT:
      assert( query_input == NULL );

      lfds_freelist_query( qs->fs, LFDS_FREELIST_QUERY_ELEMENT_COUNT, NULL, query_output );
    break;
  }

  return;
}

#pragma warning( default : 4100 )

/****************************************************************************/
int lfds_queue_enqueue( struct lfds_queue_state *qs, void *user_data )
{
  LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_queue_element
    *qe[LFDS_QUEUE_PAC_SIZE];

  assert( qs != NULL );
  // TRD : user_data can be NULL

  lfds_queue_internal_new_element_from_freelist( qs, qe, user_data );

  if( qe[LFDS_QUEUE_POINTER] == NULL )
    return( 0 );

  lfds_queue_internal_queue( qs, qe );

  return( 1 );
}





/****************************************************************************/
// int lfds_queue_guaranteed_enqueue( struct lfds_queue_state *qs, void *user_data )
// {
//   LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_queue_element
//     *qe[LFDS_QUEUE_PAC_SIZE];
// 
//   assert( qs != NULL );
//   // TRD : user_data can be NULL
// 
//   lfds_queue_internal_guaranteed_new_element_from_freelist( qs, qe, user_data );
// 
//   if( qe[LFDS_QUEUE_POINTER] == NULL )
//     return( 0 );
// 
//   lfds_queue_internal_queue( qs, qe );
// 
//   return( 1 );
// }





/****************************************************************************/
void lfds_queue_internal_queue( struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE] )
{
  LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_queue_element
    *enqueue[LFDS_QUEUE_PAC_SIZE],
    *next[LFDS_QUEUE_PAC_SIZE];

  unsigned char
    cas_result = 0;

  assert( qs != NULL );
  assert( qe != NULL );

  // TRD : the DCAS operation issues a read and write barrier, so we don't need a read barrier in the do() loop

  LFDS_BARRIER_LOAD;

  do
  {
    enqueue[LFDS_QUEUE_POINTER] = qs->enqueue[LFDS_QUEUE_POINTER];
    enqueue[LFDS_QUEUE_COUNTER] = qs->enqueue[LFDS_QUEUE_COUNTER];

    next[LFDS_QUEUE_POINTER] = enqueue[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_POINTER];
    next[LFDS_QUEUE_COUNTER] = enqueue[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_COUNTER];

    /* TRD : this if() ensures that the next we read, just above,
             really is from qs->enqueue (which we copied into enqueue)
    */

    LFDS_BARRIER_LOAD;

    if( qs->enqueue[LFDS_QUEUE_POINTER] == enqueue[LFDS_QUEUE_POINTER] && qs->enqueue[LFDS_QUEUE_COUNTER] == enqueue[LFDS_QUEUE_COUNTER] )
    {
      if( next[LFDS_QUEUE_POINTER] == NULL )
      {
        qe[LFDS_QUEUE_COUNTER] = next[LFDS_QUEUE_COUNTER] + 1;
        cas_result = lfds_abstraction_dcas( (volatile lfds_atom_t *) enqueue[LFDS_QUEUE_POINTER]->next, (lfds_atom_t *) qe, (lfds_atom_t *) next );
      }
      else
      {
        next[LFDS_QUEUE_COUNTER] = enqueue[LFDS_QUEUE_COUNTER] + 1;
        lfds_abstraction_dcas( (volatile lfds_atom_t *) qs->enqueue, (lfds_atom_t *) next, (lfds_atom_t *) enqueue );
      }
    }
  }
  while( cas_result == 0 );

  qe[LFDS_QUEUE_COUNTER] = enqueue[LFDS_QUEUE_COUNTER] + 1;
  lfds_abstraction_dcas( (volatile lfds_atom_t *) qs->enqueue, (lfds_atom_t *) qe, (lfds_atom_t *) enqueue );

  return;
}





/****************************************************************************/
int lfds_queue_dequeue( struct lfds_queue_state *qs, void **user_data )
{
  LFDS_ALIGN(LFDS_ALIGN_DOUBLE_POINTER) struct lfds_queue_element
    *enqueue[LFDS_QUEUE_PAC_SIZE],
    *dequeue[LFDS_QUEUE_PAC_SIZE],
    *next[LFDS_QUEUE_PAC_SIZE];

  unsigned char
    cas_result = 0;

  int
    rv = 1,
    state = LFDS_QUEUE_STATE_UNKNOWN,
    finished_flag = LOWERED;

  assert( qs != NULL );
  assert( user_data != NULL );

  // TRD : the DCAS operation issues a read and write barrier, so we don't need a read barrier in the do() loop

  LFDS_BARRIER_LOAD;

  do
  {
    dequeue[LFDS_QUEUE_POINTER] = qs->dequeue[LFDS_QUEUE_POINTER];
    dequeue[LFDS_QUEUE_COUNTER] = qs->dequeue[LFDS_QUEUE_COUNTER];

    enqueue[LFDS_QUEUE_POINTER] = qs->enqueue[LFDS_QUEUE_POINTER];
    enqueue[LFDS_QUEUE_COUNTER] = qs->enqueue[LFDS_QUEUE_COUNTER];

    next[LFDS_QUEUE_POINTER] = dequeue[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_POINTER];
    next[LFDS_QUEUE_COUNTER] = dequeue[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_COUNTER];

    /* TRD : confirm that dequeue didn't move between reading it
             and reading its next pointer
    */

    LFDS_BARRIER_LOAD;

    if( dequeue[LFDS_QUEUE_POINTER] == qs->dequeue[LFDS_QUEUE_POINTER] && dequeue[LFDS_QUEUE_COUNTER] == qs->dequeue[LFDS_QUEUE_COUNTER] )
    {
      if( enqueue[LFDS_QUEUE_POINTER] == dequeue[LFDS_QUEUE_POINTER] && next[LFDS_QUEUE_POINTER] == NULL )
        state = LFDS_QUEUE_STATE_EMPTY;

      if( enqueue[LFDS_QUEUE_POINTER] == dequeue[LFDS_QUEUE_POINTER] && next[LFDS_QUEUE_POINTER] != NULL )
        state = LFDS_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE;

      if( enqueue[LFDS_QUEUE_POINTER] != dequeue[LFDS_QUEUE_POINTER] )
        state = LFDS_QUEUE_STATE_ATTEMPT_DELFDS_QUEUE;

      switch( state )
      {
        case LFDS_QUEUE_STATE_EMPTY:
          *user_data = NULL;
          rv = 0;
          finished_flag = RAISED;
        break;

        case LFDS_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE:
          next[LFDS_QUEUE_COUNTER] = enqueue[LFDS_QUEUE_COUNTER] + 1;
          lfds_abstraction_dcas( (volatile lfds_atom_t *) qs->enqueue, (lfds_atom_t *) next, (lfds_atom_t *) enqueue );
        break;

        case LFDS_QUEUE_STATE_ATTEMPT_DELFDS_QUEUE:
          *user_data = next[LFDS_QUEUE_POINTER]->user_data;

          next[LFDS_QUEUE_COUNTER] = dequeue[LFDS_QUEUE_COUNTER] + 1;
          cas_result = lfds_abstraction_dcas( (volatile lfds_atom_t *) qs->dequeue, (lfds_atom_t *) next, (lfds_atom_t *) dequeue );

          if( cas_result == 1 )
            finished_flag = RAISED;
        break;
      }
    }
  }
  while( finished_flag == LOWERED );

  if( cas_result == 1 )
    lfds_freelist_push( qs->fs, dequeue[LFDS_QUEUE_POINTER]->fe );

  return( rv );
}

/****************************************************************************/
void lfds_queue_internal_init_element(struct lfds_queue_state *qs, struct lfds_queue_element *qe[LFDS_QUEUE_PAC_SIZE], struct lfds_freelist_element *fe, void *user_data)
{
    assert(qs != NULL);
    assert(qe != NULL);
    assert(fe != NULL);
    // TRD : user_data can be any value in its range

    lfds_freelist_get_user_data_from_element(fe, (void **)&qe[LFDS_QUEUE_POINTER]);
    qe[LFDS_QUEUE_COUNTER] = (struct lfds_queue_element *) lfds_abstraction_increment((lfds_atom_t *)&qs->aba_counter);

    qe[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_POINTER] = NULL;
    qe[LFDS_QUEUE_POINTER]->next[LFDS_QUEUE_COUNTER] = (struct lfds_queue_element *) lfds_abstraction_increment((lfds_atom_t *)&qs->aba_counter);

    qe[LFDS_QUEUE_POINTER]->fe = fe;
    qe[LFDS_QUEUE_POINTER]->user_data = user_data;

    return;
}