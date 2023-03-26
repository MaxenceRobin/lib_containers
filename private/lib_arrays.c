/**
 * @author Maxence ROBIN
 * @brief Provides simple static array manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_arrays.h"
#include "lib_iterators_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

struct array {
        const struct type_info *type;
        size_t len;
        void *data;
};

struct array_it {
        struct iterator it; /* Placed at top for inheritance */
        struct array *array;
        int pos;
};

/* Static functions ----------------------------------------------------------*/

static char *data_offset(const struct array *array, unsigned int pos)
{
        return (char *)array->data + pos * array->type->size;
}

/* API -----------------------------------------------------------------------*/

struct array *array_create(
                const struct type_info *type, size_t count, void *data)
{
        if (!type || !data)
                return NULL;

        if (type->size == 0 || !type->copy || !type->comp || !type->destroy)
                return NULL;

        struct array *array = malloc(sizeof(*array));
        if (!array)
                return NULL;

        array->type = type;
        array->len = count;
        array->data = data;

        return array;
}

void array_destroy(const void *array)
{
        if (!array)
                return;

        free((void *)array);
}

int array_sort(struct array *array)
{
        if (!array)
                return -EINVAL;

        qsort(array->data, array->len, array->type->size, array->type->comp);
        return 0;
}

int array_sort_by(struct array *array, type_comp_cb comp)
{
        if (!array || !comp)
                return -EINVAL;

        qsort(array->data, array->len, array->type->size, comp);
}

ssize_t array_len(const struct array *array)
{
        if (!array)
                return -EINVAL;

        return array->len;
}

void *array_value(struct array *array, unsigned int pos)
{
        if (!array || pos < 0 || array->len <= pos)
                return NULL;

        return data_offset(array, pos);
}

void *array_data(struct array *array)
{
        if (!array)
                return NULL;

        return array->data;
}

/* Iterator API --------------------------------------------------------------*/

static struct iterator_callbacks array_it_cbs;
static struct iterator_callbacks array_rit_cbs;

/* Utility functions -----------------*/

static struct array_it *array_it_create(
                const struct array *array,
                int pos,
                const struct iterator_callbacks *cbs)
{
        struct array_it *a_it = calloc(1, sizeof(*a_it));
        if (!a_it)
                return NULL;

        it_init(&a_it->it, cbs);
        a_it->array = (struct array *)array;
        a_it->pos = pos;

        return a_it;
}

/* Iterator implementation -----------*/

static int array_it_next(struct iterator *it)
{
        struct array_it *a_it = (struct array_it *)it;
        ++a_it->pos;
        return 0;
}

static int array_it_previous(struct iterator *it)
{
        struct array_it *a_it = (struct array_it *)it;
        --a_it->pos;
        return 0;
}

static bool array_it_is_valid(const struct iterator *it)
{
        const struct array_it *a_it = (const struct array_it *)it;
        return (0 <= a_it->pos && a_it->pos < a_it->array->len);
}

static void *array_it_data(const struct iterator *it)
{
        if (!array_it_is_valid(it))
                return NULL;

        const struct array_it *a_it = (const struct array_it *)it;
        return data_offset(a_it->array, a_it->pos);
}

static const struct type_info *array_it_type(const struct iterator *it)
{
        const struct array_it *a_it = (const struct array_it *)it;
        return a_it->array->type;
}

static struct iterator *array_it_dup(const struct iterator *it)
{
        const struct array_it *a_it = (const struct array_it *)it;
        if (!array_it_is_valid(it))
                return NULL;

        struct array_it *dup =
                        array_it_create(a_it->array, a_it->pos, a_it->it.cbs);
        return (struct iterator *)dup;
}

static int array_it_copy(struct iterator *dest, const struct iterator *src)
{
        struct array_it *a_dest = (struct array_it *)dest;
        const struct array_it *a_src = (const struct array_it *)src;

        if (a_dest->array != a_src->array)
                return -EINVAL;

        a_dest->pos = a_src->pos;
        return 0;
}

static void array_it_destroy(const struct iterator *it)
{
        struct array_it *a_it = (struct array_it *)it;
        free(a_it);
}

static struct iterator_callbacks array_it_cbs = {
        .next_cb = array_it_next,
        .previous_cb = array_it_previous,
        .is_valid_cb = array_it_is_valid,
        .data_cb = array_it_data,
        .type_cb = array_it_type,
        .remove_cb = NULL,
        .dup_cb = array_it_dup,
        .copy_cb = array_it_copy,
        .destroy_cb = array_it_destroy
};

static struct iterator_callbacks array_rit_cbs = {
        .next_cb = array_it_previous,
        .previous_cb = array_it_next,
        .is_valid_cb = array_it_is_valid,
        .data_cb = array_it_data,
        .type_cb = array_it_type,
        .remove_cb = NULL,
        .dup_cb = array_it_dup,
        .copy_cb = array_it_copy,
        .destroy_cb = array_it_destroy
};

/* Public API ------------------------*/

struct iterator *array_begin(const struct array *array)
{
        if (!array)
                return NULL;

        struct array_it *a_it = array_it_create(array, 0, &array_it_cbs);
        if (!a_it)
                return NULL;

        return (struct iterator *)a_it;
}

struct iterator *array_end(const struct array *array)
{
        if (!array)
                return NULL;

        struct array_it *a_it =
                        array_it_create(array, array->len - 1, &array_it_cbs);
        if (!a_it)
                return NULL;

        return (struct iterator *)a_it;
}

struct iterator *array_rbegin(const struct array *array)
{
        if (!array)
                return NULL;

        struct array_it *a_it =
                        array_it_create(array, array->len - 1, &array_rit_cbs);
        if (!a_it)
                return NULL;

        return (struct iterator *)a_it;
}

struct iterator *array_rend(const struct array *array)
{
        if (!array)
                return NULL;

        struct array_it *a_it = array_it_create(array, 0, &array_rit_cbs);
        if (!a_it)
                return NULL;

        return (struct iterator *)a_it;
}
