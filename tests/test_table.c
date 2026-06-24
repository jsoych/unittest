#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_macros.h"
#include "registry.h"
#include "table.h"
#include "unittest.h"

#define MID_NUM 2048
#define BIG_NUM 1000000

static int entry_check(void)
{
    if (table_entry_check(0, NULL))
        return TEST_FAILURE;

    int value = 42;
    if (table_entry_check(-33, &value))
        return TEST_FAILURE;

    if (!table_entry_check(13, &value))
        return TEST_FAILURE;

    return TEST_SUCCESS;
}
REGISTER_TEST("table_entry_check", entry_check);

static int entry_ok(void)
{
    struct table_entry entry = TABLE_ENTRY_EMPTY;
    if (table_entry_ok(&entry))
        return TEST_FAILURE;

    entry.id = 42;
    if (table_entry_ok(&entry))
        return TEST_FAILURE;

    int value = 33;
    entry.value = &value;
    if (!table_entry_ok(&entry))
        return TEST_FAILURE;

    entry.id = -13;
    if (table_entry_ok(&entry))
        return TEST_FAILURE;

    return TEST_SUCCESS;
}
REGISTER_TEST("table_entry_ok", entry_ok);

static int create(void)
{
    Table *table = table_create();
    if (!table)
    {
        PRINT_ERROR("failed to create table");
        goto failure;
    }
    table_destroy(table);

    return TEST_SUCCESS;

failure:
    table_destroy(table);
    return TEST_FAILURE;
}
REGISTER_TEST("table_create", create);

static int destroy(void)
{
    table_destroy(NULL);

    Table *table = table_create();
    table_destroy(table);

    return TEST_SUCCESS;
}
REGISTER_TEST("table_destroy", destroy);

static int insert(void)
{
    Table *table = table_create();
    if (table_insert(table, -42, NULL) == 0)
    {
        PRINT_ERROR("unexpected result");
        goto failure;
    }

    if (table_insert(table, 0, NULL) == 0)
    {
        PRINT_ERROR("inserted NULL valued entry");
        goto failure;
    }

    int entry = 42;
    if (table_insert(table, 33, &entry) != 0)
    {
        PRINT_ERROR("failed to insert");
        goto failure;
    }

    table_destroy(table);
    return TEST_SUCCESS;

failure:
    table_destroy(table);
    return TEST_FAILURE;
}
REGISTER_TEST("table_insert", insert);

static Table *dummy_table(int num, void *entry)
{
    Table *table = table_create();
    for (int i = 0; i < num; i++)
        (void)table_insert(table, i, entry);
    return table;
}

static int delete(void)
{
    int entry = 42;
    Table *table = dummy_table(MID_NUM, &entry);
    for (int id = 0; id < MID_NUM; id++)
        table_delete(table, id);
    table_destroy(table);
    return TEST_SUCCESS;
}
REGISTER_TEST("table_delete", delete);

static int init_iter(void)
{
    int entry = 42;
    Table *table = dummy_table(1, &entry);
    table_init_iter(table);
    table_destroy(table);
    return TEST_SUCCESS;
}
REGISTER_TEST("table_init_iter", init_iter);

static int next_iter(void)
{
    int value[MID_NUM] = {0};
    Table *table = table_create();
    for (int i = 1; i < MID_NUM + 1; i++)
        if (table_insert(table, i, &value[i - 1]) != 0)
        {
            PRINT_ERROR("failed to insert value (%d)", i);
            goto error;
        }

    // Check and count each entry
    int count = 0;
    table_init_iter(table);
    struct table_entry entry = table_next_iter(table);
    while (entry.value)
    {
        int *ev = (int *)entry.value;
        if (*ev != 0)
        {
            PRINT_ERROR("unexpected value (%d)", *ev);
            goto failure;
        }

        // Mark entry
        *ev = count++;
        entry = table_next_iter(table);
    }

    if (count != MID_NUM)
    {
        PRINT_ERROR("unexpected count (%d)", count);
        goto failure;
    }

    count = 0;
    table_init_iter(table);
    entry = table_next_iter(table);
    while (entry.value)
    {
        int *ev = (int *)entry.value;
        if (*ev != count)
        {
            PRINT_ERROR("unexpected value (%d, %d)", *ev, count);
            goto failure;
        }
        count++;
        entry = table_next_iter(table);
    }

    int ndel = 0;
    for (int i = 1; i <= MID_NUM; i += 41)
    {
        table_delete(table, i);
        ndel++;
    }

    count = 0;
    table_init_iter(table);
    entry = table_next_iter(table);
    while (entry.value)
    {
        count++;
        entry = table_next_iter(table);
    }

    if (count != (MID_NUM - ndel))
    {
        PRINT_ERROR("unexpected value (%d), expected (%d)",
                    count, (MID_NUM - ndel));
        goto failure;
    }

    table_destroy(table);
    return TEST_SUCCESS;

failure:
    table_destroy(table);
    return TEST_FAILURE;

error:
    table_destroy(table);
    return TEST_ERROR;
}
REGISTER_TEST("table_next_iter", next_iter);
