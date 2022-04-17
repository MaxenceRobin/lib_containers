/**
 * @author Maxence ROBIN
 * @brief Provides simple doubly-linked list manipulation
 */

/* Includes ------------------------------------------------------------------*/

#include "liblists.h"
#include "libtypes.h"

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

        node->data = malloc(list->type->size);
        if (!node->data) {
                free(node);
                return NULL;
        }

        node->head = list;

        node->next = previous->next;
        node->previous = previous;

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

/* API -----------------------------------------------------------------------*/

struct list *list_create(const struct type_info *type)
{
        if (!type)
                return NULL;

        if (type->size == 0 || !type->copy || !type->destroy)
                return NULL;

        struct list *list = malloc(sizeof(*list));
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

ssize_t list_len(const struct list *list)
{
        if (!list)
                return -EINVAL;

        return (ssize_t)list->len;
}
