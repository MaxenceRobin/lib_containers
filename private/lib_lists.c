/**
 * @author Maxence ROBIN
 * @brief Provides simple doubly-linked list manipulation
 */

/* Includes ------------------------------------------------------------------*/

#include "lib_iterators_private.h"
#include "lib_lists.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/* Definitions ---------------------------------------------------------------*/

struct node {
        struct list *head;
        void *data;
        struct node *next;
        struct node *previous;
};

struct list {
        const struct type_info *type;
        size_t len;
        struct node node; /* Useful for functions simplifications */
};

struct list_it {
        struct iterator it; /* Placed at top for inheritance */
        const struct list *list;
        struct node *node;
};

/* Static functions ----------------------------------------------------------*/

/* Private utility functions ---------*/

/**
 * @brief Inserts an empty node of 'list' before 'next' containing 'value'.
 *
 * @return Pointer to the created node on success.
 * @return NULL on failure.
 */
static struct node *insert_node(
                struct list *list, struct node *next, const void *value)
{
        struct node *node = calloc(1, sizeof(*node));
        if (!node)
                return NULL;

        node->data = calloc(1, list->type->size);
        if (!node->data) {
                free(node);
                return NULL;
        }

        node->head = list;

        node->next = next;
        node->previous = next->previous;

        node->next->previous = node;
        node->previous->next = node;

        list->type->copy(node->data, value);
        ++list->len;

        return node;
}

static void destroy_node(const struct node *node)
{
        node->head->type->destroy(node->data);
        free(node->data);
        free((struct node *)node);
}

static void remove_node(const struct node *node)
{
        if (&node->head->node == node)
                return;

        --node->head->len;
        node->next->previous = node->previous;
        node->previous->next = node->next;
        destroy_node(node);
}

static void clear_list(struct list *list)
{
        struct node *next = list->node.next;
        for (struct node *node = next; node != &list->node; node = next) {
                next = node->next;
                destroy_node(node);
        }

        list->len = 0;
        list->node.next = &list->node;
        list->node.previous = &list->node;
}

/* API -----------------------------------------------------------------------*/

struct list *list_create(const struct type_info *type)
{
        if (!type)
                return NULL;

        if (type->size == 0 || !type->copy || !type->destroy)
                return NULL;

        struct list *list = calloc(1, sizeof(*list));
        if (!list)
                return NULL;

        list->type = type;
        list->len = 0;
        list->node.head = list;
        list->node.next = &list->node;
        list->node.previous = &list->node;

        return list;
}

void list_destroy(const struct list *list)
{
        if (!list)
                return;

        clear_list((struct list *)list);
        free((struct list *)list);
}

struct node *list_push_front(struct list *list, const void *value)
{
        if (!list || !value)
                return NULL;

        return insert_node(list, list->node.next, value);
}

struct node *list_push_back(struct list *list, const void *value)
{
        if (!list || !value)
                return NULL;

        return insert_node(list, &list->node, value);
}

struct node *list_insert(
                struct list *list, struct node *node, const void *value)
{
        if (!list || !node || !value || node->head != list)
                return NULL;

        return insert_node(list, node, value);
}

int list_pop_front(struct list *list)
{
        if (!list)
                return -EINVAL;

        remove_node(list->node.next);
        return 0;
}

int list_pop_back(struct list *list)
{
        if (!list)
                return -EINVAL;

        remove_node(list->node.previous);
        return 0;
}

int list_remove(struct list *list, const struct node *node)
{
        if (!list || !node)
                return -EINVAL;

        remove_node(node);
        return 0;
}

int list_clear(struct list *list)
{
        if (!list)
                return -EINVAL;

        clear_list(list);
        return 0;
}

ssize_t list_len(const struct list *list)
{
        if (!list)
                return -EINVAL;

        return (ssize_t)list->len;
}

struct node *list_first(const struct list *list)
{
        if (!list)
                return NULL;

        return list->node.next;
}

struct node *list_last(const struct list *list)
{
        if (!list)
                return NULL;

        return list->node.previous;
}

struct node *list_node(const struct list *list, unsigned int pos)
{
        if (!list || pos >= list->len)
                return NULL;

        struct node *node = list->node.next;
        for (unsigned int i = 0; i < pos; ++i)
                node = node->next;

        return node;
}

struct node *node_next(const struct node *node)
{
        if (!node)
                return NULL;

        return node->next;
}

struct node *node_previous(const struct node *node)
{
        if (!node)
                return NULL;

        return node->previous;
}

bool node_is_valid(const struct node *node)
{
        return (node && node != &node->head->node);
}

void *node_data(struct node *node)
{
        if (!node || node == &node->head->node)
                return NULL;

        return node->data;
}

/* Iterator API --------------------------------------------------------------*/

static struct iterator_callbacks list_it_cbs;
static struct iterator_callbacks list_rit_cbs;

/* Utiilty functions -----------------*/

static struct list_it *list_it_create(
                const struct list *list,
                struct node *node,
                const struct iterator_callbacks *cbs)
{
        struct list_it *l_it = calloc(1, sizeof(*l_it));
        if (!l_it)
                return NULL;

        it_init(&l_it->it, &list_it_cbs);
        l_it->list = list;
        l_it->node = node;
        l_it->it.cbs = cbs;

        return l_it;
}

/* Iterator implementation -----------*/

static int list_it_next(struct iterator *it)
{
        struct list_it *l_it = (struct list_it *)it;
        l_it->node = l_it->node->next;
        return 0;
}

static int list_it_previous(struct iterator *it)
{
        struct list_it *l_it = (struct list_it *)it;
        l_it->node = l_it->node->previous;
        return 0;
}

static bool list_it_is_valid(const struct iterator *it)
{
        const struct list_it *l_it = (const struct list_it *)it;
        return (l_it->node != &l_it->list->node);
}

static void *list_it_data(const struct iterator *it)
{
        if (!list_it_is_valid(it))
                return NULL;

        const struct list_it *l_it = (const struct list_it *)it;
        return l_it->node->data;
}

static const struct type_info *list_it_type(const struct iterator *it)
{
        const struct list_it *l_it = (const struct list_it *)it;
        return l_it->list->type;
}

static int list_it_remove(struct iterator *it)
{
        if (!list_it_is_valid(it))
                return -EINVAL;

        struct list_it *l_it = (struct list_it *)it;
        struct node *next = l_it->node->next;

        remove_node(l_it->node);
        l_it->node = next;

        return 0;
}

static int list_rit_remove(struct iterator *it)
{
        if (!list_it_is_valid(it))
                return -EINVAL;

        struct list_it *l_it = (struct list_it *)it;
        struct node *next = l_it->node->previous;

        remove_node(l_it->node);
        l_it->node = next;

        return 0;
}

static struct iterator *list_it_dup(const struct iterator *it)
{
        const struct list_it *l_it = (const struct list_it *)it;
        if (!list_it_is_valid(it))
                return NULL;

        struct list_it *dup =
                        list_it_create(l_it->list, l_it->node, l_it->it.cbs);
        return (struct iterator *)dup;
}

static int list_it_copy(struct iterator *dest, const struct iterator *src)
{
        struct list_it *l_dest = (struct list_it *)dest;
        const struct list_it *l_src = (const struct list_it *)src;

        if (l_dest->list != l_src->list)
                return -EINVAL;

        l_dest->node = l_src->node;
        return 0;
}

static void list_it_destroy(const struct iterator *it)
{
        struct list_it *l_it = (struct list_it *)it;
        free(l_it);
}

static struct iterator_callbacks list_it_cbs = {
        .next_cb = list_it_next,
        .previous_cb = list_it_previous,
        .is_valid_cb = list_it_is_valid,
        .data_cb = list_it_data,
        .type_cb = list_it_type,
        .remove_cb = list_it_remove,
        .dup_cb = list_it_dup,
        .copy_cb = list_it_copy,
        .destroy_cb = list_it_destroy
};

static struct iterator_callbacks list_rit_cbs = {
        .next_cb = list_it_previous,
        .previous_cb = list_it_next,
        .is_valid_cb = list_it_is_valid,
        .data_cb = list_it_data,
        .type_cb = list_it_type,
        .remove_cb = list_rit_remove,
        .dup_cb = list_it_dup,
        .copy_cb = list_it_copy,
        .destroy_cb = list_it_destroy
};

/* Public API ------------------------*/

struct iterator *list_begin(const struct list *list)
{
        if (!list)
                return NULL;

        struct list_it *l_it =
                        list_it_create(list, list->node.next, &list_it_cbs);
        if (!l_it)
                return NULL;

        return (struct iterator *)l_it;
}

struct iterator *list_end(const struct list *list)
{
        if (!list)
                return NULL;

        struct list_it *l_it =
                        list_it_create(list, list->node.previous, &list_it_cbs);
        if (!l_it)
                return NULL;

        return (struct iterator *)l_it;
}

struct iterator *list_rbegin(const struct list *list)
{
        if (!list)
                return NULL;

        struct list_it *l_it = list_it_create(
                        list, list->node.previous, &list_rit_cbs);
        if (!l_it)
                return NULL;

        return (struct iterator *)l_it;
}

struct iterator *list_rend(const struct list *list)
{
        if (!list)
                return NULL;

        struct list_it *l_it =
                        list_it_create(list, list->node.next, &list_rit_cbs);
        if (!l_it)
                return NULL;

        return (struct iterator *)l_it;
}
