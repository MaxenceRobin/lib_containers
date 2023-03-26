/**
 * @author Maxence ROBIN
 * @brief Provides algorithms to manipulate containers
 */

#ifndef LIB_CONTAINER_ALGOS_H
#define LIB_CONTAINER_ALGOS_H

/* Includes ------------------------------------------------------------------*/

#include "lib_iterators.h"

#include <stdbool.h>

/* Definitions ---------------------------------------------------------------*/

typedef void (*ctn_action_cb)(void *, void *);
typedef bool (*ctn_match_cb)(const void *, void *);

/* API -----------------------------------------------------------------------*/

/**
 * @brief Calls 'action' for each elements starting from 'it' with 'arg' passed
 * as a second parameter.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'action' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_for_each(struct iterator *it, ctn_action_cb action, void *arg);

/**
 * @brief Counts all elements equal to 'value' starting from 'it'.
 *
 * @return The number of equal elements on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_count(struct iterator *it, const void *value);

/**
 * @brief Counts all elements matching 'match' starting from 'it' with 'arg'
 * passed as a second parameter.
 *
 * @return The number of matching elements on success.
 * @return -EINVAL if 'it' or 'match' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_count_if(struct iterator *it, ctn_match_cb match, void *arg);

/**
 * @brief Returns an iterator over the first element equal to 'value' starting
 * from 'it'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if no element was found.
 * @return NULL if 'it' or 'value' are invalid, or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
struct iterator *ctn_find(struct iterator *it, const void *value);

/**
 * @brief Returns an iterator over the first element matching 'match' starting
 * from 'it' with 'arg' passed as a second parameter.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if no element was found.
 * @return NULL if 'it' or 'match' are invalid, or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
struct iterator *ctn_find_if(
                struct iterator *it, ctn_match_cb match, void *arg);

/**
 * @brief Removes all elements equal to 'value' starting from 'it'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_remove(struct iterator *it, const void *value);

/**
 * @brief Removes all elements matching 'match' starting from 'it' with 'arg'
 * passed as a second parameter.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'match' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_remove_if(struct iterator *it, ctn_match_cb match, void *arg);

/**
 * @brief Keeps all elements equal to 'value' starting from 'it'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_keep(struct iterator *it, const void *value);

/**
 * @brief Keeps all elements matching 'match' starting from 'it' with 'arg'
 * passed as a second parameter.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'match' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_keep_if(struct iterator *it, ctn_match_cb match, void *arg);

/**
 * @brief Indicates if any element is equal to 'value' starting from 'it'.
 *
 * @return true if an element equal to 'value' was found.
 * @return false if no element was found.
 * @return false if 'it' or 'value' are invalid, or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
bool ctn_contains(struct iterator *it, const void *value);

/**
 * @brief Indicates if any element matches 'match' starting from 'it' with 'arg'
 * passed as a second parameter.
 *
 * @return true if an element matching 'match' was found.
 * @return false if no element was found.
 * @return false if 'it' or 'match' are invalid, or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
bool ctn_contains_if(struct iterator *it, ctn_match_cb match, void *arg);

/**
 * @brief Sets every elements to 'value' starting from 'it'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOMEM on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_fill(struct iterator *it, const void *value);

/**
 * @brief Returns an iterator over the lowest element starting from 'it'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'it' is invalid or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
struct iterator *ctn_min(struct iterator *it);

/**
 * @brief Returns an iterator over the greatest element starting from 'it'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'it' is invalid or on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
struct iterator *ctn_max(struct iterator *it);

/**
 * @brief Copies the lowest element into 'value' starting from 'it'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOENT on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_copy_min(struct iterator *it, void *value);

/**
 * @brief Copies the greatest element into 'value' starting from 'it'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'it' or 'value' are invalid.
 * @return -ENOENT on failure.
 *
 * @note it_unref() is called on 'it' at the end for convenience.
 * Use it_ref() when sending an iterator you want to keep.
 */
int ctn_copy_max(struct iterator *it, void *value);

#endif /* LIB_CONTAINER_ALGOS_H */
