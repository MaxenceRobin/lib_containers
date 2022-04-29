/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

#ifndef LIB_MAPS_H
#define LIB_MAPS_H

/* Includes ------------------------------------------------------------------*/

#include "libiterators.h"
#include "libtypes.h"

#include <stdbool.h>
#include <sys/types.h>

/* Definitions ---------------------------------------------------------------*/

struct map;

struct pair {
        const void *const key;
        void *value;
};

enum map_it_type {
        MAP_IT_TYPE_VALUE,
        MAP_IT_TYPE_PAIR
};

/* API -----------------------------------------------------------------------*/

/**
 * @brief Creates an empty map containing pairs of <'key_type', 'value_type'>.
 *
 * @return Pointer to the new map on success.
 * @return NULL if 'key_type' or 'value_type' are invalid.
 * @return NULL if for 'key_type', 'size' is 0, 'copy' 'comp' 'hash' or
 * 'destroy' are invalid.
 * @return NULL if for 'value_type', 'size' is 0, 'copy' or 'destroy' are
 * invalid.
 */
struct map *map_create(
                const struct type_info *key_type,
                const struct type_info *value_type);

/**
 * @brief Destroys 'map'.
 */
void map_destroy(const struct map *map);

/**
 * @brief Adds the pair <'key', 'value'> to 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map', 'key' or 'value' are invalid.
 * @return -EEXIST if 'key' already exists in 'map'.
 * @return -ENOMEM if the pair could not be added.
 */
int map_add(struct map *map, const void *key, const void *value);

/**
 * @brief Returns the value associated to 'key' inside 'map'.
 *
 * @return Pointer to the value on success.
 * @return NULL if 'map' or 'key' are invalid, or if the value could not be
 * found.
 */
void *map_value(const struct map *map, const void *key);

/**
 * @brief Returns the <key, value> pair associated to 'key' inside 'map'.
 *
 * @return Pointer to the pair on success.
 * @return NULL if 'map' or 'key' are invalid, or if the pair could not be
 * found.
 */
struct pair *map_pair(const struct map *map, const void *key);

/**
 * @brief Removes 'key' from 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map' or 'key' are invalid.
 * @return -ENOENT if no value associated to 'key' was found.
 */
int map_remove(struct map *map, const void *key);

/**
 * @brief Clears 'map'.
 *
 * @return 0 on success.
 * @return -EINVAL if 'map' is invalid.
 * @return -ENOMEM if the call failed.
 */
int map_clear(struct map *map);

/* Iterator API --------------------------------------------------------------*/

/**
 * @brief Creates an iterator of over the first element of 'map'. The iterator
 * iterates over values or pairs depending on 'type'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'map' is invalid or on failure.
 */
struct iterator *map_begin(const struct map *map, enum map_it_type type);

/**
 * @brief Creates an iterator over the last element of 'map'. The iterator
 * iterates over values or pairs depending on 'type'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'map' is invalid or on failure.
 */
struct iterator *map_end(const struct map *map, enum map_it_type type);

/**
 * @brief Creates a reverse iterator over the last element of 'map'. The
 * iterator iterates over values or pairs depending on 'type'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'map' is invalid or on failure.
 */
struct iterator *map_rbegin(const struct map *map, enum map_it_type type);

/**
 * @brief Creates a reverse iterator over the first element of 'map'. The
 * iterator iterates over values or pairs depending on 'type'.
 *
 * @return Pointer to the iterator on success.
 * @return NULL if 'map' is invalid or on failure.
 */
struct iterator *map_rend(const struct map *map, enum map_it_type type);

#endif /* LIB_MAPS_H */
