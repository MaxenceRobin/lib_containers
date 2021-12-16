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

/* Definitions ---------------------------------------------------------------*/

enum loop_action {
        LOOP_ACTION_CONTINUE = 1, /* Don't interfer with 0 return value */
        LOOP_ACTION_REMOVE,
        LOOP_ACTION_STOP
};

/* Static functions ----------------------------------------------------------*/

static int loop(
                struct container *ctx,
                int (*cb)(struct container *, struct iterator *, void *),
                void *args)
{
        int res;

        struct iterator *it = ctn_first(ctx);
        if (!it)
                return -ENOMEM;

        while (iterator_is_valid(it)) {
                res = cb(ctx, it, args);
                if (res < 0)
                        goto error_iterate;

                if (res == LOOP_ACTION_STOP)
                        return LOOP_ACTION_STOP;

                res = (res == LOOP_ACTION_REMOVE) ?
                                ctn_remove_it(ctx, it) : iterator_next(it);
                if (res < 0)
                        goto error_iterate;
        }

        res = 0;
error_iterate:
        iterator_destroy(it);
        return res;
}

/* API -----------------------------------------------------------------------*/

/* Callback implementations ----------*/

struct iterator *ctn_first(const struct container *ctx)
{
        if (!ctx || !ctx->cbs || !ctx->cbs->first)
                return NULL;

        return ctx->cbs->first(ctx);
}

struct iterator *ctn_last(const struct container *ctx)
{
        if (!ctx || !ctx->cbs || !ctx->cbs->last)
                return NULL;

        return ctx->cbs->last(ctx);
}

int ctn_insert_it(struct container *ctx, struct iterator *it, const void *data)
{
        if (!ctx || !it || !data)
                return -EINVAL;

        if (!ctx->cbs || !ctx->cbs->insert)
                return -ENOTSUP;

        return ctx->cbs->insert(ctx, it, data);
}

int ctn_remove_it(struct container *ctx, const struct iterator *it)
{
        if (!ctx || !it)
                return -EINVAL;

        if (!ctx->cbs || !ctx->cbs->remove)
                return -ENOTSUP;

        return ctx->cbs->remove(ctx, it);
}

/* Algorithms ------------------------*/

/* for_each ----------------*/

struct for_each_args {
        int (*cb)(struct iterator *, void *);
        void *data;
};

static int for_each_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct for_each_args *args = data;

        const int res = args->cb(it, args->data);
        return (res < 0) ?  res : LOOP_ACTION_CONTINUE;
}

int ctn_for_each(
                struct container *ctx,
                int (*cb)(struct iterator *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct for_each_args args = { .cb = cb, .data = data };
        return loop(ctx, for_each_cb, &args);
}

/* count -------------------*/

struct count_args {
        const void *data;
        int count;
};

static int count_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct count_args *args = data;

        if (ctx->type->comp(
                        args->data, iterator_data(it), ctx->type->size) == 0)
                ++args->count;

        return LOOP_ACTION_CONTINUE;
}

int ctn_count(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        struct count_args args = { .data = data, .count = 0};
        const int res = loop(ctx, count_cb, &args);
        if (res < 0)
                return res;

        return args.count;
}

struct count_if_args {
        bool (*cb)(const void *, void *);
        void *data;
        int count;
};

static int count_if_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct count_if_args *args = data;

        if (args->cb(iterator_data(it), args->data))
                ++args->count;

        return LOOP_ACTION_CONTINUE;
}

int ctn_count_if(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        struct count_if_args args = { .cb = cb, .data = data, .count = 0};
        const int res = loop(ctx, count_if_cb, &args);
        if (res < 0)
                return res;

        return args.count;
}

/* find --------------------*/

struct find_args {
        const void *data;
        struct iterator *it;
};

static int find_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct find_args *args = data;

        if (ctx->type->comp(
                        args->data, iterator_data(it), ctx->type->size) == 0) {
                args->it = it;
                return LOOP_ACTION_STOP;
        }

        return LOOP_ACTION_CONTINUE;
}

struct iterator *ctn_find(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        struct find_args args = { .data = data };
        if (loop(ctx, find_cb, &args) < 0)
                return NULL;

        return args.it;
}

struct find_if_args {
        bool (*cb)(const void *, void *);
        void *data;
        struct iterator *it;
};

static int find_if_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct find_if_args *args = data;

        if (args->cb(iterator_data(it), args->data)) {
                args->it = it;
                return LOOP_ACTION_STOP;
        }

        return LOOP_ACTION_CONTINUE;
}

struct iterator *ctn_find_if(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct find_if_args args = { .cb = cb, .data = data };
        if (loop(ctx, find_if_cb, &args) < 0)
                return NULL;

        return args.it;
}

/* contains ----------------*/

bool ctn_contains(const struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        struct find_args args = { .data = data };
        if (loop(ctx, find_cb, &args) < 0)
                return false;

        iterator_destroy(args.it);
        return true;
}

bool ctn_any_of(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct find_if_args args = { .cb = cb, .data = data };
        if (loop(ctx, find_if_cb, &args) < 0)
                return false;

        iterator_destroy(args.it);
        return true;
}

static int all_of_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct find_if_args *args = data;

        if (!args->cb(iterator_data(it), args->data))
                return -1;

        return LOOP_ACTION_CONTINUE;
}

bool ctn_all_of(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct find_if_args args = { .cb = cb, .data = data };
        if (loop(ctx, all_of_cb, &args) < 0)
                return false;

        return true;
}

/* fill --------------------*/

static int fill_cb(struct container *ctx, struct iterator *it, void *data)
{
        const int res = iterator_set_data(it, data);
        if (res < 0)
                return res;

        return LOOP_ACTION_CONTINUE;
}

int ctn_fill(struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        return loop(ctx, fill_cb, (void *)data);
}

struct generate_args {
        const void *(*cb)(void *);
        void *data;
};

static int generate_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct generate_args *args = data;

        const int res = iterator_set_data(it, args->cb(args->data));
        if (res < 0)
                return res;

        return LOOP_ACTION_CONTINUE;
}

int ctn_generate(struct container *ctx, const void *(*cb)(void *), void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct generate_args args = { .cb = cb, .data = data };
        return loop(ctx, generate_cb, &args);
}

/* min/max -----------------*/

enum min_max_mode {
        MODE_MIN,
        MODE_MAX
};

struct min_max_args {
        const void *value;
        enum min_max_mode mode;
        bool first;
};

static int min_max_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct min_max_args *args = data;
        const void *it_data = iterator_data(it);

        if (args->first) {
                args->value = it_data;
                args->first = false;
                return LOOP_ACTION_CONTINUE;
        }

        const int res = ctx->type->comp(it_data, args->value, ctx->type->size);
        if (res < 0 && args->mode == MODE_MIN ||
                        res > 0 && args->mode == MODE_MAX)
                args->value = it_data;

        return LOOP_ACTION_CONTINUE;
}

static const void *min_max(const struct container *ctx, enum min_max_mode mode)
{
        struct min_max_args args = {
                .mode = mode,
                .first = true,
                .value = NULL
        };

        const int res = loop(ctx, min_max_cb, &args);
        if (res < 0)
                return NULL;

        return args.value;
}

const void *ctn_min(const struct container *ctx)
{
        if (!ctx)
                return NULL;

        return min_max(ctx, MODE_MIN);
}

const void *ctn_max(const struct container *ctx)
{
        if (!ctx)
                return NULL;

        return min_max(ctx, MODE_MAX);
}

/* remove ------------------*/

int ctn_remove(struct container *ctx, const void *data)
{
        if (!ctx || !data)
                return -EINVAL;

        struct find_args args = { .data = data };
        int res = loop(ctx, find_cb, &args);
        if (res < 0)
                return res;

        res = ctn_remove_it(ctx, args.it);
        iterator_destroy(args.it);
        return res;
}

static int remove_if_cb(struct container *ctx, struct iterator *it, void *data)
{
        struct find_if_args *args = data;

        if (args->cb(iterator_data(it), args->data))
                return LOOP_ACTION_REMOVE;

        return LOOP_ACTION_CONTINUE;
}

int ctn_remove_if(
                struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data)
{
        if (!ctx || !cb)
                return -EINVAL;

        struct find_if_args args = { .cb = cb, .data = data };
        const int res = loop(ctx, remove_if_cb, &args);
        if (res < 0)
                return res;

        iterator_destroy(args.it);
        return 0;
}
