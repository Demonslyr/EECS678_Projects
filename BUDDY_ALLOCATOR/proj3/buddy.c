/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	int index;
	int order;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		g_pages[i].index = i;
		g_pages[i].order = -1;
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
	g_pages[0].order = MAX_ORDER;
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int order, iter;
	order = find_order_needed(size);

	iter = order;

	if( order < 0 )
	{
		return NULL;
	}

	while( list_empty( &free_area[iter] ) )
	{
		iter ++;
	}

	while( iter > order )
	{
		if( !list_empty( &free_area[iter] ) )
		{
			page_t *pg = list_entry( free_area[iter].next, page_t, list );

			int index = pg->index;
			g_pages[index].order = iter-1;

			int buddy_index = ADDR_TO_PAGE( BUDDY_ADDR( PAGE_TO_ADDR(index), (iter-1) ) );
			g_pages[buddy_index].order = iter-1;
			
			list_move( free_area[iter].next, free_area[iter-1].next );
			list_add_tail( &g_pages[buddy_index].list, &free_area[ iter-1 ] );

			//printf( "\nsize: %d   index: %d   buddy: %d\nindex_order: %d   buddy_order: %d\n", size, g_pages[index].index, g_pages[buddy_index].index, g_pages[index].order, g_pages[buddy_index].order);
			iter--;
		}
		else
		{
			return NULL;
		}
	}

	struct list_head * holder = free_area[order].next;
	list_del( free_area[order].next );
	return holder;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int move = 0;
	page_t * pg = list_entry( addr, page_t, list );

	while( pg->order <= MAX_ORDER )
	{
		int found = 0;
		int buddy_index = ADDR_TO_PAGE(BUDDY_ADDR( PAGE_TO_ADDR(pg->index), pg->order ) );
		//printf("\norder: %d    index: %d   buddy index: %d\n", pg->order, pg->index, buddy_index);

		struct list_head * i;
		list_for_each( i, &free_area[pg->order] )
		{
			if( buddy_index == list_entry( i, page_t, list )->index )//if buddy is in list it is free
			{	
				found = 1;
				//printf("%d\n", pg->index);
				pg->order++;
				if( pg->index < buddy_index )
				{
					if( move == 1 )
					{
						list_move(&g_pages[pg->index].list, &free_area[pg->order] );
					}
					else
					{
						list_add( &g_pages[pg->index].list, &free_area[pg->order] );
					}
					list_del( &g_pages[buddy_index].list );
				}
				else
				{
					list_move( &g_pages[buddy_index].list, &free_area[pg->order] );
                    
                    //////////////////////////////////////////
                    if(move == 1)
                    {
                    list_del( &g_pages[pg->index].list );
                    }
                    
                    pg = &g_pages[buddy_index];//list_entry(PAGE_TO_ADDR(buddy_index),page_t, list);
                    pg->order++;//increase "buddy's" order
                    ///////////////////////////////////////////
                    
				}
				move = 1;
				break;
			}
		}
		if( !found )
		{
			if( move )
			{
				list_move( &g_pages[pg->index].list, &free_area[pg->order]);
			}
			else
			{
				list_add( &g_pages[pg->index].list, &free_area[pg->order] );
			}
			break;
		}

	}
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	//printf("**");
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}

/**
 * Find needed order
 * 
 * returns the order we need
 */
int find_order_needed(int size)
{
	int o;
	for( o = MIN_ORDER; o <= MAX_ORDER; o++ )
	{
		if( (1<<o) >= size )
		{
			return o;
		}
	}

	return -1;
}
