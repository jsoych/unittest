#include <errno.h>
#include <stdlib.h>

#include "print_macros.h"
#include "table.h"

#define LOAD_FACTOR 2
#define TABLE_CAPACITY 8

struct table_bucket
{
	/*
	 * Invariants:
	 * - id >= 0
	 * - entry is not NULL
	 * - The last bucket in the list next is NULL
	 */

	struct table_entry entry;

	struct table_bucket *next;
	struct table_bucket *iter_next;
	struct table_bucket *iter_prev;
};

struct table_iter
{
	/*
	 * - The current bucket is either NULL or points to the current bucket
	 * - The head points to either NULL or the first bucket in the list
	 * - The tail points to either NULL or the last bucket in the list
	 * - The head bucket previous iterator bucket is always NULL
	 * - The tail bucket next iterator bucket is always NULL
	 */

	struct table_bucket *curr;
	struct table_bucket *head;
	struct table_bucket *tail;
};

struct Table
{
	/*
	 * Invariants:
	 * - each bucket id in the Table is unique
	 * - every bucket points to the head of the linked list
	 * - every linked list ends with NULL
	 */

	int size;
	int capacity;
	struct table_bucket **bucket;

	struct table_iter iter;
};

/*
 * Initializes the iterator with given bucket.
 */
static void iter_init(struct table_iter *it)
{
	it->curr = NULL;
	it->head = NULL;
	it->tail = NULL;
}

/*
 * Unlinks the bucket from the iterator.
 */
static inline void unlink(struct table_bucket *bkt)
{
	if (!bkt)
		return;
	if (bkt->iter_next)
		bkt->iter_next->iter_prev = bkt->iter_prev;
	if (bkt->iter_prev)
		bkt->iter_prev->iter_next = bkt->iter_next;
	bkt->iter_next = NULL;
	bkt->iter_prev = NULL;
}

/*
 * Links the given buckets together.
 */
static inline void link(struct table_bucket *from, struct table_bucket *to)
{
	if (from)
		from->iter_next = to;
	if (to)
		to->iter_prev = from;
}

/*
 * Adds the bucket to the iterator the end of the iterator list.
 */
static void iter_add(struct table_iter *it, struct table_bucket *bkt)
{
	if (!bkt)
		return;
	link(it->tail, bkt);
	if (!it->head)
	{
		it->head = bkt;
		it->head->iter_prev = NULL;
	}
	it->tail = bkt;
	it->tail->iter_next = NULL;
}

/*
 * Removes the bucket from the iterator list.
 */
static void iter_remove(struct table_iter *it, struct table_bucket *bkt)
{
	if (!bkt)
		return;
	if (it->head == bkt)
	{
		it->head = it->head->iter_next;
		if (it->head)
			it->head->iter_prev = NULL;
	}
	if (it->tail == bkt)
	{
		it->tail = it->tail->iter_prev;
		if (it->tail)
			it->tail->iter_next = NULL;
	}
	unlink(bkt);
}

Table *table_create(void)
{
	Table *table = malloc(sizeof(Table));
	if (!table)
	{
		PRINT_SYSERR("malloc", ENOMEM);
		return NULL;
	}
	table->size = 0;
	table->capacity = TABLE_CAPACITY;
	table->bucket = calloc(TABLE_CAPACITY, sizeof(struct table_bucket *));
	if (!table->bucket)
	{
		PRINT_SYSERR("calloc", ENOMEM);
		free(table);
		return NULL;
	}
	iter_init(&table->iter);
	return table;
}

void table_destroy(Table *table)
{
	if (!table)
		return;
	for (int i = 0; i < table->capacity; i++)
	{
		struct table_bucket *curr = table->bucket[i];
		while (curr)
		{
			struct table_bucket *prev = curr;
			curr = curr->next;
			free(prev);
		}
	}
	free(table->bucket);
	free(table);
}

static int grow_bucket(Table *table)
{
	int capacity = 2 * table->capacity;
	struct table_bucket **bucket =
		calloc(capacity, sizeof(struct table_bucket *));
	if (!bucket)
	{
		PRINT_SYSERR("calloc", ENOMEM);
		return ENOMEM;
	}

	struct table_bucket *curr = table->iter.head;
	while (curr)
	{
		struct table_bucket **last = &bucket[curr->entry.id % capacity];
		while (*last)
			last = &(*last)->next;
		curr->next = NULL;
		*last = curr;
		curr = curr->iter_next;
	}

	free(table->bucket);
	table->bucket = bucket;
	table->capacity = capacity;
	return 0;
}

int table_insert(Table *table, int id, void *value)
{
	if (!table_entry_check(id, value))
	{
		PRINT_WARNING("invalid arguments");
		return -1;
	}

	if (table->size / table->capacity > LOAD_FACTOR)
	{
		if (grow_bucket(table) != 0)
		{
			PRINT_ERROR("unable to grow bucket");
			return -1;
		}
	}

	/* Check for duplicate id */
	struct table_bucket **last = &table->bucket[id % table->capacity];
	while (*last)
	{
		if ((*last)->entry.id == id)
		{
			PRINT_WARNING("duplicate id (%d)", id);
			return -1;
		}
		last = &(*last)->next;
	}

	/* Create bucket and add it */
	struct table_bucket *bucket = malloc(sizeof(struct table_bucket));
	if (!bucket)
	{
		PRINT_SYSERR("malloc", ENOMEM);
		return ENOMEM;
	}

	bucket->entry.id = id;
	bucket->entry.value = value;
	bucket->next = NULL;

	*last = bucket;
	iter_add(&table->iter, bucket);
	return 0;
}

void table_delete(Table *table, int id)
{
	struct table_bucket **curr = &table->bucket[id % table->capacity];
	while (*curr)
	{
		if ((*curr)->entry.id == id)
		{
			struct table_bucket *bucket = *curr;
			*curr = (*curr)->next;
			iter_remove(&table->iter, bucket);
			free(bucket);
			return;
		}
		curr = &(*curr)->next;
	}
}

struct table_entry table_lookup(Table *table, int id)
{
	struct table_bucket *curr = table->bucket[id % table->capacity];
	while (curr)
	{
		if (curr->entry.id == id)
			return curr->entry;
		curr = curr->next;
	}
	return TABLE_ENTRY_EMPTY;
}

void *table_get(Table *table, int id)
{
	struct table_entry entry = table_lookup(table, id);
	if (!table_entry_ok(&entry))
		return NULL;
	return entry.value;
}

void table_init_iter(Table *table)
{
	table->iter.curr = table->iter.head;
}

struct table_entry table_next_iter(Table *table)
{
	struct table_bucket *bkt = table->iter.curr;
	if (!bkt)
		return TABLE_ENTRY_EMPTY;
	table->iter.curr = bkt->iter_next;
	return bkt->entry;
}
