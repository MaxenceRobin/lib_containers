/**
 * @author Maxence ROBIN
 * @brief Abstraction over containers for data structure manipulation.
 */

#ifndef LIB_ITERATORS_H
#define LIB_ITERATORS_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>

/* Definitions ---------------------------------------------------------------*/

struct iterator;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates a new iterator copied from 'it'.
 * 
 * @return Pointer to the new iterator on success.
 * @return NULL on failure. 
 */
struct iterator *iterator_copy(const struct iterator *it);

/**
 * @brief Destroys 'it'.
 */
void iterator_destroy(const struct iterator *it);

/**
 * @brief Indicates if 'it' is a valid iterator.
 * 
 * @return true if 'it' is valid.
 * @return false if 'it' is invalid.
 */
bool iterator_is_valid(const struct iterator *it);

/**
 * @brief Makes 'it' point to the next element of its container.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'it' is invalid.
 * @return -ENOTSUP if this operation is not supported.
 * @return A negative errno on failure.
 */
int iterator_next(struct iterator *it);

/**
 * @brief Makes 'it' point to the previous element of its container.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'it' is invalid.
 * @return -ENOTSUP if this operation is not supported.
 * @return A negative errno on failure. 
 */
int iterator_previous(struct iterator *it);

/**
 * @brief Returns the data pointed by 'it'.
 * 
 * @return The data of 'it' on success.
 * @return NULL on failure. 
 */
void *iterator_data(struct iterator *it);

#endif /* LIB_ITERATORS_H */