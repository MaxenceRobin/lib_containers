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

int container_for_each(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        return for_each(ctx, cb, data, container_first, iterator_next);
}

int container_for_each_r(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        return for_each(ctx, cb, data, container_last, iterator_previous);
}

bool container_contains(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return false;

        struct iterator *it = container_first(ctx);
        if (!it)
                return false;

        bool found = true;

        while (iterator_is_valid(it)) {
                if (memcmp(iterator_data(it), data, ctx->elem_size) == 0)
                        goto data_found;
                
                if (iterator_next(it) < 0)
                        break;
        }

        found = false;
data_found:
        iterator_destroy(it);
        return found;
}
