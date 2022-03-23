/**
 * @author Maxence ROBIN
 * @brief Provides simple dynamic array manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"
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

        return meta_to_vector(meta);
}

static char *data_offset(const void *vector, unsigned int pos)
{
        return (char *)vector + pos * vector_to_meta(vector)->type->size;
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

        /* Destroying values if new length is less than current */
        for (unsigned int i = len; i < meta->len; ++i)
                meta->type->destroy(data_offset(vector, i));

        meta->len = len;
        res = 0;
error_capacity:
        if (ret)
                *ret = res;

        return vector;
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
        const size_t elem_size = meta->type->size;
        char *offset = data_offset(vector, pos);
        /* We substract 2 from the moved size because the length has just been
         * increased by 1.
        */
        memmove(offset + elem_size, offset, (meta->len - pos - 2) * elem_size);
        meta->type->copy(offset, data, elem_size);

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

        const size_t elem_size = meta->type->size;
        char *offset = data_offset(vector, pos);

        meta->type->destroy(offset);
        memmove(offset, offset + elem_size, (meta->len - pos - 1) * elem_size);
        --meta->len;
}

static void destroy_values(const void *vector)
{
        struct meta *meta = vector_to_meta(vector);
        for (unsigned int i = 0; i < meta->len; ++i)
                meta->type->destroy(data_offset(vector, i));
}

/* API -----------------------------------------------------------------------*/

void *vector_create(const struct type_info *type, size_t count)
{
        if (!type)
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
        if (!vector)
                return;

        destroy_values(vector);
        free(vector_to_meta(vector));
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
        const size_t elem_size = meta->type->size;
        char *offset = data_offset(vector, meta->len - 1);

        meta->type->copy(offset, data, elem_size);
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

        meta->type->destroy(data_offset(vector, meta->len - 1));
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

void *vector_resize(void *vector, size_t size, int *ret)
{
        int res = 0;

        if (!vector) {
                res = -EINVAL;
                goto error_args;
        }

        vector = set_len(vector, size, &res);
error_args:
        if (ret)
                *ret = res;

        return vector;
}

int vector_clear(void *vector)
{
        if (!vector)
                return -EINVAL;

        destroy_values(vector);
        vector_to_meta(vector)->len = 0;
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
