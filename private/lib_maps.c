/**
 * @author Maxence ROBIN
 * @brief Provides simple string maps manipulation.
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_iterators_private.h"
#include "lib_maps.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DEFAULT_BUCKET_LIST_COUNT 16

struct m_pair {
        void *key;
        void *value;
};

struct node {
        struct m_pair pair;
        unsigned long hash;
        struct node *next;
        struct node *previous;
};

struct map {
        const struct type_info *key_type;
        const struct type_info *value_type;
        unsigned int count;
        struct node *bucket_list;
        size_t bucket_count;
};

struct map_it {
        struct iterator it; /* Placed at top for inheritance */
        struct map *map;
        int bucket_pos;
        struct node *node;
};

/* Static functions ----------------------------------------------------------*/

/* Node API --------------------------*/

static struct node *create_node(
                const void *key,
                const void *value,
                const struct type_info *key_type,
                const struct type_info *value_type)
{
        struct node *node = calloc(1, sizeof(*node));
        if (!node)
                goto error_alloc_node;

        node->pair.key = calloc(1, key_type->size);
        if (!node->pair.key)
                goto error_alloc_key;

        key_type->copy(node->pair.key, key);
        node->pair.value = calloc(1, value_type->size);
        if (!node->pair.value)
                goto error_alloc_value;

        value_type->copy(node->pair.value, value);
        node->hash = key_type->hash(key);

        return node;

error_alloc_value:
        key_type->destroy(node->pair.key);
        free(node->pair.key);
error_alloc_key:
        free(node);
error_alloc_node:
        return NULL;
}

static void destroy_node(
                struct node *node,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        destroy_key(node->pair.key);
        free(node->pair.key);
        destroy_value(node->pair.value);
        free(node->pair.value);
        free(node);
}

/* Bucket API ------------------------*/

static struct node *create_bucket_list(size_t count)
{
        struct node *bucket_list;
        const size_t bucket_list_size = sizeof(*bucket_list) * count;

        bucket_list = calloc(1, bucket_list_size);
        if (!bucket_list)
                return NULL;

        for (unsigned int i = 0; i < count; ++i) {
                bucket_list[i].next = &bucket_list[i];
                bucket_list[i].previous = &bucket_list[i];
        }

        return bucket_list;
}

static void destroy_bucket(
                struct node *bucket,
                type_destroy_cb destroy_key,
                type_destroy_cb destroy_value)
{
        struct node *node = bucket->next;

        while (node != bucket) {
                struct node *next = node->next;
                destroy_node(node, destroy_key, destroy_value);
                node = next;
        }
}

static struct node *get_node_from_bucket(
                const struct node *bucket, const void *key, type_comp_cb comp)
{
        struct node *node = bucket->next;

        while (node != bucket) {
                if (comp(node->pair.key, key) == 0)
                        return node;

                node = node->next;
        }

        return NULL;
}

static void add_node_to_bucket_list(
                struct node *list, size_t count, struct node *node)
{
        unsigned long i = node->hash % count;
        struct node *bucket = &list[i];

        node->next = bucket;
        node->previous = bucket->previous;

        bucket->previous->next = node;
        bucket->previous = node;
}

/* Map API ---------------------------*/

static void destroy_map_bucket_list(const struct map *map)
{
        for (unsigned int i = 0; i < map->bucket_count; ++i) {
                destroy_bucket(&map->bucket_list[i],
                                map->key_type->destroy,
                                map->value_type->destroy);
        }

        free(map->bucket_list);
}

static int resize_map_bucket_list(struct map *map)
{
        const size_t new_count = map->bucket_count * 2;
        struct node *new_list = create_bucket_list(new_count);
        if (!new_list)
                return -ENOMEM;

        /* Move nodes from old list to new one */
        for (unsigned int i = 0; i < map->bucket_count; ++i) {
                const struct node *bucket = &map->bucket_list[i];
                struct node *node = bucket->next;

                while (node != bucket) {
                        struct node *next = node->next;
                        add_node_to_bucket_list(new_list, new_count, node);
                        node = next;
                }
        }

        free(map->bucket_list);
        map->bucket_list = new_list;
        map->bucket_count = new_count;

        return 0;
}

static void remove_node_from_map(struct map *map, struct node *node)
{
        node->previous->next = node->next;
        node->next->previous = node->previous;

        destroy_node(node, map->key_type->destroy, map->value_type->destroy);
        --map->count;
}

static struct node *get_node_from_map(const struct map *map, const void *key)
{
        const unsigned long i = map->key_type->hash(key) % map->bucket_count;
        const struct node *bucket = &map->bucket_list[i];

        return get_node_from_bucket(bucket, key, map->key_type->comp);
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

        struct map *map = calloc(1, sizeof(*map));
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

        if (get_node_from_map(map, key))
                return -EEXIST;

        struct node *node = create_node(
                        key, value, map->key_type, map->value_type);
        if (!node)
                return -ENOMEM;

        if (map->count >= map->bucket_count * 3 / 4) {
                if (resize_map_bucket_list(map) < 0) {
                        destroy_node(node, map->key_type->destroy,
                                        map->value_type->destroy);
                        return -ENOMEM;
                }
        }

        add_node_to_bucket_list(map->bucket_list, map->bucket_count, node);
        ++map->count;

        return 0;
}

void *map_value(const struct map *map, const void *key)
{
        if (!map || !key)
                return NULL;

        struct node *node = get_node_from_map(map, key);
        return (node ? node->pair.value : NULL);
}

struct pair *map_pair(const struct map *map, const void *key)
{
        if (!map || !key)
                return NULL;

        struct node *node = get_node_from_map(map, key);
        return (struct pair *)(node ? &node->pair : NULL);
}

int map_remove(struct map *map, const void *key)
{
        if (!map || !key)
                return -EINVAL;

        struct node *node = get_node_from_map(map, key);
        if (!node)
                return -ENOENT;

        remove_node_from_map(map, node);
        return 0;
}

int map_clear(struct map *map)
{
        if (!map)
                return -EINVAL;

        struct node *new_list = create_bucket_list(DEFAULT_BUCKET_LIST_COUNT);
        if (!new_list)
                return -ENOMEM;

        destroy_map_bucket_list(map);
        map->bucket_list = new_list;
        map->bucket_count = DEFAULT_BUCKET_LIST_COUNT;
        map->count = 0;

        return 0;
}

/* Iterator API --------------------------------------------------------------*/

static struct iterator_callbacks map_it_cbs;
static struct iterator_callbacks map_rit_cbs;

/* Utility function ------------------*/

static struct node *map_it_current_bucket(const struct map_it *m_it)
{
        return &m_it->map->bucket_list[m_it->bucket_pos];
}

static void map_it_seek_next(struct map_it *m_it)
{
        while (true) {
                m_it->node = m_it->node->next;
                if (m_it->node != map_it_current_bucket(m_it))
                        return;

                ++m_it->bucket_pos;
                if (m_it->bucket_pos >= m_it->map->bucket_count) {
                        m_it->node = NULL;
                        return;
                }

                m_it->node = map_it_current_bucket(m_it);
        }
}

static void map_it_seek_previous(struct map_it *m_it)
{
        while (true) {
                m_it->node = m_it->node->previous;
                if (m_it->node != map_it_current_bucket(m_it))
                        return;

                --m_it->bucket_pos;
                if (m_it->bucket_pos < 0) {
                        m_it->node = NULL;
                        return;
                }

                m_it->node = map_it_current_bucket(m_it);
        }
}

static struct map_it *map_it_create(
                const struct map *map,
                int bucket_pos,
                const struct iterator_callbacks *cbs)
{
        struct map_it *m_it = calloc(1, sizeof(*m_it));
        if (!m_it)
                return NULL;

        it_init(&m_it->it, &map_it_cbs);
        m_it->map = (struct map *)map;
        m_it->bucket_pos = bucket_pos;
        m_it->it.cbs = cbs;

        return m_it;
}

/* Iterator implementation -----------*/

static int map_it_next(struct iterator *it)
{
        struct map_it *m_it = (struct map_it *)it;
        if (!m_it->node)
                return -ERANGE;

        map_it_seek_next(m_it);
        return 0;
}

static int map_it_previous(struct iterator *it)
{
        struct map_it *m_it = (struct map_it *)it;
        if (!m_it->node)
                return -ERANGE;

        map_it_seek_previous(m_it);
        return 0;
}

static bool map_it_is_valid(const struct iterator *it)
{
        const struct map_it *m_it = (const struct map_it *)it;
        return (m_it->node != NULL);
}

static void *map_it_data(const struct iterator *it)
{
        if (!map_it_is_valid(it))
                return NULL;

        const struct map_it *m_it = (const struct map_it *)it;
        return m_it->node->pair.value;
}

static void *map_it_data_pair(const struct iterator *it)
{
        if (!map_it_is_valid(it))
                return NULL;

        const struct map_it *m_it = (const struct map_it *)it;
        return &m_it->node->pair;
}

static const struct type_info *map_id_type(const struct iterator *it)
{
        const struct map_it *m_it = (const struct map_it *)it;
        return m_it->map->value_type;
}

static int map_it_remove(struct iterator *it)
{
        if (!map_it_is_valid(it))
                return -EINVAL;

        struct map_it *m_it = (struct map_it *)it;
        struct node *node = m_it->node;
        map_it_seek_next(m_it);
        remove_node_from_map(m_it->map, node);

        return 0;
}

static int map_rit_remove(struct iterator *it)
{
        if (!map_it_is_valid(it))
                return -EINVAL;

        struct map_it *m_it = (struct map_it *)it;
        struct node *node = m_it->node;
        map_it_seek_previous(m_it);
        remove_node_from_map(m_it->map, node);

        return 0;
}

static struct iterator *map_it_dup(const struct iterator *it)
{
        if (!map_it_is_valid(it))
                return NULL;

        const struct map_it *m_it = (const struct map_it *)it;
        struct map_it *dup = map_it_create(
                        m_it->map, m_it->bucket_pos, m_it->it.cbs);
        if (!dup)
                return NULL;

        dup->node = m_it->node;
        return (struct iterator *)dup;
}

static int map_it_copy(struct iterator *dest, const struct iterator *src)
{
        if (!map_it_is_valid(dest) || !map_it_is_valid(src))
                return -EINVAL;

        struct map_it *m_dest = (struct map_it *)dest;
        const struct map_it *m_src = (const struct map_it *)src;

        if (m_dest->map != m_src->map)
                return -EINVAL;

        m_dest->bucket_pos = m_src->bucket_pos;
        m_dest->node = m_src->node;
        return 0;
}

static void map_it_destroy(const struct iterator *it)
{
        struct map_it *m_it = (struct map_it *)it;
        free(m_it);
}

static struct iterator_callbacks map_it_cbs = {
        .next_cb = map_it_next,
        .previous_cb = map_it_previous,
        .is_valid_cb = map_it_is_valid,
        .data_cb = map_it_data,
        .type_cb = map_id_type,
        .remove_cb = map_it_remove,
        .dup_cb = map_it_dup,
        .copy_cb = map_it_copy,
        .destroy_cb = map_it_destroy
};

static struct iterator_callbacks map_rit_cbs = {
        .next_cb = map_it_previous,
        .previous_cb = map_it_next,
        .is_valid_cb = map_it_is_valid,
        .data_cb = map_it_data,
        .type_cb = map_id_type,
        .remove_cb = map_rit_remove,
        .dup_cb = map_it_dup,
        .copy_cb = map_it_copy,
        .destroy_cb = map_it_destroy
};

static struct iterator_callbacks map_it_pair_cbs = {
        .next_cb = map_it_next,
        .previous_cb = map_it_previous,
        .is_valid_cb = map_it_is_valid,
        .data_cb = map_it_data_pair,
        .type_cb = map_id_type,
        .remove_cb = map_it_remove,
        .dup_cb = map_it_dup,
        .copy_cb = map_it_copy,
        .destroy_cb = map_it_destroy
};

static struct iterator_callbacks map_rit_pair_cbs = {
        .next_cb = map_it_previous,
        .previous_cb = map_it_next,
        .is_valid_cb = map_it_is_valid,
        .data_cb = map_it_data_pair,
        .type_cb = map_id_type,
        .remove_cb = map_rit_remove,
        .dup_cb = map_it_dup,
        .copy_cb = map_it_copy,
        .destroy_cb = map_it_destroy
};

/* Public API ------------------------*/

struct iterator *map_begin(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(map, 0, &map_it_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the first bucket */
        m_it->node = &map->bucket_list[0];
        map_it_seek_next(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_end(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(
                        map, map->bucket_count - 1, &map_it_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the last bucket */
        m_it->node = &map->bucket_list[map->bucket_count - 1];
        map_it_seek_previous(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_rbegin(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(
                        map, map->bucket_count - 1, &map_rit_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the last bucket */
        m_it->node = &map->bucket_list[map->bucket_count - 1];
        map_it_seek_previous(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_rend(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(map, 0, &map_rit_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the first bucket */
        m_it->node = &map->bucket_list[0];
        map_it_seek_next(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_begin_pair(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(map, 0, &map_it_pair_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the first bucket */
        m_it->node = &map->bucket_list[0];
        map_it_seek_next(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_end_pair(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(
                        map, map->bucket_count - 1, &map_it_pair_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the last bucket */
        m_it->node = &map->bucket_list[map->bucket_count - 1];
        map_it_seek_previous(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_rbegin_pair(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(
                        map, map->bucket_count - 1, &map_rit_pair_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the last bucket */
        m_it->node = &map->bucket_list[map->bucket_count - 1];
        map_it_seek_previous(m_it);

        return (struct iterator *)m_it;
}

struct iterator *map_rend_pair(const struct map *map)
{
        if (!map)
                return NULL;

        struct map_it *m_it = map_it_create(map, 0, &map_rit_pair_cbs);
        if (!m_it)
                return NULL;

        /* Looking for the first valid node starting from the first bucket */
        m_it->node = &map->bucket_list[0];
        map_it_seek_next(m_it);

        return (struct iterator *)m_it;
}
