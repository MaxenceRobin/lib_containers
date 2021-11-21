/**
 * @author Maxence ROBIN
 * @brief Private utilities for containers manipulation.
 */

#ifndef LIB_CONTAINERS_PRIVATE_H
#define LIB_CONTAINERS_PRIVATE_H

/* Includes ------------------------------------------------------------------*/

#include "libcontainers.h"
#include "libiterators.h"

/* Definitions ---------------------------------------------------------------*/

struct container_callbacks {
        struct iterator *(*first)(const struct container *);
        struct iterator *(*last)(const struct container *);
};

struct container {
        const struct container_callbacks *cbs;
};

#endif /* LIB_CONTAINERS_PRIVATE_H */
