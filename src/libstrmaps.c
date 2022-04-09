/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libstrmaps.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DEFAULT_BUCKET_LIST_COUNT 8

struct pair {
        char *key;
        void *value;
        unsigned long hash;
        struct pair *next;
};

struct bucket {
        struct pair *head;
};

struct strmap {
        const struct type_info *type;
        unsigned int count;
        struct bucket *bucket_list;
        size_t bucket_count;
};

/* Static functions ----------------------------------------------------------*/

/* Utility functions -----------------*/

static unsigned long hash_str(const char *key)
{
        unsigned long hash = 5381;
        unsigned char c;

        while (c = *key++)
                hash = ((hash << 5) + hash) + c;

        return hash;
}

static struct bucket *get_bucket_from_key(
                const struct strmap *map, const char *key)
{
        const unsigned long i = hash_str(key) % map->bucket_count;
        return &(map->bucket_list[i]);
}

/* Pair API --------------------------*/

static struct pair *create_pair(
                const char *key,
                const void *value,
                const struct type_info *type)
{
        struct pair *pair = malloc(sizeof(*pair));
        if (!pair)
                goto error_alloc_pair;

        pair->key = strdup(key);
        if (!pair->key)
                goto error_copy_key;

        pair->value = malloc(type->size);
        if (!pair->value)
                goto error_alloc_value;

        type->copy(pair->value, value);
        pair->hash = hash_str(key);
        pair->next = NULL;

        return pair;

error_alloc_value:
error_copy_key:
        free(pair);
error_alloc_pair:
        return NULL;
}

static void destroy_pair(struct pair *pair, type_destroy_cb destroy_cb)
{
        free(pair->key);
        destroy_cb(pair->value);
        free(pair->value);
        free(pair);
}

static void destroy_pair_list(struct pair *head, type_destroy_cb destroy_cb)
{
        while (head) {
                struct pair *next = head->next;
                destroy_pair(head, destroy_cb);
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

static void *get_value_from_bucket(const struct bucket *bucket, const char *key)
{
        struct pair *pair = bucket->head;

        while (pair) {
                if (strcmp(pair->key, key) == 0)
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

static void destroy_map_bucket_list(const struct strmap *map)
{
        for (unsigned int i = 0; i < map->bucket_count; ++i)
                destroy_pair_list(map->bucket_list[i].head, map->type->destroy);

        free(map->bucket_list);
}

static int resize_map_bucket_list(struct strmap *map)
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

static void *get_value_from_map(const struct strmap *map, const char *key)
{
        const struct bucket *bucket = get_bucket_from_key(map, key);
        return get_value_from_bucket(bucket, key);
}

/* Public API ----------------------------------------------------------------*/

struct strmap *strmap_create(const struct type_info *type)
{
        if (!type)
                return NULL;

        struct strmap *map = malloc(sizeof(*map));
        if (!map)
                return NULL;

        map->bucket_list = create_bucket_list(DEFAULT_BUCKET_LIST_COUNT);
        if (!map->bucket_list) {
                free(map);
                return NULL;
        }

        map->type = type;
        map->bucket_count = DEFAULT_BUCKET_LIST_COUNT;
        map->count = 0;

        return map;
}

void strmap_destroy(const struct strmap *map)
{
        if (!map)
                return;

        destroy_map_bucket_list(map);
        free((void *)map);
}

int strmap_add(struct strmap *map, const char *key, const void *value)
{
        if (!map || !key || !value)
                return -EINVAL;

        if (get_value_from_map(map, key))
                return -EEXIST;

        struct pair *pair = create_pair(key, value, map->type);
        if (!pair)
                return -ENOMEM;

        if (map->count == map->bucket_count) {
                if (resize_map_bucket_list(map) < 0) {
                        destroy_pair(pair, map->type->destroy);
                        return -ENOMEM;
                }
        }

        add_pair_to_bucket_list(map->bucket_list, map->bucket_count, pair);
        ++map->count;

        return 0;
}

void *strmap_get(const struct strmap *map, const char *key)
{
        if (!map || !key)
                return NULL;

        return get_value_from_map(map, key);
}

int strmap_remove(struct strmap *map, const char *key)
{
        if (!map || !key)
                return -EINVAL;

        struct bucket *bucket = get_bucket_from_key(map, key);
        struct bucket *previous = NULL;

        while (bucket->head) {
                if (strcmp(bucket->head->key, key) != 0) {
                        previous->head = bucket->head;
                        bucket->head = bucket->head->next;
                        continue;
                }

                /* Match found */
                struct pair *next = bucket->head->next;
                destroy_pair(bucket->head, map->type->destroy);
                --map->count;

                if (!previous)
                        bucket->head = next;
                else
                        previous->head->next = next;

                return 0;
        }

        return -ENOENT;
}

int strmap_clear(struct strmap *map)
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
