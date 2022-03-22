/**
 * @author Maxence ROBIN
 * @brief Provides simple doubly-linked list manipulation
 */

#ifndef LIB_LISTS_H
#define LIB_LISTS_H

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct list;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates an empty list of 'type'.
 *
 * @return Pointer to the created list on success.
 * @return NULL on failure.
 */
struct list *list_create(const struct type_info *type);

/**
 * @brief Destroys 'list'.
 */
void list_destroy(const struct list *list);

/**
 * @brief Adds 'value' at the beginning of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' or 'value' are invalid.
 * @return -ENOMEM if on failure.
 */
int list_push_front(struct list *list, const void *value);

/**
 * @brief Adds 'value' at the end of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' or 'value' are invalid.
 * @return -ENOMEM on failure.
 */
int list_push_back(struct list *list, const void *value);

/**
 * @brief Removes the first element of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' is invalid.
 * @return -ENOMEM on failure.
 */
int list_pop_front(struct list *list);

/**
 * @brief Removes the last element of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' is invalid.
 * @return -ENOMEM on failure.
 */
int list_pop_back(struct list *list);

/**
 * @brief Returns the number of elements inside 'list'.
 *
 * @return The number of elements inside 'list' on success.
 * @return -EINVAL if 'list' is invalid.
 */
ssize_t list_len(const struct list *list);

#endif /* LIB_LISTS_H */
