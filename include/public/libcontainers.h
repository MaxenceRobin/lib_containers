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
 * @brief Inserts 'data' inside 'ctx' where 'it' points to.
 * 
 * @return 0 on success.
 * @return -ENOTSUP if this operation is not supported.
 * @return -EINVAL if 'ctx', 'it' or 'data' are invalid.
 * @return A negative errno for other failures.
 */
int container_insert(
                struct container *ctx, struct iterator *it, const void *data);

/**
 * @brief Removes the data pointed by 'it' inside 'ctx'.
 * 
 * @return 0 on success.
 * @return -ENOTSUP if this operation is not supported.
 * @return -EINVAL if 'ctx' or 'it' are invalid.
 * @return A negative errno for other failures.
 */
int container_remove(struct container *ctx, const struct iterator *it);

/**
 * @brief Calls 'cb' for each element of 'ctx', with 'data' as context.
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
                int (*cb)(const void *, void *),
                void *data);

/**
 * @brief Calls 'cb' for each element of 'ctx' in reverse order, with 'data' as
 * context. 
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
                int (*cb)(const void *, void *),
                void *data);

/**
 * @brief Filters elements of 'ctx' based on 'cb', with 'data' as context.
 *  
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 'true' if the element must be
 * kept in the container, and 'false' if it must be removed.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return -ENOMEM if the first element failed.
 * @return A negative errno if the iteration failed.
 */
int container_remove_if(
                struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

/**
 * @brief Finds 'data' inside 'ctx'.
 * 
 * @return An iterator over the found data on success.
 * @return NULL on failure. 
 */
struct iterator *container_find(const struct container *ctx, const void *data);

/**
 * @brief Indicates if 'ctx' contains 'data'.
 * 
 * @return true if 'ctx' contains 'data'.
 * @return false if 'ctx' does not contain 'data', if 'ctx' or 'data' are
 * invalid or if the iteration failed.
 */
bool container_contains(const struct container *ctx, const void *data);

#endif /* LIB_CONTAINERS_H */
