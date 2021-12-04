/**
 * @author Maxence ROBIN
 * @brief Private utilities for iterators manipulation.
 */

#ifndef LIB_ITERATORS_PRIVATE_H
#define LIB_ITERATORS_PRIVATE_H

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"

#include <stdbool.h>

/* Definitions ---------------------------------------------------------------*/

struct iterator_callbacks {
        struct iterator *(*copy)(const struct iterator *);
        void (*destroy)(const struct iterator *);
        bool (*is_valid)(const struct iterator *);
        int (*next)(struct iterator *);
        int (*previous)(struct iterator *);
        const void *(*data)(struct iterator *);
        int (*set_data)(struct iterator *, const void *);
};

struct iterator {
        struct iterator_callbacks *cbs;
};

#endif /* LIB_ITERATORS_PRIVATE_H */
