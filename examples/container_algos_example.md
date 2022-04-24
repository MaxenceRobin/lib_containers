# Libcontainers_algos example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "libcontainer_algos.h"
#include "liblists.h"
#include "libtypes.h"
#include "libvectors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_container(const char *name, const struct iterator *from)
{
        struct iterator *it = it_dup(from); /* Avoid modifying given it */

        bool first = true;
        printf("\n%s = {", name);

        while (it_is_valid(it)) {
                const int *data = it_data(it);

                if (!first)
                        printf(", ");
                printf("%d", *data);

                first = false;
                it_next(it);
        }

        printf("}\n");

        it_unref(from);
        it_unref(it);
}

static void modulo(void *data, void *arg)
{
        int *value = data;
        int *mod = arg;

        *value %= *mod;
}

static bool is_lower_than(const void *data, void *arg)
{
        const int *value = data;
        int *limit = arg;

        return (*value < *limit);
}

/* Main ----------------------------------------------------------------------*/

void main()
{
        int *vector = vector_create(type_int(), 10);
        struct list *list = list_create(type_int());

        for (int i = 0; i < 10; ++i) {
                vector[i] = i + 10;
                list_push_back(list, INT(i + 100));
        }

        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));

        ctn_for_each(vector_begin(vector), modulo, INT(10));
        ctn_for_each(list_begin(list), modulo, INT(10));
        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));

        ctn_remove_if(vector_begin(vector), is_lower_than, INT(7));
        ctn_remove_if(list_begin(list), is_lower_than, INT(5));
        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));

        print_container("vector reverse", vector_rbegin(vector));
        print_container("list reverse", list_rbegin(list));

        vector_destroy(vector);
        list_destroy(list);
}
```

Output :
```
vector = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19}

list = {100, 101, 102, 103, 104, 105, 106, 107, 108, 109}

vector = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

list = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

vector = {7, 8, 9}

list = {5, 6, 7, 8, 9}

vector reverse = {9, 8, 7}

list reverse = {9, 8, 7, 6, 5}
```
