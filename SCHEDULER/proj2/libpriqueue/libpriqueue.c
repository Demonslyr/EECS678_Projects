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
  q->count = 0;
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

  //printf("count: %d\n", q->count);

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
    q->count--;
    priqueue_reset_index(q->head->next,1);
  }
  else
  {
    q->head = NULL;
  }
  
  return node->data;
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
  if(q->head==NULL)
  {
    return 0;
  }

  struct priqueue_node *first = NULL;//node before first deleted node. This is for the priqueue_reset_index funciton.
  struct priqueue_node *prev = NULL;//Node before the current node
  struct priqueue_node *curr = q->head;//current node being checked
  
  int removed = 0;//Number of entries removed

  while(curr!=NULL)
  {
    if(curr->data == ptr)
    {
      if(prev==NULL)//a deletion but prev is null meaning we're deleting the head of the list so use special rules becasue the root is a different type than a node
      {
        q->head = q->head->next;
        curr = q-> head;
      }
      else//not deleting the head
      {
        if(first==NULL && prev!=NULL)//first deletion but prev is not null meaning we're deleting an arbitrary entry
        {
          first=prev;
        }

        prev->next = curr->next;
        free(curr);// it here//delete curr here;
        curr = prev->next;
      }
      q->count--;
      removed++;
    }
    else
    {
      prev=prev->next;
      curr=curr->next;
    }
  }
  if(removed > 0)//meaning there were deletions
  {
  priqueue_reset_index(first,first->id+1);//fix the indexing
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
  struct priqueue_node *node = q->head;
  struct priqueue_node *prev = NULL;

  while( node != NULL )
  {
    if( node->id == index )
    {
      //If we are the head, set head to next node 
      //If not change the previous nodes next pointer to our next pointer
      if( q->head == node )
      {
        q->head == node->next;
      }
      else//if we aren't the head, prev should never be NULL
      {
        prev->next = node->next;
      }

      //If we are tail, set tail to previous node
      if( q->tail == node )
      {
        q->tail = prev;
      }
      
      priqueue_reset_index( node->next, node->id );

      struct priqueue_node *tmp = node;

      free(node);

      q->count--;

      return tmp;
    }

    prev = node;
    node = node->next;
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
	return q->count;
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

  //free the queue
  free(q);
}
