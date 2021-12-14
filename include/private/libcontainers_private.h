/**
 * @author Maxence ROBIN
 * @brief Private utilities for containers manipulation.
 */

#ifndef LIB_CONTAINERS_PRIVATE_H
#define LIB_CONTAINERS_PRIVATE_H

/* Includes ------------------------------------------------------------------*/

#include "libcontainers.h"
#include "libiterators.h"
#include "libtypes.h"

#include <stddef.h>

/* Definitions ---------------------------------------------------------------*/

struct container_callbacks {
        struct iterator *(*first)(const struct container *);
        struct iterator *(*last)(const struct container *);
        int (*insert)(struct container *, struct iterator *, const void *);
        int (*remove)(struct container *, const struct iterator *);
};

struct container {
        const struct container_callbacks *cbs;
        const struct type_info *type;
};

#endif /* LIB_CONTAINERS_PRIVATE_H */
