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
        const struct meta *meta = vector_to_meta(v_it->vector);

        return (v_it->index < meta->len);
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

static void *vector_it_data(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;
        const struct meta *meta = vector_to_meta(v_it->vector);

        if (v_it->index >= meta->len)
                return NULL;

        return (char *)v_it->vector + v_it->index * meta->ctx.elem_size;
}

static int vector_it_insert(struct iterator *it, void *data)
{
        struct vector_it *v_it = (struct vector_it *)it;
        int res;
        
        v_it->vector = vector_insert(v_it->vector, v_it->index, data, &res);
        return res;
}

static int vector_it_remove(struct iterator *it)
{
        struct vector_it *v_it = (struct vector_it *)it;

        return vector_remove(v_it->vector, v_it->index);
}

static struct iterator_callbacks iterator_cbs = {
        .copy = vector_it_copy,
        .destroy = vector_it_destroy,
        .is_valid = vector_it_is_valid,
        .next = vector_it_next,
        .previous = vector_it_previous,
        .data = vector_it_data,
        .insert = vector_it_insert,
        .remove = vector_it_remove
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

        it->vector = meta_to_vector((struct meta *)ctx);
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

static const struct container_callbacks container_cbs = {
        .first = vector_container_first,
        .last = vector_container_last
};

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
                                sizeof(*meta) + meta->ctx.elem_size * capacity);
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

/* API -----------------------------------------------------------------------*/

void *vector_create(size_t elem_size, size_t count)
{
        struct meta *meta = malloc(sizeof(*meta) + count * elem_size);
        if (!meta)
                return NULL;

        meta->ctx.cbs = &container_cbs;
        meta->ctx.elem_size = elem_size;
        meta->len = count;
        meta->capacity = count;

        return meta_to_vector(meta);
}

void vector_destroy(const void *vector)
{
        if (!vector)
                return;

        free(vector_to_meta(vector));
}

void *vector_push(void *vector, void *data, int *ret)
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
        memcpy((char *)vector + (meta->len - 1) * meta->ctx.elem_size,
                        data,
                        meta->ctx.elem_size);
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

        --meta->len;
        return 0;
}

void *vector_insert(void *vector, unsigned int pos, void *data, int *ret)
{
        int res;

        if (!vector || !data) {
                res = -EINVAL;
                goto error_args;
        }

        struct meta *meta = vector_to_meta(vector);
        if (meta->len < pos) {
                res = -ERANGE;
                goto error_pos;
        }

        vector = set_len(vector, meta->len + 1, &res);
        if (res < 0)
                goto error_len;

        meta = vector_to_meta(vector); /* 'vector' may have changed */
        char *offset = (char *)vector + pos * meta->ctx.elem_size;
        /* We substract 2 from the moved size because the length has just been
         * increased by 1.
        */
        memmove(offset + meta->ctx.elem_size,
                        offset,
                        (meta->len - pos - 2) * meta->ctx.elem_size);
        memcpy(offset, data, meta->ctx.elem_size);
        res = 0;
        
error_len:
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

        struct meta *meta = vector_to_meta(vector);
        if (meta->len <= pos)
                return -ERANGE;

        char *offset = (char *)vector + pos * meta->ctx.elem_size;
        memmove(offset,
                        offset + meta->ctx.elem_size,
                        (meta->len - pos - 1) * meta->ctx.elem_size);
        --meta->len;
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

const struct container *vector_container(const void *vector)
{
        if (!vector)
                return NULL;

        return (const struct container *)vector_to_meta(vector);
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
