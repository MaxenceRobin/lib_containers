/**
 * @author Maxence ROBIN
 * @brief Abstraction over containers for data structure manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"
#include "libiterators_private.h"

#include <errno.h>
#include <stddef.h>

/* API -----------------------------------------------------------------------*/

struct iterator *iterator_copy(const struct iterator *it)
{
        if (!it || !it->cbs || !it->cbs->copy)
                return NULL;

        return it->cbs->copy(it);
}

void iterator_destroy(const struct iterator *it)
{
        if (!it || !it->cbs || !it->cbs->destroy)
                return;

        it->cbs->destroy(it);
}

bool iterator_is_valid(const struct iterator *it)
{
        if (!it || !it->cbs || !it->cbs->is_valid)
                return false;

        return it->cbs->is_valid(it);
}

int iterator_next(struct iterator *it)
{
        if (!it)
                return -EINVAL;

        if (!it->cbs || !it->cbs->next)
                return -ENOTSUP;

        return it->cbs->next(it);
}


int iterator_previous(struct iterator *it)
{
        if (!it)
                return -EINVAL;

        if (!it->cbs || !it->cbs->previous)
                return -ENOTSUP;

        return it->cbs->previous(it);
}

void *iterator_data(struct iterator *it)
{
        if (!it || !it->cbs || !it->cbs->data)
                return NULL;

        return it->cbs->data(it);
}
