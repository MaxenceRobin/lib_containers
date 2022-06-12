/**
 * @author Maxence ROBIN
 * @brief Provides simple doubly-linked list manipulation
 */

#ifndef LIB_LISTS_H
#define LIB_LISTS_H

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"
#include "libtypes.h"

#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct list;

struct node;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates an empty list of 'type'.
 *
 * @return Pointer to the created list on success.
 * @return NULL if 'type' is invalid.
 * @return NULL if for 'type', 'size' is 0, 'copy' or 'destroy' are invalid.
 */
struct list *list_create(const struct type_info *type);

/**
 * @brief Destroys 'list'.
 */
void list_destroy(const struct list *list);

/**
 * @brief Adds 'value' at the beginning of 'list'.
 *
 * @return Pointer to the new node on success.
 * @return NULL if 'list' or 'value' are invalid, or on failure.
 */
struct node *list_push_front(struct list *list, const void *value);

/**
 * @brief Adds 'value' at the end of 'list'.
 *
 * @return Pointer to the new node on success.
 * @return NULL if 'list' or 'value' are invalid, or on failure.
 */
struct node *list_push_back(struct list *list, const void *value);

/**
 * @brief Inserts 'value' inside 'list' at the position pointed by 'node'.
 * 'node' is the next node of the newly inserted one after the call.
 *
 * @return Pointer to the new node on success.
 * @return NULL if 'list', 'node' or 'value' are invalid, or on failure.
 */
struct node *list_insert(
                struct list *list, struct node *node, const void *value);

/**
 * @brief Removes the first element of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' is invalid.
 */
int list_pop_front(struct list *list);

/**
 * @brief Removes the last element of 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' is invalid.
 */
int list_pop_back(struct list *list);

/**
 * @brief Removes 'node' from 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' or 'node' are invalid.
 *
 * @warning 'node' SHOULD NOT be used after calling this function.
 */
int list_remove(struct list *list, const struct node *node);

/**
 * @brief Clears 'list'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'list' is invalid.
 */
int list_clear(struct list *list);

/**
 * @brief Returns the number of elements inside 'list'.
 *
 * @return The number of elements inside 'list' on success.
 * @return -EINVAL if 'list' is invalid.
 */
ssize_t list_len(const struct list *list);

/**
 * @brief Returns the first node of 'list'.
 *
 * @return Pointer to the node on success.
 * @return NULL if 'list' is invalid.
 */
struct node *list_first(const struct list *list);

/**
 * @brief Returns the last node of 'list'.
 *
 * @return Pointer to the node on success.
 * @return NULL if 'list' is invalid.
 */
struct node *list_last(const struct list *list);

/**
 * @brief Returns the 'pos'th node of 'list', startin at 0.
 *
 * @return Pointer to the node on success.
 * @return NULL if 'list' is invalid or 'pos' is out of bounds.
 */
struct node *list_node(const struct list *list, unsigned int pos);

/**
 * @brief Returns the next node of 'node'.
 *
 * @return Pointer to the node on success.
 * @return NULL if 'node' is invalid.
 */
struct node *node_next(const struct node *node);

/**
 * @brief Returns the previous node of 'node'.
 *
 * @return Pointer to the node on success.
 * @return NULL if 'node' is invalid.
 */
struct node *node_previous(const struct node *node);

/**
 * @brief Indicates if 'node' is valid.
 *
 * @return true if 'node' is valid.
 * @return false if 'node' is NULL or is invalid.
 */
bool node_is_valid(const struct node *node);

/**
 * @brief Returns the data of 'node'.
 *
 * @return Pointer to the data on success.
 * @return NULL if 'node' is invalid.
 */
void *node_data(struct node *node);

/* Iterator API --------------------------------------------------------------*/

/**
 * @brief Creates an iterator over the first element of 'list'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'list' is invalid or on failure.
 */
struct iterator *list_begin(const struct list *list);

/**
 * @brief Creates an iterator over the last element of 'list'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'list' is invalid or on failure.
 */
struct iterator *list_end(const struct list *list);

/**
 * @brief Creates a reverse iterator over the last element of 'list'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'list' is invalid or on failure.
 */
struct iterator *list_rbegin(const struct list *list);

/**
 * @brief Creates a reverse iterator over the first element of 'list'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'list' is invalid or on failure.
 */
struct iterator *list_rend(const struct list *list);

#endif /* LIB_LISTS_H */
