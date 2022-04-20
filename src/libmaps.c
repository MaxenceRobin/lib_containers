/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "libiterators_private.h"
#include "libmaps.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DEFAULT_BUCKET_LIST_COUNT 16

struct holder {
        struct pair pair;
        unsigned long hash;
        struct holder *next;
};

struct bucket {
        struct holder *head;
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

/* Holder API --------------------------*/

static struct holder *create_holder(
                const void *key,
                const void *value,
                const struct type_info *key_type,
                const struct type_info *value_type)
{
        struct holder *holder = malloc(sizeof(*holder));
        if (!holder)
                goto error_alloc_holder;

        holder->pair.key = malloc(key_type->size);
        if (!holder->pair.key)
                goto error_alloc_key;

        key_type->copy(holder->pair.key, key);
        holder->pair.value = malloc(value_type->size);
        if (!holder->pair.value)
                goto error_alloc_value;

        value_type->copy(holder->pair.value, value);
        holder->hash = key_type->hash(key);
        holder->next = NULL;

        return holder;

error_alloc_value:
        key_type->destroy(holder->pair.key);
        free(holder->pair.key);
error_alloc_key:
        free(holder);
error_alloc_holder:
        return NULL;
}

static void destroy_holder(
                struct holder *holder,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        destroy_key(holder->pair.key);
        free(holder->pair.key);
        destroy_value(holder->pair.value);
        free(holder->pair.value);
        free(holder);
}

static void destroy_holder_list(
                struct holder *head,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        while (head) {
                struct holder *next = head->next;
                destroy_holder(head, destroy_key, destroy_value);
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

static struct pair *get_pair_from_bucket(
                const struct bucket *bucket, const void *key, type_comp_cb comp)
{
        struct holder *holder = bucket->head;

        while (holder) {
                if (comp(holder->pair.key, key) == 0)
                        return &(holder->pair);

                holder = holder->next;
        }

        return NULL;
}

static void add_holder_to_bucket_list(
                struct bucket *list, size_t count, struct holder *holder)
{
        unsigned long i = holder->hash % count;
        struct bucket *bucket = &(list[i]);

        holder->next = bucket->head;
        bucket->head = holder;
}

/* Map API ---------------------------*/

static void destroy_map_bucket_list(const struct map *map)
{
        for (unsigned int i = 0; i < map->bucket_count; ++i) {
                destroy_holder_list(
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
                struct holder *holder = map->bucket_list[i].head;

                while (holder) {
                        struct holder *next = holder->next;
                        add_holder_to_bucket_list(new_list, new_count, holder);
                        holder = next;
                }
        }

        free(map->bucket_list);
        map->bucket_list = new_list;
        map->bucket_count = new_count;

        return 0;
}

static struct pair *get_pair_from_map(const struct map *map, const void *key)
{
        const struct bucket *bucket = get_bucket_from_key(map, key);
        return get_pair_from_bucket(bucket, key, map->key_type->comp);
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

        if (get_pair_from_map(map, key))
                return -EEXIST;

        struct holder *holder = create_holder(
                        key, value, map->key_type, map->value_type);
        if (!holder)
                return -ENOMEM;

        if (map->count == map->bucket_count) {
                if (resize_map_bucket_list(map) < 0) {
                        destroy_holder(holder, map->key_type->destroy,
                                        map->value_type->destroy);
                        return -ENOMEM;
                }
        }

        add_holder_to_bucket_list(map->bucket_list, map->bucket_count, holder);
        ++map->count;

        return 0;
}

void *map_value(const struct map *map, const void *key)
{
        if (!map || !key)
                return NULL;

        const struct pair *pair = get_pair_from_map(map, key);
        return (pair ? pair->value : NULL);
}

struct pair *map_pair(const struct map *map, const void *key)
{
        if (!map || !key)
                return NULL;

        return get_pair_from_map(map, key);
}

int map_remove(struct map *map, const void *key)
{
        if (!map || !key)
                return -EINVAL;

        struct bucket *bucket = get_bucket_from_key(map, key);
        struct bucket *previous = NULL;

        while (bucket->head) {
                if (map->key_type->comp(bucket->head->pair.key, key) != 0) {
                        previous->head = bucket->head;
                        bucket->head = bucket->head->next;
                        continue;
                }

                /* Match found */
                struct holder *next = bucket->head->next;
                destroy_holder(bucket->head, map->key_type->destroy,
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

/* Iterator API --------------------------------------------------------------*/

/* Iterator implementation -----------*/

struct map_it {
        struct iterator it; /* Placed at top for inheritance */
        const struct map *map;
        unsigned int bucket_pos;
        struct holder *holder;
};

static void map_it_seek_next(struct map_it *m_it)
{
        do {
                if (m_it->holder) {
                        m_it->holder = m_it->holder->next;
                } else {
                        const unsigned int new_pos = ++m_it->bucket_pos;
                        if (new_pos >= m_it->map->bucket_count)
                                break;

                        m_it->holder = m_it->map->bucket_list[new_pos].head;
                }
        } while (!m_it->holder);
}

static int map_it_next(struct iterator *it)
{
        struct map_it *m_it = (struct map_it *)it;
        map_it_seek_next(m_it);
        return 0;
}

static int map_it_previous(struct iterator *it)
{
        return -EOPNOTSUPP;
}

static bool map_it_is_valid(const struct iterator *it)
{
        const struct map_it *m_it = (const struct map_it *)it;
        return (m_it->bucket_pos < m_it->map->bucket_count && m_it->holder);
}

static void *map_it_data(const struct iterator *it)
{
        if (!map_it_is_valid(it))
                return NULL;

        const struct map_it *m_it = (const struct map_it *)it;
        return m_it->holder->pair.value;
}

static void map_it_destroy(struct iterator *it)
{
        struct map_it *m_it = (struct map_it *)it;
        free(m_it);
}

static struct iterator_callbacks map_it_cbs = {
        .next_cb = map_it_next,
        .previous_cb = map_it_previous,
        .is_valid_cb = map_it_is_valid,
        .data_cb = map_it_data,
        .destroy_cb = map_it_destroy
};

/* Static functions ------------------*/

static struct map_it *map_it_create(const struct map *map)
{
        struct map_it *m_it = malloc(sizeof(*m_it));
        if (!m_it)
                return NULL;

        it_init(&m_it->it, &map_it_cbs);
        m_it->map = map;

        return m_it;
}

/* Public API ------------------------*/

struct iterator *map_begin(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(map);
        if (!m_it)
                return NULL;

        m_it->bucket_pos = 0;
        m_it->holder = map->bucket_list[0].head;
        if (!m_it->holder)
                map_it_seek_next(m_it);

        return (struct iterator *)m_it;
}

struct pair *map_pair_from_it(const struct iterator *it)
{
        if (!it || !map_it_is_valid(it))
                return NULL;

        const struct map_it *m_it = (const struct map_it *)it;
        return &m_it->holder->pair;
}
