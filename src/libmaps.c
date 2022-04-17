/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libmaps.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DEFAULT_BUCKET_LIST_COUNT 16

struct pair {
        void *key;
        void *value;
        unsigned long hash;
        struct pair *next;
};

struct bucket {
        struct pair *head;
};

struct map {
        const struct type_info *key_type;
        const struct type_info *value_type;
        unsigned int count;
        struct bucket *bucket_list;
        size_t bucket_count;
};

/* Static functions ----------------------------------------------------------*/

static struct bucket *get_bucket_from_key(
                const struct map *map, const void *key)
{
        const unsigned long i = map->key_type->hash(key) % map->bucket_count;
        return &(map->bucket_list[i]);
}

/* Pair API --------------------------*/

static struct pair *create_pair(
                const void *key,
                const void *value,
                const struct type_info *key_type,
                const struct type_info *value_type)
{
        struct pair *pair = malloc(sizeof(*pair));
        if (!pair)
                goto error_alloc_pair;

        pair->key = malloc(key_type->size);
        if (!pair->key)
                goto error_alloc_key;

        key_type->copy(pair->key, key);
        pair->value = malloc(value_type->size);
        if (!pair->value)
                goto error_alloc_value;

        value_type->copy(pair->value, value);
        pair->hash = key_type->hash(key);
        pair->next = NULL;

        return pair;

error_alloc_value:
        key_type->destroy(pair->key);
        free(pair->key);
error_alloc_key:
        free(pair);
error_alloc_pair:
        return NULL;
}

static void destroy_pair(
                struct pair *pair,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        destroy_key(pair->key);
        free(pair->key);
        destroy_value(pair->value);
        free(pair->value);
        free(pair);
}

static void destroy_pair_list(
                struct pair *head,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        while (head) {
                struct pair *next = head->next;
                destroy_pair(head, destroy_key, destroy_value);
                head = next;
        }
}

/* Bucket API ------------------------*/

static struct bucket *create_bucket_list(size_t count)
{
        struct bucket *bucket_list;
        const size_t bucket_list_size = sizeof(*bucket_list) * count;

        bucket_list = malloc(bucket_list_size);
        if (!bucket_list)
                return NULL;

        memset(bucket_list, 0, bucket_list_size);
        return bucket_list;
}

static void *get_value_from_bucket(
                const struct bucket *bucket, const void *key, type_comp_cb comp)
{
        struct pair *pair = bucket->head;

        while (pair) {
                if (comp(pair->key, key) == 0)
                        return pair->value;

                pair = pair->next;
        }

        return NULL;
}

static void add_pair_to_bucket_list(
                struct bucket *list, size_t count, struct pair *pair)
{
        unsigned long i = pair->hash % count;
        struct bucket *bucket = &(list[i]);

        pair->next = bucket->head;
        bucket->head = pair;
}

/* Map API ---------------------------*/

static void destroy_map_bucket_list(const struct map *map)
{
        for (unsigned int i = 0; i < map->bucket_count; ++i) {
                destroy_pair_list(
                                map->bucket_list[i].head,
                                map->key_type->destroy,
                                map->value_type->destroy);
        }

        free(map->bucket_list);
}

static int resize_map_bucket_list(struct map *map)
{
        const size_t new_count = map->bucket_count * 2;
        struct bucket *new_list = create_bucket_list(new_count);
        if (!new_list)
                return -ENOMEM;

        for (unsigned int i = 0; i < map->count; ++i) {
                struct pair *pair = map->bucket_list[i].head;

                while (pair) {
                        struct pair *next = pair->next;
                        add_pair_to_bucket_list(new_list, new_count, pair);
                        pair = next;
                }
        }

        free(map->bucket_list);
        map->bucket_list = new_list;
        map->bucket_count = new_count;

        return 0;
}

static void *get_value_from_map(const struct map *map, const void *key)
{
        const struct bucket *bucket = get_bucket_from_key(map, key);
        return get_value_from_bucket(bucket, key, map->key_type->comp);
}

/* Public API ----------------------------------------------------------------*/

struct map *map_create(
                const struct type_info *key_type,
                const struct type_info *value_type)
{
        if (!key_type || key_type->size == 0 || !key_type->copy
                        || !key_type->comp || !key_type->hash
                        || !key_type->destroy)
                return NULL;

        if (!value_type || value_type->size == 0 || !value_type->copy
                        || !value_type->destroy)
                return NULL;

        struct map *map = malloc(sizeof(*map));
        if (!map)
                return NULL;

        map->bucket_list = create_bucket_list(DEFAULT_BUCKET_LIST_COUNT);
        if (!map->bucket_list) {
                free(map);
                return NULL;
        }

        map->key_type = key_type;
        map->value_type = value_type;
        map->bucket_count = DEFAULT_BUCKET_LIST_COUNT;
        map->count = 0;

        return map;
}

void map_destroy(const struct map *map)
{
        if (!map)
                return;

        destroy_map_bucket_list(map);
        free((void *)map);
}

int map_add(struct map *map, const void *key, const void *value)
{
        if (!map || !key || !value)
                return -EINVAL;

        if (get_value_from_map(map, key))
                return -EEXIST;

        struct pair *pair = create_pair(
                        key, value, map->key_type, map->value_type);
        if (!pair)
                return -ENOMEM;

        if (map->count == map->bucket_count) {
                if (resize_map_bucket_list(map) < 0) {
                        destroy_pair(pair, map->key_type->destroy,
                                        map->value_type->destroy);
                        return -ENOMEM;
                }
        }

        add_pair_to_bucket_list(map->bucket_list, map->bucket_count, pair);
        ++map->count;

        return 0;
}

void *map_get(const struct map *map, const void *key)
{
        if (!map || !key)
                return NULL;

        return get_value_from_map(map, key);
}

int map_remove(struct map *map, const void *key)
{
        if (!map || !key)
                return -EINVAL;

        struct bucket *bucket = get_bucket_from_key(map, key);
        struct bucket *previous = NULL;

        while (bucket->head) {
                if (map->key_type->comp(bucket->head->key, key) != 0) {
                        previous->head = bucket->head;
                        bucket->head = bucket->head->next;
                        continue;
                }

                /* Match found */
                struct pair *next = bucket->head->next;
                destroy_pair(bucket->head, map->key_type->destroy,
                                map->value_type->destroy);
                --map->count;

                if (!previous)
                        bucket->head = next;
                else
                        previous->head->next = next;

                return 0;
        }

        return -ENOENT;
}

int map_clear(struct map *map)
{
        if (!map)
                return -EINVAL;

        struct bucket *new_list = create_bucket_list(DEFAULT_BUCKET_LIST_COUNT);
        if (!new_list)
                return -ENOMEM;

        destroy_map_bucket_list(map);
        map->bucket_list = new_list;
        map->bucket_count = DEFAULT_BUCKET_LIST_COUNT;
        map->count = 0;

        return 0;
}
