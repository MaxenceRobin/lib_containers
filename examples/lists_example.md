# Lib_lists example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "lib_lists.h"
#include "lib_types.h"

#include <stdio.h>

/* Definitions ---------------------------------------------------------------*/

#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_list(const struct list *list)
{
        printf("\nlen = %ld\n", list_len(list));
        printf("{");

        bool first = true;

        struct node *node = list_first(list);
        while (node_is_valid(node)) {
                if (!first)
                        printf(" -> ");

                const int *value = node_data(node);
                printf("%d", *value);

                first = false;
                node = node_next(node);
        }

        printf("}\n");
}

/* Main ----------------------------------------------------------------------*/

void main()
{
        struct list *list = list_create(type_int());
        print_list(list);

        for (int i = 0; i < 5; ++i) {
                list_push_front(list, INT(i + 10));
                list_push_back(list, INT(i + 100));
        }

        print_list(list);

        struct node *node = list_node(list, 5);
        for (int i = 0; i < 5; ++i)
                list_insert(list, node, INT(i + 1000));

        print_list(list);

        for (int i = 0; i < 3; ++i)
                list_remove(list, list_first(list));

        print_list(list);

        for (int i = 0; i < 2; ++i) {
                list_pop_front(list);
                list_pop_back(list);
        }
        print_list(list);

        list_clear(list);
        print_list(list);

        list_destroy(list);
}
```

Output :
```
len = 0
{}

len = 10
{14 -> 13 -> 12 -> 11 -> 10 -> 100 -> 101 -> 102 -> 103 -> 104}

len = 15
{14 -> 13 -> 12 -> 11 -> 10 -> 1000 -> 1001 -> 1002 -> 1003 -> 1004 -> 100 -> 101 -> 102 -> 103 -> 104}

len = 12
{11 -> 10 -> 1000 -> 1001 -> 1002 -> 1003 -> 1004 -> 100 -> 101 -> 102 -> 103 -> 104}

len = 8
{1000 -> 1001 -> 1002 -> 1003 -> 1004 -> 100 -> 101 -> 102}

len = 0
{}
```
