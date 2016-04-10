/** @file queuetest.c
 */

#include <stdio.h>
#include <stdlib.h>

#include "libpriqueue/libpriqueue.h"

typedef struct thing
{
    int integer1;
    int integer2;

} thing;

int compare1(const void * a, const void * b)
{
	const thing * blah1 = a;
	const thing * blah2 = b;
	printf("blah1->integer1: %d\n", blah1->integer1 );
	return ( blah1->integer1 - blah2->integer1 );
}

int compare2(const void * a, const void * b)
{
	const thing * blah1 = a;
	const thing * blah2 = b;
	return ( blah2->integer1 - blah1->integer1 );
}

int main()
{
	priqueue_t q, q2;

	priqueue_init(&q, compare1);
	priqueue_init(&q2, compare2);

	/* Pupulate some data... */
	struct thing *values = (struct thing*)malloc(sizeof(struct thing));

	int i;

	values->integer1 = 10;
	values->integer2 = 5;

	/* Add 5 values, 3 unique. */
	priqueue_offer(&q, values);
	priqueue_offer(&q, values);
	priqueue_offer(&q, values);
	priqueue_offer(&q, values);
	priqueue_offer(&q, values);
	printf("Total elements: %d (expected 5).\n", priqueue_size(&q));

	int val = ((thing *)priqueue_poll(&q))->integer1;
	printf("Top element: %d (expected 12).\n", val);
	printf("Total elements: %d (expected 4).\n", priqueue_size(&q));

	int vals_removed = priqueue_remove(&q, &values);
	printf("Elements removed: %d (expected 2).\n", vals_removed);
	printf("Total elements: %d (expected 2).\n", priqueue_size(&q));

	priqueue_offer(&q, &values[0]);
	priqueue_offer(&q, &values[0]);
	priqueue_offer(&q, &values[0]);

	priqueue_offer(&q2, &values[0]);
	priqueue_offer(&q2, &values[0]);
	priqueue_offer(&q2, &values[0]);

	printf("Elements in order queue (expected 10 13 14 20 30): ");
	for (i = 0; i < priqueue_size(&q); i++)
		printf("%d ", ((thing *)priqueue_at(&q, i))->integer1 );
	printf("\n");

	printf("Elements in reverse order queue (expected 30 20 10): ");
	for (i = 0; i < priqueue_size(&q2); i++)
		printf("%d ", ((thing *)priqueue_at(&q2, i))->integer1 );
	printf("\n");

	priqueue_destroy(&q2);
	priqueue_destroy(&q);

	free(values);

	return 0;
}
