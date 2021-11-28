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
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 0 on success and a negative
 * errno on failure.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return -ENOMEM if the first element failed.
 * @return A negative errno if the iteration failed.
 * @return A custom negative errno if a call to 'cb' failed.
 */
int container_for_each(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data);

/**
 * @brief Calls 'cb' with 'data' for each element of 'ctx', in reverse order. 
 * 
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 0 on success and a negative
 * errno on failure.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return -ENOMEM if the last element failed.
 * @return A negative errno if the iteration failed.
 * @return A custom negative errno if a call to 'cb' failed.
 */
int container_for_each_r(
                const struct container *ctx,
                int (*cb)(void *, void *),
                void *data);

/**
 * @brief Filters every element of 'ctx' according to 'cb'.
 * 
 * @param cb : For each call, the element of 'ctx' will be passed. It must
 * return 'true' to keep it and 'false' to discard it.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return A negative errno if the iteration failed.
 */
int container_filter(const struct container *ctx, bool (*cb)(void *));

#endif /* LIB_CONTAINERS_H */
