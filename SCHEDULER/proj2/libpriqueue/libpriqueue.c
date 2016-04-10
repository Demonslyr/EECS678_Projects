/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "../libscheduler/libscheduler.h"
#include "libpriqueue.h"
/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  if(NULL == q)
  {
    printf("\n Node creation failed \n");
    
  }

  q->head = NULL;

  q->comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  struct priqueue_node *node = (struct priqueue_node*)malloc(sizeof(struct priqueue_node));

  node->next = NULL;
  node->data = ptr;

  if( q->head == NULL )
  {
    node->id = 0;
    q->head = node;
  }
  else
  {
    struct priqueue_node *curr = q->head;
    struct priqueue_node *prev = NULL;

    while( curr != NULL ) 
    {
      //printf("compare: %d\n", (*q->comparer)( node->data, curr->data));
      if( (*q->comparer)( node->data, curr->data) > 0 )
      {
        if( curr->next == NULL )
        {
            curr->next = node;
            break;
        }
        prev = curr;
        curr = curr->next;
      }
      else
      {
        if( curr == q->head )
        {
          node->next = q->head;
          q->head = node;
        }
        else
        {
          node->next = curr;
          prev->next = node;
        }
        break;
      }
    }
  }

  priqueue_reindex( q );
  //priqueue_print( q );

  return node->id;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  printf("priqueue_peek\n");
  if(q->head == NULL){
      return NULL;
  }
  return q->head->data;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  if( q->head == NULL )
  {
    return NULL;
  }
  
  struct priqueue_node *node = q->head;
  void *data = node->data;
  
  if(q->head->next != NULL)
  {
    free( node );
    q->head = q->head->next;
    priqueue_reindex( q );
  }
  else
  {
    q->head = NULL;
  }
  
  return data;
}

void priqueue_reindex(priqueue_t *q)
{
  int index = 0;
  struct priqueue_node *curr = q->head;

  while( curr != NULL )
  {
    curr->id = index;
    curr = curr->next;
    index++;
  }
}

void priqueue_print(priqueue_t *q)
{
  priqueue_node *curr = q->head;

  while( curr != NULL )
  {
    printf(": [%d] %d ", curr->id, *(int*)curr->data );
    curr = curr->next;
  }
  printf(":\n");
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  struct priqueue_node *node = q->head;
  
  while( node != NULL )
  {
    //printf("while loop: %d\n", node->id );
    if( node->id == index )
    {
      return node->data;
    }
    else
    {
      node = node->next;
    }
  }

	return NULL;
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  //printf("priqueue_remove\n");

  if( q->head == NULL )
  {
    return 0;
  }

  struct priqueue_node *prev = NULL;//Node before the current node
  struct priqueue_node *curr = q->head;//current node being checked

  int removed = 0;//Number of entries removed

  while( curr != NULL )
  {
    if( curr->data == ptr )//( (*q->comparer)( curr->data, ptr) == 0 )
    {
      if( curr == q->head )//we are deleting the head node
      {
        q->head = curr->next;
      }
      else
      {
        prev->next = curr->next;
      }

      prev = curr;
      free(curr);//delete curr here;
      curr = prev->next;
      
      removed++;
    }
    else
    {
      prev=curr;
      curr = curr->next;
    } 
  }
  if(removed > 0)//meaning there were deletions
  {
    priqueue_reindex( q );//fix the indexing
  }

	return removed;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
  struct priqueue_node *curr = q->head;
  struct priqueue_node *prev = NULL;

  while( curr != NULL )
  {
    if( curr->id == index )
    {
      //If we are the head, set head to next node 
      //If not change the previous nodes next pointer to our next pointer
      if( q->head == curr )
      {
        q->head == curr->next;
      }
      else//if we aren't the head, prev should never be NULL
      {
        prev->next = curr->next;
      }

      struct priqueue_node *tmp = curr;
      free(curr);

      priqueue_reindex( q );

      return tmp;
    }

    prev = curr;
    curr = curr->next;
  }

	return NULL;
}


/**
  Returns the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
  struct priqueue_node *curr = q->head;

  while( curr->next != NULL )
  {
    curr = curr->next;
  }

  return (curr->id + 1);
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  struct priqueue_node *node;
  
  //free all nodes inside queue
  while( q->head != NULL )
  {
    node = q->head;
    q->head = node->next;

    free(node);
  }
}
