/**
 * @author Maxence ROBIN
 * @brief Provides generic functions for containers manipulation and algorithms.
 */

/* Includes ------------------------------------------------------------------*/

#include "libcontainers.h"
#include "libcontainers_private.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Static functions ----------------------------------------------------------*/

/**
 * @brief Calls 'cb' with 'data' for each element of 'ctx', starting with the
 * element given by 'it_get' and iterating using 'it_iterate'. 
 * 
 * @return 0 on success.
 * @return -ENOMEM if 'it_get' failed.
 * @return A negative errno if the iteration failed.
 * @return A custom negative errno if a call to 'cb' failed.
 * 
 * @note For each call to 'cb', the element of 'ctx' will be the first argument
 * and 'data' will be the second.
 * @warning 'cb' MUST return a negative errno on failure, and 0 on success.
 */
static int for_each(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data,
                struct iterator *(*it_get)(const struct container *),
                int (*it_iterate)(struct iterator *))
{
        int res;
        struct iterator *it = it_get(ctx);
        if (!it) {
                res = -ENOMEM;
                goto error_get;
        }

        while (iterator_is_valid(it)) {
                res = cb(iterator_data(it), data);
                if (res < 0)
                        goto error_call;

                res = it_iterate(it);
                if (res < 0)
                        goto error_iterate;
        }

        res = 0;
error_iterate:
error_call:
        iterator_destroy(it);
error_get:
        return res;
}

/**
 * @brief Finds 'data' inside 'ctx'.
 * 
 * @return An iterator over the found data on success.
 * @return NULL on failure. 
 */
static struct iterator *find_element(
                const struct container *ctx, const void *data)
{
        struct iterator *it = container_first(ctx);
        if (!it)
                goto error_first;

        while (iterator_is_valid(it)) {
                if (memcmp(iterator_data(it), data, ctx->elem_size) == 0)
                        goto data_found;
                
                if (iterator_next(it) < 0)
                        break;
        }

        iterator_destroy(it);
        it = NULL;
data_found:
error_first:
        return it;
}

/* API -----------------------------------------------------------------------*/

struct iterator *container_first(const struct container *ctx)
{
        if (!ctx || !ctx->cbs || !ctx->cbs->first)
                return NULL;

        return ctx->cbs->first(ctx);
}

struct iterator *container_last(const struct container *ctx)
{
        if (!ctx || !ctx->cbs || !ctx->cbs->last)
                return NULL;

        return ctx->cbs->last(ctx);
}

int container_insert(
                struct container *ctx, struct iterator *it, const void *data)
{
        if (!ctx || !it || !data)
                return -EINVAL;

        if (!ctx->cbs || !ctx->cbs->insert)
                return -ENOTSUP;

        return ctx->cbs->insert(ctx, it, data);
}

int container_remove(struct container *ctx, const struct iterator *it)
{
        if (!ctx || !it)
                return -EINVAL;

        if (!ctx->cbs || !ctx->cbs->remove)
                return -ENOTSUP;

        return ctx->cbs->remove(ctx, it);
}

int container_for_each(
                struct container *ctx,
                int (*cb)(void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        return for_each(ctx, cb, data, container_first, iterator_next);
}

int container_for_each_r(
                struct container *ctx,
                int (*cb)(void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        return for_each(ctx, cb, data, container_last, iterator_previous);
}

struct iterator *container_find(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return NULL;

        return find_element(ctx, data);
}

bool container_contains(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return false;

        const struct iterator *it = find_element(ctx, data);
        if (!it)
                return false;

        iterator_destroy(it);
        return true;
}
