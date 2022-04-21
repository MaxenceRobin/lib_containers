/**
 * @author Maxence ROBIN
 * @brief Provides private iterators definitions and API.
 */

#ifndef LIB_ITERATORS_PRIVATE_H
#define LIB_ITERATORS_PRIVATE_H

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"

#include <stdatomic.h>

/* Definitions ---------------------------------------------------------------*/

typedef int (*it_next_cb)(struct iterator *);
typedef int (*it_previous_cb)(struct iterator *);
typedef bool (*it_is_valid_cb)(const struct iterator *);
typedef void *(*it_data_cb)(const struct iterator *);
typedef const struct type_info *(*it_type_cb)(const struct iterator *);
typedef int (*it_remove_cb)(struct iterator *);
typedef void (*it_destroy_cb)(struct iterator *);

struct iterator_callbacks {
        it_next_cb next_cb;
        it_previous_cb previous_cb;
        it_is_valid_cb is_valid_cb;
        it_data_cb data_cb;
        it_type_cb type_cb;
        it_remove_cb remove_cb;
        it_destroy_cb destroy_cb;
};

struct iterator {
        atomic_uint count;
        const struct iterator_callbacks *cbs;
};

/* API -----------------------------------------------------------------------*/

/**
 * @brief Initialize 'it' with 'cbs' as callbacks.
 */
void it_init(struct iterator *it, const struct iterator_callbacks *cbs);

#endif /* LIB_ITERATORS_PRIVATE_H */
