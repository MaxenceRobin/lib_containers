/**
 * @author Maxence ROBIN
 * @brief Provides simple dynamic array manipulation.
 */

#ifndef LIB_VECTORS_H
#define LIB_VECTORS_H

/* Includes ------------------------------------------------------------------*/

#include "libcontainers.h"
#include "libtypes.h"

#include <stddef.h>
#include <sys/types.h>

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates a vector of 'count' elements of 'type'.
 * 
 * @return Pointer to the new vector on success.
 * @return NULL on failure. 
 */
void *vector_create(const struct type_info *type, size_t count);

/**
 * @brief Destroys 'vector'.
 */
void vector_destroy(const void *vector);

/**
 * @brief Adds the value pointer by 'data' at the end of 'vector'.
 * 
 * @return Pointer to a valid vector, if it was modified or not.
 * 
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -EINVAL if 'vector' or 'data' are invalid.
 *      -ENOMEM if the value could not be added.
 */
void *vector_push(void *vector, const void *data, int *ret);

/**
 * @brief Removes the last element of 'vector'.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'vector' is invalid.
 * @return -ENOBUFS if 'vector' is already empty. 
 */
int vector_pop(void *vector);

/**
 * @brief Inserts 'data' at 'pos' inside 'vector'.
 * 
 * @return Pointer to a valid vector, if it was modified or not.
 * 
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -EINVAL if 'vector' or 'data' are invalid.
 *      -ERANGE if 'pos' is out of bounds.
 *      -ENOMEM if the valud could not be inserted.
 */
void *vector_insert(void *vector, unsigned int pos, const void *data, int *ret);

/**
 * @brief Removes the value at 'pos' inside 'vector'.
 * 
 * @return 0 on success.
 * @return -EINVAL if 'vector' is invalid.
 * @return -ERANGE if 'pos' is out of bounds. 
 */
int vector_remove(void *vector, unsigned int pos);

/**
 * @brief Allocates enough memory for 'vector' to contain 'count' elements
 * without changing its length. If 'count' is lower than the current capacity
 * of 'vector', this function does nothing.
 * 
 * @return Pointer to a valid vector, if it was modified or not.
 * 
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -EINVAL if 'vector' is invalid.
 *      -ENOMEM if the reallocation failed.
 */
void *vector_reserve(void *vector, size_t count, int *ret);

/**
 * @brief Reallocates 'vector' in order for its capacity to match its lenght.
 * 
 * @return Pointer to a valid vector, if it was modified or not.
 * 
 * @note If 'ret' is not NULL, its value will be modified to indicate if the
 * operation was successful or not :
 *      0 on success.
 *      -EINVAL if 'vector' is invalid.
 *      -ENOMEM if the reallocation failed. 
 */
void *vector_fit(void *vector, int *ret);

/**
 * @brief Returns the associated container of 'vector'.
 * 
 * @return Pointer to the container of 'vector' on success.
 * @return NULL if 'vector' is invalid. 
 */
struct container *vector_container(void *vector);

/**
 * @brief Returns the number of elements of 'vector'.
 * 
 * @return The number of elements of 'vector' on success.
 * @return -EINVAL if 'vector' is invalid.
 */
ssize_t vector_len(const void *vector);

/**
 * @brief Returns the number of elements that 'vector' can hold without
 * reallocating memory.
 * 
 * @return The capacity of 'vector' on success.
 * @return -EINVAL if 'vector' is invalid. 
 */
ssize_t vector_capacity(const void *vector);

#endif /* LIB_VECTORS_H */
