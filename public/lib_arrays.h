/**
 * @author Maxence ROBIN
 * @brief Provides simple static array manipulation.
 */

#ifndef LIB_ARRAYS_H
#define LIB_ARRAYS_H

/* Includes ------------------------------------------------------------------*/

#include "lib_iterators.h"
#include "lib_types.h"

#include <stddef.h>
#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct array;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates an array over 'data' of 'count' elements of 'type'.
 *
 * @return Pointer to the new array on success.
 * @return NULL if 'type' or 'data' are invalid.
 * @return NULL if for 'type', 'size' is 0, 'copy' 'comp' or 'destroy' are
 * invalid.
 */
struct array *array_create(
                const struct type_info *type, size_t count, void *data);

/**
 * @brief Destroys 'array'.
 */
void array_destroy(const void *array);

/**
 * @brief Sorts 'array' in ascending order.
 *
 * @return 0 on success.
 * @return -EINVAL if 'array' is invalid.
 */
int array_sort(struct array *array);

/**
 * @brief Sorts 'array' in ascending order following 'comp' rule.
 *
 * @return 0 on success.
 * @return -EINVAL if 'array' or 'comp' are invalid.
 */
int array_sort_by(struct array *array, type_comp_cb comp);

/**
 * @brief Returns the number of elements of 'array'.
 *
 * @return The number of elements of 'array' on success.
 * @return -EINVAL if 'array' is invalid.
 */
ssize_t array_len(const struct array *array);

/**
 * @brief Returns the data contained in 'array' at 'pos'.
 *
 * @return Pointer on data on success.
 * @return NULL if 'array' or 'pos' are invalid.
 */
void *array_value(struct array *array, unsigned int pos);

/**
 * @brief Returns the data contained in 'array'.
 *
 * @return Pointer on data on success.
 * @return NULL if 'array' is invalid.
 */
void *array_data(struct array *array);

/* Iterator API --------------------------------------------------------------*/

/**
 * @brief Creates an iterator over the first element of 'array'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'array' is invalid or on failure.
 */
struct iterator *array_begin(const struct array *array);

/**
 * @brief Creates an iterator over the last element of 'array'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'array' is invalid or on failure.
 */
struct iterator *array_end(const struct array *array);

/**
 * @brief Creates a reverse iterator over the last element of 'array'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'array' is invalid or on failure.
 */
struct iterator *array_rbegin(const struct array *array);

/**
 * @brief Creates a reverse iterator over the first element of 'array'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'array' is invalid or on failure.
 */
struct iterator *array_rend(const struct array *array);

#endif /* LIB_ARRAYS_H */
