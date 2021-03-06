/**
 * @author Maxence ROBIN
 * @brief Provides simple iterators manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_iterators.h"
#include "lib_iterators_private.h"

#include <errno.h>
#include <stdlib.h>

/* Definitions ---------------------------------------------------------------*/

/* API -----------------------------------------------------------------------*/

void it_init(struct iterator *it, const struct iterator_callbacks *cbs)
{
        it->count = 1;
        it->cbs = cbs;
}

struct iterator *it_ref(struct iterator *it)
{
        if (!it)
                return NULL;

        ++it->count;
        return it;
}

struct iterator *it_unref(struct iterator *it)
{
        if (!it)
                return NULL;

        if (atomic_fetch_sub(&it->count, 1) == 1) {
                it->cbs->destroy_cb(it);
                it = NULL;
        }

        return it;
}

int it_next(struct iterator *it)
{
        if (!it)
                return -EINVAL;

        return it->cbs->next_cb(it);
}

int it_previous(struct iterator *it)
{
        if (!it)
                return -EINVAL;

        return it->cbs->previous_cb(it);
}

bool it_is_valid(const struct iterator *it)
{
        if (!it)
                return false;

        return it->cbs->is_valid_cb(it);
}

void *it_data(const struct iterator *it)
{
        if (!it)
                return NULL;

        return it->cbs->data_cb(it);
}

const struct type_info *it_type(const struct iterator *it)
{
        if (!it)
                return NULL;

        return it->cbs->type_cb(it);
}

int it_remove(struct iterator *it)
{
        if (!it)
                return -EINVAL;

        return it->cbs->remove_cb(it);
}

struct iterator *it_dup(const struct iterator *it)
{
        if (!it)
                return NULL;

        return it->cbs->dup_cb(it);
}

int it_copy(struct iterator *dest, const struct iterator *src)
{
        if (!dest || !src)
                return -EINVAL;

        return dest->cbs->copy_cb(dest, src);
}
