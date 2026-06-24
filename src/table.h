#ifndef TABLE_H
#define TABLE_H

struct table_entry
{
    int id;
    void *value;
};

#define TABLE_ENTRY_EMPTY ((struct table_entry){0})

/**
 * @brief Validates raw input arguments against core table invariants.
 *
 * This function enforces the library-wide design constraint that a valid table
 * record must possess a non-zero identifier and a non-NULL payload pointer.
 * It is typically used for inbound validation prior to memory insertion.
 *
 * @note Implemented as a high-speed inline check to completely eliminate
 *       function call stack frame overhead during rapid input parsing.
 *
 * @param[in] id     The unique identifier key to verify (must be != 0).
 * @param[in] value  The data payload pointer to verify (must be != NULL).
 * @return int       Non-zero (true) if both constraints are satisfied;
 *                   0 (false) if either constraint is violated.
 */
static inline int table_entry_check(int id, const void *value)
{
    return id > 0 && value != NULL;
}

/**
 * @brief Validates a populated table entry structure layout.
 *
 * Evaluates a structural record block to ensure it represents a live, healthy
 * database entity. This serves as the outbound safety shield, instantly
 * detecting uninitialized, corrupted, or empty states.
 *
 * @note Designed specifically to intercept the global `(struct table_entry){0}`
 *       empty sentinel value returned by lookup engines when a search fails.
 *
 * @param[in] entry  Pointer to the table entry structure to verify.
 * @return int       Non-zero (true) if the structure pointer is valid and its
 *                   internal fields satisfy all live-data constraints;
 *                   0 (false) if the entry is NULL or represents an empty sentinel.
 */
static inline int table_entry_ok(const struct table_entry *entry)
{
    return entry && entry->id > 0 && entry->value;
}

typedef struct Table Table;

/*
 * Create a new table and returns it.
 */
Table *table_create(void);

/*
 * Destroys the table and all of its resources.
 */
void table_destroy(Table *table);

/*
 * Inserts the result into the table with id and returns 0, on success and
 * -1, on failure
 */
int table_insert(Table *table, int id, void *entry);

/*
 * Deletes the result by its id.
 */
void table_delete(Table *table, int id);

/*
 * Looks up the entry by its id and returns, if there is a result with
 * given id. Otherwise, returns (struct table_entry){0}.
 */
struct table_entry table_lookup(Table *table, int id);

/*
 * Gets the table value and returns it.
 */
void *table_get(Table *table, int id);

/*
 * Start table iterator.
 */
void table_init_iter(Table *table);

/*
 * Gets the next iterator entry and returns it. Otherwise, returns NULL
 * indicating there is no more entries to return.
 */
struct table_entry table_next_iter(Table *table);

#endif
