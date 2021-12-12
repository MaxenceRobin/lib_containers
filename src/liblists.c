/**
 * @author Maxence ROBIN
 * @brief Provides simple doubly-linked list manipulation
 */

/* Includes ------------------------------------------------------------------*/

#include "libcontainers_private.h"
#include "libiterators_private.h"
#include "liblists.h"

#include <errno.h>
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
        struct container ctx; /* Placed at top for inheritance */
        size_t len;
        struct node node; /* Useful for functions simplifications */
};

struct list_it {
        struct iterator it; /* Placed at top for inheritance */
        struct node *node;
};

/* Static functions ----------------------------------------------------------*/

/* Private utility functions ---------*/

/**
 * @brief Inserts an empty node of 'list' after 'previous' containing 'value'.
 *
 * @return Pointer to the created node on success.
 * @return NULL on failure.
 */
static struct node *insert_node(
                struct list *list, struct node *previous, const void *value)
{
        struct node *node = malloc(sizeof(*node));
        if (!node)
                return NULL;

        node->data = malloc(list->ctx.type->size);
        if (!node->data) {
                free(node);
                return NULL;
        }

        node->head = list;

        node->next = previous->next;
        node->previous = previous;

        node->next->previous = node;
        node->previous->next = node;

        list->ctx.type->copy(node->data, value, list->ctx.type->size);
        ++list->len;

        return node;
}

static void destroy_node(const struct node *node)
{
        node->head->ctx.type->destroy(node->data);
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

static bool it_is_valid(const struct list *list, const struct list_it *it)
{
        return (it->node->head == list && it->node != &list->node);
}

/* Iterator implementation -----------*/

static struct list_it *list_it_create(); /* Forward declaration */

static struct iterator *list_it_copy(const struct iterator *it)
{
        struct list_it *copy = list_it_create();
        if (!copy)
                return NULL;

        copy->node = ((struct list_it *)it)->node;
        return (struct iterator *)copy;
}

static void list_it_destroy(const struct iterator *it)
{
        free((struct list_it *)it);
}

static bool list_it_is_valid(const struct iterator *it)
{
        const struct list_it *l_it = (const struct list_it *)it;
        return it_is_valid(l_it->node->head, l_it);
}

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

static const void *list_it_data(const struct iterator *it)
{
        const struct list_it *l_it = (const struct list_it *)it;

        if (!it_is_valid(l_it->node->head, l_it))
                return NULL;

        return l_it->node->data;
}

static int list_it_set_data(struct iterator *it, const void *data)
{
        if (!list_it_is_valid(it))
                return -EINVAL;

        struct list_it *l_it = (struct list_it *)it;
        struct container *ctx = &l_it->node->head->ctx;

        ctx->type->copy(l_it->node->data, data, ctx->type->size);
        return 0;
}

static struct iterator_callbacks iterator_cbs = {
        .copy = list_it_copy,
        .destroy = list_it_destroy,
        .is_valid = list_it_is_valid,
        .next = list_it_next,
        .previous = list_it_previous,
        .data = list_it_data,
        .set_data = list_it_set_data
};

static struct list_it *list_it_create()
{
        struct list_it *it = malloc(sizeof(*it));
        if (!it)
                return NULL;

        it->it.cbs = &iterator_cbs;
        return it;
}

/* Container implementation ----------*/

static struct iterator *list_container_first(const struct container *ctx)
{
        struct list_it *it = list_it_create();
        if (!it)
                return NULL;

        it->node = ((const struct list *)ctx)->node.next;
        return (struct iterator *)it;
}

static struct iterator *list_container_last(const struct container *ctx)
{
        struct list_it *it = list_it_create();
        if (!it)
                return NULL;

        it->node = ((const struct list *)ctx)->node.previous;
        return (struct iterator *)it;
}

static int list_container_insert(
                struct container *ctx, struct iterator *it, const void *data)
{
        struct list *list = (struct list *)ctx;
        struct list_it *l_it = (struct list_it *)it;

        if (!it_is_valid(list, l_it))
                return -EINVAL;

        struct node *node = insert_node(list, l_it->node->previous, data);
        if (!node)
                return -ENOMEM;

        return 0;
}

static int list_container_remove(
                struct container *ctx, const struct iterator *it)
{
        const struct list *list = (const struct list *)ctx;
        struct list_it *l_it = (struct list_it *)it;

        if (!it_is_valid(list, l_it))
                return -EINVAL;

        struct node *next = l_it->node->next;
        remove_node(l_it->node);
        l_it->node = next;

        return 0;
}

static struct container_callbacks container_cbs = {
        .first = list_container_first,
        .last = list_container_last,
        .insert = list_container_insert,
        .remove = list_container_remove
};

/* API -----------------------------------------------------------------------*/

struct list *list_create(const struct type_info *type)
{
        if (!type)
                return NULL;

        struct list *list = malloc(sizeof(*list));
        if (!list)
                return NULL;

        list->ctx.cbs = &container_cbs;
        list->ctx.type = type;
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

        struct node *copy = list->node.next;
        for (struct node *node = copy; node != &list->node; node = copy) {
                copy = node->next;
                destroy_node(node);
        }

        free((struct list *)list);
}

int list_push_front(struct list *list, const void *value)
{
        if (!list || !value)
                return -EINVAL;

        struct node *node = insert_node(list, &list->node, value);
        if (!node)
                return -ENOMEM;

        return 0;
}

int list_push_back(struct list *list, const void *value)
{
        if (!list || !value)
                return -EINVAL;

        struct node *node = insert_node(list, list->node.previous, value);
        if (!node)
                return -ENOMEM;

        return 0;
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

struct container *list_container(struct list *list)
{
        if (!list)
                return NULL;

        return &list->ctx;
}

ssize_t list_len(const struct list *list)
{
        if (!list)
                return -EINVAL;

        return (ssize_t)list->len;
}
