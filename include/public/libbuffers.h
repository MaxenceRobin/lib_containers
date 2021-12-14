/**
 * @author Maxence ROBIN
 * @brief Provides simple circular buffers manipulation
 */

#ifndef LIB_BUFFERS_H
#define LIB_BUFFERS_H

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct buffer;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates a new buffer of 'count' elements of type 'type'.
 *
 * @return Pointer to the new buffer on success.
 * @return NULL on failure.
 */
struct buffer *buffer_create(const struct type_info *type, size_t count);

/**
 * @brief Destroys 'buffer'.
 */
void buffer_destroy(const struct buffer *buffer);

/**
 * @brief Adds 'data' into 'buffer'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'buffer' or 'data' are invalid.
 * @return -ENOBUFS if 'buffer' is full.
 */
int buffer_push(struct buffer *buffer, const void *data);

/**
 * @brief Removes the first value of 'buffer'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'buffer' is invalid.
 * @return -ENOMEM if 'buffer' is empty.
 */
int buffer_pop(struct buffer *buffer);

/**
 * @brief Returns the last value of 'buffer'.
 *
 * @return Pointer to the value on success.
 * @return NULL if 'buffer' is invalid or if 'buffer' is empty.
 */
const void *buffer_data(const struct buffer *buffer);

/**
 * @brief Indicates if 'buffer' is empty.
 *
 * @return true if 'buffer' is empty.
 * @return false if 'buffer' is not empty OR if 'buffer' is invalid.
 */
bool buffer_is_empty(const struct buffer *buffer);

/**
 * @brief Indicates if 'buffer' is full.
 *
 * @return true if 'buffer' is full.
 * @return false if 'buffer' is not full OR if 'buffer' is invalid.
 */
bool buffer_is_full(const struct buffer *buffer);

/**
 * @brief Returns the number of elements of 'buffer' defined at creation.
 *
 * @return Number of elements of 'buffer' on success.
 * @return -EINVAL on failure.
 */
ssize_t buffer_count(const struct buffer *buffer);

#endif /* LIB_BUFFERS_H */