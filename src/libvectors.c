/**
 * @author Maxence ROBIN
 * @brief Provides simple dynamic array manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libiterators_private.h"
#include "libvectors.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

struct meta {
        const struct type_info *type;
        size_t len;
        size_t capacity;
};

struct vector_it {
        struct iterator it; /* Placed at top for inheritance */
        struct meta *meta;
        int pos;
};

/* Static functions ----------------------------------------------------------*/

/* Conversion functions --------------*/

static struct meta *vector_to_meta(const void *vector)
{
        return vector ?
                (struct meta *)vector - 1
                : NULL;
}

static void *meta_to_vector(const struct meta *meta)
{
        return meta ?
                (void *)(meta + 1)
                : NULL;
}

/* Private utility functions ---------*/

/**
 * @brief Sets the capacity of 'meta' to 'capacity'.
 *
 * @return Pointer to a valid meta, reallocated or not.
 *
 * @note if 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful ot not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static struct meta *set_capacity(struct meta *meta, size_t capacity, int *ret)
{
        struct meta *new_meta;
        int res;

        new_meta = realloc(meta, sizeof(*meta) + meta->type->size * capacity);
        if (!new_meta) {
                res = -ENOMEM;
                goto error_realloc;
        }

        meta = new_meta;
        meta->capacity = capacity;
        res = 0;
error_realloc:
        if (ret)
                *ret = res;

        return meta;
}

static char *data_offset(const struct meta *meta, unsigned int pos)
{
        return (char *)meta_to_vector(meta) + pos * meta->type->size;
}

/**
 * @brief Sets the len of 'meta' to 'len', and reallocates it if needed.
 *
 * @return Pointer to a valid meta, if the length was modified or not.
 *
 * @note if 'ret' is not NULL, its valud will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static struct meta *set_len(struct meta *meta, size_t len, int *ret)
{
        int res;

        if (meta->capacity < len) {
                meta = set_capacity(meta, len * 2, &res);
                if (res < 0)
                        goto error_capacity;
        }

        /* Destroying values if new length is less than current */
        for (unsigned int i = len; i < meta->len; ++i)
                meta->type->destroy(data_offset(meta, i));

        meta->len = len;
        res = 0;
error_capacity:
        if (ret)
                *ret = res;

        return meta;
}

/**
 * @brief Inserts 'data' at 'pos' inside 'meta'.
 *
 * @return Pointer to a valid meta, if it was modified or not.
 *
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static struct meta *insert_element(
                struct meta *meta, unsigned int pos, const void *data, int *ret)
{
        int res;

        meta = set_len(meta, meta->len + 1, &res);
        if (res < 0)
                goto error_len;

        const size_t elem_size = meta->type->size;
        char *offset = data_offset(meta, pos);

        memmove(offset + elem_size, offset, (meta->len - pos - 1) * elem_size);
        meta->type->copy(offset, data);

        res = 0;
error_len:
        if (ret)
                *ret = res;

        return meta;
}

/**
 * @brief Removes the element at 'pos' inside 'meta'.
 */
static void remove_element(struct meta *meta, unsigned int pos)
{
        const size_t elem_size = meta->type->size;
        char *offset = data_offset(meta, pos);

        meta->type->destroy(offset);
        memmove(offset, offset + elem_size, (meta->len - pos - 1) * elem_size);
        --meta->len;
}

static void destroy_values(const struct meta *meta)
{
        for (unsigned int i = 0; i < meta->len; ++i)
                meta->type->destroy(data_offset(meta, i));
}

/* API -----------------------------------------------------------------------*/

void *vector_create(const struct type_info *type, size_t count)
{
        if (!type)
                return NULL;

        if (type->size == 0 || !type->copy || !type->comp || !type->destroy)
                return NULL;

        struct meta *meta = malloc(sizeof(*meta) + count * type->size);
        if (!meta)
                return NULL;

        meta->type = type;
        meta->len = count;
        meta->capacity = count;

        return meta_to_vector(meta);
}

void vector_destroy(const void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return;

        destroy_values(meta);
        free(meta);
}

void *vector_push(void *vector, const void *data, int *ret)
{
        int res;

        struct meta *meta = vector_to_meta(vector);
        if (!meta || !data) {
                res = -EINVAL;
                goto error_args;
        }

        meta = set_len(meta, meta->len + 1, &res);
        if (res < 0)
                goto error_len;

        char *offset = data_offset(meta, meta->len - 1);
        meta->type->copy(offset, data);
        res = 0;
error_len:
error_args:
        if (ret)
                *ret = res;

        return meta_to_vector(meta);
}

int vector_pop(void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        if (meta->len == 0)
                return -ENOBUFS;

        meta->type->destroy(data_offset(meta, meta->len - 1));
        --meta->len;
        return 0;
}

void *vector_insert(void *vector, unsigned int pos, const void *data, int *ret)
{
        int res;

        struct meta *meta = vector_to_meta(vector);
        if (!meta || !data) {
                res = -EINVAL;
                goto error_args;
        }

        if (meta->len < pos) {
                res = -ERANGE;
                goto error_pos;
        }

        meta = insert_element(meta, pos, data, &res);
error_pos:
error_args:
        if (ret)
                *ret = res;

        return meta_to_vector(meta);
}

int vector_remove(void *vector, unsigned int pos)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        if (meta->len <= pos)
                return -ERANGE;

        remove_element(meta, pos);
        return 0;
}

void *vector_resize(void *vector, size_t size, int *ret)
{
        int res = 0;

        struct meta *meta = vector_to_meta(vector);
        if (!meta) {
                res = -EINVAL;
                goto error_args;
        }

        meta = set_len(meta, size, &res);
error_args:
        if (ret)
                *ret = res;

        return meta_to_vector(meta);
}

int vector_sort(void *vector)
{
        const struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        qsort(vector, meta->len, meta->type->size, meta->type->comp);
        return 0;
}

int vector_clear(void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        destroy_values(meta);
        meta->len = 0;

        return 0;
}

void *vector_reserve(void *vector, size_t count, int *ret)
{
        int res = 0;

        struct meta *meta = vector_to_meta(vector);
        if (!meta) {
                res = -EINVAL;
                goto error_args;
        }

        if (meta->capacity < count)
                meta = set_capacity(meta, count, &res);
error_args:
        if (ret)
                *ret = res;

        return meta_to_vector(meta);
}

void *vector_fit(void *vector, int *ret)
{
        int res;

        struct meta *meta = vector_to_meta(vector);
        if (!meta) {
                res = -EINVAL;
                goto error_args;
        }

        meta = set_capacity(meta, meta->len, &res);
error_args:
        if (ret)
                *ret = res;

        return meta_to_vector(meta);
}

ssize_t vector_len(const void *vector)
{
        const struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        return (ssize_t)meta->len;
}

ssize_t vector_capacity(const void *vector)
{
        const struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return -EINVAL;

        return (ssize_t)meta->capacity;
}

/* Iterator API --------------------------------------------------------------*/

/* Iterator implementation -----------*/

static int vector_it_next(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        ++v_it->pos;
        return 0;
}

static int vector_it_previous(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        --v_it->pos;
        return 0;
}

static bool vector_it_is_valid(const struct iterator *it)
{
        const struct vector_it *v_it = (const struct vector_it *)it;
        return (0 <= v_it->pos && v_it->pos < v_it->meta->len);
}

static void *vector_it_data(const struct iterator *it)
{
        if (!vector_it_is_valid(it))
                return NULL;

        const struct vector_it *v_it = (const struct vector_it *)it;
        return data_offset(v_it->meta, v_it->pos);
}

static int vector_it_remove(struct iterator *it)
{
        if (!vector_it_is_valid(it))
                return -EINVAL;

        struct vector_it *v_it = (struct vector_it *)it;
        remove_element(v_it->meta, v_it->pos);

        return 0;
}

static void vector_it_destroy(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        free(v_it);
}

static struct iterator_callbacks vector_it_cbs = {
        .next_cb = vector_it_next,
        .previous_cb = vector_it_previous,
        .is_valid_cb = vector_it_is_valid,
        .data_cb = vector_it_data,
        .remove_cb = vector_it_remove,
        .destroy_cb = vector_it_destroy
};

/* Static functions ------------------*/

struct vector_it *vector_it_create(struct meta *meta)
{
        struct vector_it *v_it = malloc(sizeof(*v_it));
        if (!v_it)
                return NULL;

        it_init(&v_it->it, &vector_it_cbs);
        v_it->meta = meta;

        return v_it;
}

/* Public API ------------------------*/

struct iterator *vector_begin(const void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return NULL;

        struct vector_it *v_it = vector_it_create(meta);
        if (!v_it)
                return NULL;

        v_it->pos = 0;
        return (struct iterator *)v_it;
}

struct iterator *vector_end(const void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        if (!meta)
                return NULL;

        struct vector_it *v_it = vector_it_create(meta);
        if (!v_it)
                return NULL;

        v_it->pos = meta->len - 1;
        return (struct iterator *)v_it;
}
