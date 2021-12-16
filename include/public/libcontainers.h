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

/* Callback implementations ----------*/

/**
 * @brief Returns an iterator on the first element of 'ctx'.
 *
 * @return Pointer to the first element on success.
 * @return NULL if 'ctx' is invalid or does not support first iterator access.
 */
struct iterator *ctn_first(const struct container *ctx);

/**
 * @brief Returns an iterator on the last element of 'ctx'.
 *
 * @return Pointer to the last element on success.
 * @return NULL if 'ctx' is invalid or does not support last iterator access.
 */
struct iterator *ctn_last(const struct container *ctx);

/**
 * @brief Inserts 'data' inside 'ctx' where 'it' points to.
 *
 * @return 0 on success.
 * @return -ENOTSUP if this operation is not supported.
 * @return -EINVAL if 'ctx', 'it' or 'data' are invalid.
 * @return A negative errno for other failures.
 */
int ctn_insert_it(struct container *ctx, struct iterator *it, const void *data);

/**
 * @brief Removes the data pointed by 'it' inside 'ctx'.
 *
 * @return 0 on success.
 * @return -ENOTSUP if this operation is not supported.
 * @return -EINVAL if 'ctx' or 'it' are invalid.
 * @return A negative errno for other failures.
 */
int ctn_remove_it(struct container *ctx, const struct iterator *it);

/* Algorithms ------------------------*/

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
int ctn_for_each(
                struct container *ctx,
                int (*cb)(struct iterator *, void *),
                void *data);

/**
 * @brief Returns the number of elements matching 'data' inside 'ctx'.
 *
 * @return The number of matching elements on success.
 * @return -EINVAL if 'ctx' or 'data' are invalid.
 * @return A negative errno on failure.
 */
int ctn_count(const struct container *ctx, const void *data);

/**
 * @brief Returns the number of elements matching 'cb' inside 'ctx'.
 *
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 'true' if the element matches
 * the criteria and 'false' otherwise.
 *
 * @return The number of matchin elements on success.
 * @return -EINVAL if 'ctx' or 'cb' are invalid.
 * @return A negative errno on failure.
 */
int ctn_count_if(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

/**
 * @brief Finds 'data' inside 'ctx'.
 *
 * @return An iterator over the found data on success.
 * @return NULL on failure.
 */
struct iterator *ctn_find(const struct container *ctx, const void *data);

/**
 * @brief Finds an element matching 'cb' inside 'ctx'.
 *
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 'true' if the element matches
 * the criteria and 'false' otherwise.
 *
 * @return An iterator over the found data on success.
 * @return NULL on failure.
 */
struct iterator *ctn_find_if(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

/**
 * @brief Indicates if 'ctx' contains 'data'.
 *
 * @return true if 'ctx' contains 'data'.
 * @return false if 'ctx' does not contain 'data', if 'ctx' or 'data' are
 * invalid or if the iteration failed.
 */
bool ctn_contains(const struct container *ctx, const void *data);

/**
 * @brief Indicates if any of the elements of 'ctx' are matching 'cb'.
 *
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 'true' if the element matches
 * the criteria and 'false' otherwise.
 *
 * @return true if any element of 'ctx' matches the criteria.
 * @return false if no element matches, if 'ctx' or 'cb' are invalid or if the
 * iteration failed.
 */
bool ctn_any_of(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

/**
 * @brief Indicates if all the elements of 'ctx' are matchin 'cb'.
 *
 * @param cb : For each call, the element of 'ctx' will be the first argument
 * and 'data' will be the second. It must return 'true' if the element matches
 * the criteria and 'false' otherwise.
 *
 * @return true if all the elements of 'ctx' are matching the criteria.
 * @return false if any element does not match, if 'ctx' or 'cb' are invalid or
 * if the iteration failed.
 */
bool ctn_all_of(
                const struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

/**
 * @brief Fills 'ctx' with 'data'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'data' are invalid.
 * @return A negative errno on failure.
 */
int ctn_fill(struct container *ctx, const void *data);

/**
 * @brief Fills 'ctx' with data provided by 'cb'.
 *
 * @param cb : For each call, data' will be passed. It must return a value to be
 * inserted inside 'ctx'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'data' are invalid.
 * @return A negative errno on failure.
 */
int ctn_generate(struct container *ctx, const void *(*cb)(void *), void *data);

/**
 * @brief Returns the minimum value inside 'ctx'.
 *
 * @return Pointer to the minimum value on success.
 * @return NULL if 'ctx' is invalid or on failure.
 */
const void *ctn_min(const struct container *ctx);

/**
 * @brief Returns the maximum value inside 'ctx'.
 *
 * @return Pointer to the maximum value on success.
 * @return NULL if 'ctx' is invalid or on failure.
 */
const void *ctn_max(const struct container *ctx);

/**
 * @brief Removes the first element matching 'data' inside 'ctx'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'ctx' or 'data' are invalid.
 * @return A negative errno on failure.
 */
int ctn_remove(struct container *ctx, const void *data);

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
int ctn_remove_if(
                struct container *ctx,
                bool (*cb)(const void *, void *),
                void *data);

#endif /* LIB_CONTAINERS_H */
