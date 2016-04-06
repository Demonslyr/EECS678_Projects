/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

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

  q->head = q->tail = NULL;
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

  q->count++;
  node->next = NULL;
  node->data = ptr;

  if( q->head == NULL )
  {
    node->id = 0;
    q->head = node;
  }
  else
  {
    node->id = q->tail->id + 1;
    q->tail->next = node;//set current tails next node to the node we are adding
  }

  q->tail = node;//set tail to new node

  int tmp = node->id;
  return tmp;
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
  if( q->count == 0 )
  {
    return NULL;
  }

  return q->head;
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
  
  if(q->head->next != NULL)
  {
    q->head = q->head->next;
    priqueue_reset_index(q->head->next,1);
  }
  else
  {
    q->head = NULL;
  }
  
  return node;
}

void priqueue_reset_index(priqueue_node *n, int count)
{
  n->id = count;
  if(n->next != NULL)
  {
    priqueue_reset_index(n->next, count+1);
  }
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
  
  while(node != NULL)
  {
    if(node->id == index)
    {
      return node;
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
	return 0;
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
	return 0;
}


/**
  Returns the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return 0;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{

}
