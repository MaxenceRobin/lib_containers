/**
 * @author Maxence ROBIN
 * @brief Provides generic functions for containers manipulation and algorithms.
 */

#ifndef LIB_CONTAINERS_H
#define LIB_CONTAINERS_H

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"

/* Definitions ---------------------------------------------------------------*/

struct container;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Returns an iterator on the first element of 'ctx'.
 * 
 * @return Pointer to the first element on success.
 * @return NULL if 'ctx' is invalid or does not support first iterator access.
 */
struct iterator *container_first(const struct container *ctx);

/**
 * @brief Returns an iterator on the last element of 'ctx'.
 * 
 * @return Pointer to the last element on success.
 * @return NULL if 'ctx' is invalid or does not support last iterator access.
 */
struct iterator *container_last(const struct container *ctx);

/**
 * @brief Calls 'cb' with 'data' for each element of 'ctx'.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return -ENOMEM if the first element failed.
 * @return A negative errno if the iteration failed.
 * @return A custom negative errno if a call to 'cb' failed.
 * 
 * @note For each call to 'cb', the element of 'ctx' will be the first argument
 * and 'data' will be the second.
 * @warning 'cb' MUST return a negative errno on failure, and 0 on success.
 */
int container_for_each(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data);

/**
 * @brief Calls 'cb' with 'data' for each element of 'ctx', in reverse order. 
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return -ENOMEM if the last element failed.
 * @return A negative errno if the iteration failed.
 * @return A custom negative errno if a call to 'cb' failed.
 * 
 * @note For each call to 'cb', the element of 'ctx' will be the first argument
 * and 'data' will be the second.
 * @warning 'cb' MUST return a negative errno on failure, and 0 on success.
 */
int container_for_each_r(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data);

#endif /* LIB_CONTAINERS_H */
