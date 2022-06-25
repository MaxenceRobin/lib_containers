/**
 * @author Maxence ROBIN
 * @brief Provides simple iterators manipulation.
 */

#ifndef LIB_ITERATORS_H
#define LIB_ITERATORS_H

/* Includes ------------------------------------------------------------------*/

#include "lib_types.h"

#include <stdbool.h>

/* Definition ----------------------------------------------------------------*/

struct iterator;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Increases the reference count of 'it'.
 *
 * @return 'it' itself for convenience.
 * @return NULL if 'it' is invalid.
 */
struct iterator *it_ref(struct iterator *it);

/**
 * @brief Decreses the reference count of 'it'. If the reference count reaches 0
 * 'it' will be destroyed.
 *
 * @return 'it' itself for convenience if it's not destroyed.
 * @return NULL if 'it' gets destroyed during the call or if 'it' is invalid.
 */
struct iterator *it_unref(struct iterator *it);

/**
 * @brief Change the value pointed by 'it' to the next one.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' is invalid.
 */
int it_next(struct iterator *it);

/**
 * @brief Change the value pointed by 'it' to the previous one.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' is invalid.
 */
int it_previous(struct iterator *it);

/**
 * @brief Indicates if 'it' points to a valid value.
 *
 * @return true if 'it' points to a valid value.
 * @return -false if 'it' points to an invalid value or if 'it' is invalid.
 */
bool it_is_valid(const struct iterator *it);

/**
 * @brief Returns the value pointed by 'it'.
 *
 * @return The value pointed by 'it' on success.
 * @return NULL if 'it' is invalid.
 */
void *it_data(const struct iterator *it);

/**
 * @brief Returns the type_info of 'it".
 *
 * @return Pointer to the type_info on success.
 * @return NULL if 'it' is invalid.
 */
const struct type_info *it_type(const struct iterator *it);

/**
 * @brief Removes the value pointed by 'it'. After the call 'it' points to the
 * next element of the removed one.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' is invalid.
 */
int it_remove(struct iterator *it);

/**
 * @brief Returns a duplicate of 'it'.
 *
 * @return A duplicate of 'it' on success.
 * @return NULL on failure.
 */
struct iterator *it_dup(const struct iterator *it);

/**
 * @brief Makes 'dest' identical to 'src'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'dest' or 'src' are invalid.
 */
int it_copy(struct iterator *dest, const struct iterator *src);

#endif /* LIB_ITERATORS_H */
