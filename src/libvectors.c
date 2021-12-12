/**
 * @author Maxence ROBIN
 * @brief Provides simple dynamic array manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libcontainers_private.h"
#include "libiterators_private.h"
#include "libvectors.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

struct meta {
        struct container ctx; /* Placed at top for inheritance */
        size_t len;
        size_t capacity;
};

struct vector_it {
        struct iterator it; /* Placed at top for inheritance */
        void *vector;
        unsigned int index;
};

/* Static functions ----------------------------------------------------------*/

/* Conversion functions --------------*/

static struct meta *vector_to_meta(const void *vector)
{
        return (struct meta *)vector - 1;
}

static void *meta_to_vector(const struct meta *meta)
{
        return (void *)(meta + 1);
}

/* Private utility functions ---------*/

/**
 * @brief Sets the capacity of 'vector' to 'capacity'.
 *
 * @return Pointer to a valid vector, reallocated or not.
 *
 * @note if 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful ot not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static void *set_capacity(void *vector, size_t capacity, int *ret)
{
        struct meta *meta = vector_to_meta(vector);
        struct meta *new_meta;
        int res;

        new_meta = realloc(meta,
                        sizeof(*meta) + meta->ctx.type->size * capacity);
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

        return meta_to_vector(meta);
}

/**
 * @brief Sets the len of 'vector' to 'len', and reallocates it if needed.
 *
 * @return Pointer to a valid vector, if the length was modified or not.
 *
 * @note if 'ret' is not NULL, its valud will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static void *set_len(void *vector, size_t len, int *ret)
{
        struct meta *meta = vector_to_meta(vector);
        int res;

        if (meta->capacity < len) {
                vector = set_capacity(vector, len * 2, &res);
                if (res < 0)
                        goto error_capacity;

                meta = vector_to_meta(vector); /* 'vector' may have changed */
        }

        meta->len = len;
        res = 0;

error_capacity:
        if (ret)
                *ret = res;

        return vector;
}

char *data_offset(const void *vector, unsigned int pos)
{
        return (char *)vector + pos * vector_to_meta(vector)->ctx.type->size;
}

/**
 * @brief Inserts 'data' at 'pos' inside 'vector'.
 *
 * @return Pointer to a valid vector, if it was modified or not.
 *
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -ENOMEM on failure.
 */
static void *insert_element(
                void *vector, unsigned int pos, const void *data, int *ret)
{
        int res;

        struct meta *meta = vector_to_meta(vector);
        vector = set_len(vector, meta->len + 1, &res);
        if (res < 0)
                goto error_len;

        meta = vector_to_meta(vector); /* 'vector' may have changed */
        const size_t elem_size = meta->ctx.type->size;
        char *offset = data_offset(vector, pos);
        /* We substract 2 from the moved size because the length has just been
         * increased by 1.
        */
        memmove(offset + elem_size, offset, (meta->len - pos - 2) * elem_size);
        meta->ctx.type->copy(offset, data, elem_size);

        res = 0;

error_len:
        if (ret)
                *ret = res;

        return vector;
}

/**
 * @brief Removes the element at 'pos' inside 'vector'.
 */
static void remove_element(void *vector, unsigned int pos)
{
        struct meta *meta = vector_to_meta(vector);

        const size_t elem_size = meta->ctx.type->size;
        char *offset = data_offset(vector, pos);

        meta->ctx.type->destroy(offset);
        memmove(offset, offset + elem_size, (meta->len - pos - 1) * elem_size);
        --meta->len;
}

static bool it_is_valid(const void *vector, const struct vector_it *it)
{
        return (it->vector == vector
                        && it->index < vector_to_meta(vector)->len);
}

/* Iterator implementation -----------*/

static struct vector_it *vector_it_create(); /* Forward declaration */

static struct iterator *vector_it_copy(const struct iterator *it)
{
        struct vector_it *copy = vector_it_create();
        if (!copy)
                return NULL;

        const struct vector_it *v_it = (const struct vector_it *)it;
        copy->vector = v_it->vector;
        copy->index = v_it->index;

        return (struct iterator *)copy;
}

static void vector_it_destroy(const struct iterator *it)
{
        free((struct vector_it *)it);
}

static bool vector_it_is_valid(const struct iterator *it)
{
        const struct vector_it *v_it = (const struct vector_it *)it;
        return it_is_valid(v_it->vector, v_it);
}

static int vector_it_next(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        ++v_it->index;
        return 0;
}

static int vector_it_previous(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        --v_it->index;
        return 0;
}

static const void *vector_it_data(const struct iterator *it)
{
        const struct vector_it *v_it = (const struct vector_it *)it;

        if (!it_is_valid(v_it->vector, v_it))
                return NULL;

        const struct meta *meta = vector_to_meta(v_it->vector);

        return (char *)v_it->vector + v_it->index * meta->ctx.type->size;
}

static int vector_it_set_data(struct iterator *it, const void *data)
{
        struct vector_it *v_it = (struct vector_it *)it;
        const struct meta *meta = vector_to_meta(v_it->vector);

        if (v_it->index >= meta->len)
                return -ERANGE;

        char *offset = data_offset(v_it->vector, v_it->index);

        meta->ctx.type->copy(offset, data, meta->ctx.type->size);
        return 0;
}

static struct iterator_callbacks iterator_cbs = {
        .copy = vector_it_copy,
        .destroy = vector_it_destroy,
        .is_valid = vector_it_is_valid,
        .next = vector_it_next,
        .previous = vector_it_previous,
        .data = vector_it_data,
        .set_data = vector_it_set_data
};

static struct vector_it *vector_it_create()
{
        struct vector_it *it = malloc(sizeof(*it));
        if (!it)
                return NULL;

        it->it.cbs = &iterator_cbs;
        return it;
}

/* Container implementation ----------*/

static struct iterator *vector_container_first(const struct container *ctx)
{
        struct vector_it *it = vector_it_create();
        if (!it)
                return NULL;

        it->vector = meta_to_vector((const struct meta *)ctx);
        it->index = 0;

        return (struct iterator *)it;
}

static struct iterator *vector_container_last(const struct container *ctx)
{
        struct vector_it *it = vector_it_create();
        if (!it)
                return NULL;

        const struct meta *meta = (const struct meta *)ctx;
        it->vector = meta_to_vector(meta);
        it->index = meta->len - 1;

        return (struct iterator *)it;
}

static int vector_container_insert(
                struct container *ctx, struct iterator *it, const void *data)
{
        const void *vector = meta_to_vector((const struct meta *)ctx);
        struct vector_it *v_it = (struct vector_it *)it;

        if (!it_is_valid(vector, v_it))
                return -EINVAL;

        int res;
        v_it->vector = insert_element(v_it->vector, v_it->index, data, &res);

        return res;
}

static int vector_container_remove(
                struct container *ctx, const struct iterator *it)
{
        void *vector = meta_to_vector((const struct meta *)ctx);
        const struct vector_it *v_it = (const struct vector_it *)it;

        if (!it_is_valid(vector, v_it))
                return -EINVAL;

        remove_element(vector, v_it->index);
        return 0;
}

static const struct container_callbacks container_cbs = {
        .first = vector_container_first,
        .last = vector_container_last,
        .insert = vector_container_insert,
        .remove = vector_container_remove
};

/* API -----------------------------------------------------------------------*/

void *vector_create(const struct type_info *type, size_t count)
{
        if (!type)
                return NULL;

        struct meta *meta = malloc(sizeof(*meta) + count * type->size);
        if (!meta)
                return NULL;

        meta->ctx.cbs = &container_cbs;
        meta->ctx.type = type;

        meta->len = count;
        meta->capacity = count;

        return meta_to_vector(meta);
}

void vector_destroy(const void *vector)
{
        if (!vector)
                return;

        struct meta *meta = vector_to_meta(vector);
        for (unsigned int i = 0; i < meta->len; ++i)
                meta->ctx.type->destroy(data_offset(vector, i));

        free(meta);
}

void *vector_push(void *vector, const void *data, int *ret)
{
        int res;

        if (!vector || !data) {
                res = -EINVAL;
                goto error_args;
        }

        vector = set_len(vector, vector_to_meta(vector)->len + 1, &res);
        if (res < 0)
                goto error_len;

        const struct meta *meta = vector_to_meta(vector);
        const size_t elem_size = meta->ctx.type->size;
        char *offset = data_offset(vector, meta->len - 1);

        meta->ctx.type->copy(offset, data, elem_size);
        res = 0;

error_len:
error_args:
        if (ret)
                *ret = res;

        return vector;
}

int vector_pop(void *vector)
{
        if (!vector)
                return -EINVAL;

        struct meta *meta = vector_to_meta(vector);
        if (meta->len == 0)
                return -ENOBUFS;

        meta->ctx.type->destroy(data_offset(vector, meta->len - 1));
        --meta->len;
        return 0;
}

void *vector_insert(void *vector, unsigned int pos, const void *data, int *ret)
{
        int res;

        if (!vector || !data) {
                res = -EINVAL;
                goto error_args;
        }

        if (vector_to_meta(vector)->len < pos) {
                res = -ERANGE;
                goto error_pos;
        }

        vector = insert_element(vector, pos, data, &res);

error_pos:
error_args:
        if (ret)
                *ret = res;

        return vector;
}

int vector_remove(void *vector, unsigned int pos)
{
        if (!vector)
                return -EINVAL;

        if (vector_to_meta(vector)->len <= pos)
                return -ERANGE;

        remove_element(vector, pos);
        return 0;
}

void *vector_reserve(void *vector, size_t count, int *ret)
{
        int res = 0;

        if (!vector) {
                res = -EINVAL;
                goto error_args;
        }

        if (vector_to_meta(vector)->capacity < count)
                vector = set_capacity(vector, count, &res);

error_args:
        if (ret)
                *ret = res;

        return vector;
}

void *vector_fit(void *vector, int *ret)
{
        int res;

        if (!vector) {
                res = -EINVAL;
                goto error_args;
        }

        vector = set_capacity(vector, vector_to_meta(vector)->len, &res);

error_args:
        if (ret)
                *ret = res;

        return vector;
}

struct container *vector_container(void *vector)
{
        if (!vector)
                return NULL;

        return &vector_to_meta(vector)->ctx;
}

ssize_t vector_len(const void *vector)
{
        if (!vector)
                return -EINVAL;

        return (ssize_t)vector_to_meta(vector)->len;
}

ssize_t vector_capacity(const void *vector)
{
        if (!vector)
                return -EINVAL;

        return (ssize_t)vector_to_meta(vector)->capacity;
}
