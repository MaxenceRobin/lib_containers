# Lib_containers_algos example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "lib_container_algos.h"
#include "lib_lists.h"
#include "lib_maps.h"
#include "lib_types.h"
#include "lib_vectors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define STR(VALUE) (&(struct type_string) { .string = VALUE })
#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_container(const char *name, struct iterator *from)
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
        struct map *map = map_create(
                        type_string(TYPE_DESTROY_POLICY_AUTO_FREE), type_int());

        char key[8];
        for (int i = 0; i < 10; ++i) {
                vector[i] = i + 10;
                list_push_back(list, INT(i + 100));

                snprintf(key, sizeof(key), "key_%d", i);
                map_add(map, STR(strdup(key)), INT(i + 1000));
        }

        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));
        print_container("map", map_begin(map));

        ctn_for_each(vector_begin(vector), modulo, INT(10));
        ctn_for_each(list_begin(list), modulo, INT(10));
        ctn_for_each(map_begin(map), modulo, INT(10));
        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));
        print_container("map", map_begin(map));

        ctn_remove_if(vector_begin(vector), is_lower_than, INT(3));
        ctn_remove_if(list_begin(list), is_lower_than, INT(5));
        ctn_remove_if(map_begin(map), is_lower_than, INT(7));
        print_container("vector", vector_begin(vector));
        print_container("list", list_begin(list));
        print_container("map", map_begin(map));

        print_container("vector reverse", vector_rbegin(vector));
        print_container("list reverse", list_rbegin(list));
        print_container("map reverse", map_rbegin(map));

        vector_destroy(vector);
        list_destroy(list);
        map_destroy(map);
}
```

Output :
```
vector = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19}

list = {100, 101, 102, 103, 104, 105, 106, 107, 108, 109}

map = {1003, 1004, 1005, 1006, 1007, 1008, 1009, 1000, 1001, 1002}

vector = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

list = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

map = {3, 4, 5, 6, 7, 8, 9, 0, 1, 2}

vector = {3, 4, 5, 6, 7, 8, 9}

list = {5, 6, 7, 8, 9}

map = {7, 8, 9}

vector reverse = {9, 8, 7, 6, 5, 4, 3}

list reverse = {9, 8, 7, 6, 5}

map reverse = {9, 8, 7}
```
