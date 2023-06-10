/**
 * @author Maxence ROBIN
 * @brief Provides algorithms to manipulate containers
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_container_algos.h"

#include <errno.h>

/* Definitions ---------------------------------------------------------------*/

typedef int (*iterate_cb)(struct iterator *);

struct match_equal_ctx {
        const struct type_info *type;
        const void *value;
};

struct fill_ctx {
        const struct type_info *type;
        const void *value;
};

enum comp_type {
        COMP_TYPE_MIN,
        COMP_TYPE_MAX
};

/* Callbacks functions -------------------------------------------------------*/

static bool match_equal(const void *value, void *arg)
{
        struct match_equal_ctx *ctx = arg;
        return (ctx->type->comp(value, ctx->value) == 0);
}

static void fill(void *dest, void *arg)
{
        struct fill_ctx *ctx = arg;
        ctx->type->destroy(dest);
        ctx->type->copy(dest, ctx->value);
}

/* Static functions ----------------------------------------------------------*/

static int for_each(struct iterator *it, ctn_action_cb action, void *arg)
{
        struct iterator *dup = it_dup(it);
        if (!dup)
                return -ENOMEM;

        while (it_is_valid(dup)) {
                action(it_data(dup), arg);
                it_next(dup);
        }

        it_unref(dup);
        return 0;
}

static int count_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *dup = it_dup(it);
        if (!dup)
                return -ENOMEM;

        int count = 0;
        while (it_is_valid(dup)) {
                if (match(it_data(dup), arg))
                        ++count;

                it_next(dup);
        }

        it_unref(dup);
        return count;
}

static struct iterator *find_if(
                struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *dup = it_dup(it);
        if (!dup)
                return NULL;

        while (it_is_valid(dup)) {
                if (match(it_data(dup), arg))
                        return dup;

                it_next(dup);
        }

        it_unref(dup);
        return NULL;
}

static int remove_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *dup = it_dup(it);
        if (!dup)
                return -ENOMEM;

        while (it_is_valid(dup)) {
                if (match(it_data(dup), arg))
                        it_remove(dup);
                else
                        it_next(dup);
        }

        it_unref(dup);
        return 0;
}

static int keep_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *dup = it_dup(it);
        if (!dup)
                return -ENOMEM;

        while (it_is_valid(dup)) {
                if (!match(it_data(dup), arg))
                        it_remove(dup);
                else
                        it_next(dup);
        }

        it_unref(dup);
        return 0;
}

static bool contains_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *found = find_if(it, match, arg);
        if (found) {
                it_unref(found);
                return true;
        } else {
                return false;
        }
}

static struct iterator *min_max(struct iterator *it, enum comp_type comp_type)
{
        struct iterator *found = NULL;

        struct iterator *dup = it_dup(it);
        if (!dup)
                return NULL;

        if (!it_is_valid(dup))
                goto error_it_invalid;

        found = it_dup(it);
        if (!found)
                goto error_it_invalid;

        const struct type_info *type = it_type(it);
        do {
                const int res = type->comp(it_data(dup), it_data(found));
                if ((comp_type == COMP_TYPE_MIN && res < 0)
                                || (comp_type == COMP_TYPE_MAX && res > 0)) {
                        it_copy(found, dup);
                }

                it_next(dup);
        } while (it_is_valid(dup));

error_it_invalid:
        it_unref(dup);
        return found;
}

static int copy_min_max(
                struct iterator *it, void *value, enum comp_type comp_type)
{
        struct iterator *found = min_max(it, comp_type);
        if (!found)
                return -ENOENT;

        it_type(it)->copy(value, it_data(found));
        it_unref(found);

        return 0;
}

/* API -----------------------------------------------------------------------*/

int ctn_for_each(struct iterator *it, ctn_action_cb action, void *arg)
{
        int res = -EINVAL;
        if (!it || !action)
                goto out;

        res = for_each(it, action, arg);
out:
        it_unref(it);
        return res;
}

int ctn_count(struct iterator *it, const void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = count_if(it, match_equal, &(struct match_equal_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

int ctn_count_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        int res = -EINVAL;
        if (!it || !match)
                goto out;

        res = count_if(it, match, arg);
out:
        it_unref(it);
        return res;
}

struct iterator *ctn_find(struct iterator *it, const void *value)
{
        struct iterator *res = NULL;
        if (!it || !value)
                goto out;

        res = find_if(it, match_equal, &(struct match_equal_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

struct iterator *ctn_find_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        struct iterator *res = NULL;
        if (!it || !match)
                goto out;

        res = find_if(it, match, arg);
out:
        it_unref(it);
        return res;
}

int ctn_remove(struct iterator *it, const void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = remove_if(it, match_equal, &(struct match_equal_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

int ctn_remove_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        int res = -EINVAL;
        if (!it || !match)
                goto out;

        res = remove_if(it, match, arg);
out:
        it_unref(it);
        return res;
}

int ctn_keep(struct iterator *it, const void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = keep_if(it, match_equal, &(struct match_equal_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

int ctn_keep_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        int res = -EINVAL;
        if (!it || !match)
                goto out;

        res = keep_if(it, match, arg);
out:
        it_unref(it);
        return res;
}

bool ctn_contains(struct iterator *it, const void *value)
{
        bool res = false;
        if (!it || !value)
                goto out;

        res = contains_if(it, match_equal, &(struct match_equal_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

bool ctn_contains_if(struct iterator *it, ctn_match_cb match, void *arg)
{
        bool res = false;
        if (!it || !match)
                goto out;

        res = contains_if(it, match, arg);
out:
        it_unref(it);
        return res;
}

int ctn_fill(struct iterator *it, const void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = for_each(it, fill, &(struct fill_ctx) {
                .type = it_type(it),
                .value = value
        });
out:
        it_unref(it);
        return res;
}

struct iterator *ctn_min(struct iterator *it)
{
        struct iterator *res = NULL;
        if (!it)
                goto out;

        res = min_max(it, COMP_TYPE_MIN);
out:
        it_unref(it);
        return res;
}

struct iterator *ctn_max(struct iterator *it)
{
        struct iterator *res = NULL;
        if (!it)
                goto out;

        res = min_max(it, COMP_TYPE_MAX);
out:
        it_unref(it);
        return res;
}

int ctn_copy_min(struct iterator *it, void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = copy_min_max(it, value, COMP_TYPE_MIN);
out:
        it_unref(it);
        return res;
}

int ctn_copy_max(struct iterator *it, void *value)
{
        int res = -EINVAL;
        if (!it || !value)
                goto out;

        res = copy_min_max(it, value, COMP_TYPE_MAX);
out:
        it_unref(it);
        return res;
}
