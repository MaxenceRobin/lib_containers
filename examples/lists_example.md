# Liblists example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "liblists.h"
#include "libtypes.h"

#include <stdio.h>

/* Definitions ---------------------------------------------------------------*/

#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_list(const struct list *list)
{
        bool first = true;
        struct iterator *it = list_begin(list);

        printf("\nlen = %ld\n", list_len(list));
        printf("{");

        while (it_is_valid(it)) {
                if (!first)
                        printf(" -> ");

                const int *value = it_data(it);
                printf("%d", *value);

                it_next(it);
                first = false;
        }

        printf("}\n");
        it_unref(it);
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

        struct iterator *it = list_begin(list);
        for (int i = 0; i < 5; ++i)
                it_next(it);

        for (int i = 0; i < 5; ++i)
                list_insert(list, it, INT(i + 1000));
        print_list(list);

        it_previous(it);
        for (int i = 0; i < 3; ++i)
                it_remove(it);
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
{14 -> 13 -> 12 -> 11 -> 10 -> 1004 -> 1003 -> 1002 -> 1001 -> 1000 -> 100 -> 101 -> 102 -> 103 -> 104}

len = 12
{14 -> 13 -> 12 -> 11 -> 1002 -> 1001 -> 1000 -> 100 -> 101 -> 102 -> 103 -> 104}
```
