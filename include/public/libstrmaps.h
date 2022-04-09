/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

#ifndef LIB_STRMAPS_H
#define LIB_STRMAPS_H

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <stdbool.h>
#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct strmap;

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates an empty strmap containin elements of 'type'.
 *
 * @return Pointer to the new strmap on success.
 * @return NULL on failure.
 */
struct strmap *strmap_create(const struct type_info *type);

/**
 * @brief Destroys 'map'.
 */
void strmap_destroy(const struct strmap *map);

/**
 * @brief Adds the pair <'key', 'value'> to 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map', 'key' or 'value' are invalid.
 * @return -EEXIST if 'key' already exists in 'map'.
 * @return -ENOMEM if the pair could not be added.
 *
 * @warning 'key' MUST be a NULL terminated string.
 */
int strmap_add(struct strmap *map, const char *key, const void *value);

/**
 * @brief Returns the value associated to 'key' inside 'map'.
 *
 * @return Pointer to the value on success.
 * @return NULL if 'map' or 'key' are invalid, or if the value could not be
 * found.
 *
 * @warning 'key' MUST be a NULL terminated string.
 */
void *strmap_get(const struct strmap *map, const char *key);

/**
 * @brief Removes 'key' from 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map' or 'key' are invalid.
 * @return -ENOENT if no value associated to 'key' was found.
 *
 * @warning 'key' MUST be a NULL terminated string.
 */
int strmap_remove(struct strmap *map, const char *key);

/**
 * @brief Clears 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map' is invalid.
 * @return -ENOMEM if the call failed.
 */
int strmap_clear(struct strmap *map);

#endif /* LIB_STRMAPS_H */
