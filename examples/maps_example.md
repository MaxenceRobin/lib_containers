# Lib_maps example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "lib_iterators.h"
#include "lib_maps.h"
#include "lib_types.h"

#include <stdio.h>

/* Definitions ---------------------------------------------------------------*/

#define STR(VALUE) (&(struct type_string) { .string = VALUE })
#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_pair(const struct pair *pair)
{
        const struct type_string *key = pair->key;
        const int *value = pair->value;

        printf("\t<'%s', %d>\n", key->string, *value);
}

static void print_map(const struct map *map)
{
        struct iterator *it = map_begin_pair(map);
        printf("\nmap = {\n");

        while (it_is_valid(it)) {
                const struct pair *pair = it_data(it);
                print_pair(pair);
                it_next(it);
        }

        printf("}\n");
        it_unref(it);
}

/* Main ----------------------------------------------------------------------*/

void main()
{
        struct map *map = map_create(
                        type_string(TYPE_DESTROY_POLICY_NO_FREE), type_int());

        map_add(map, STR("CPU"), INT(300));
        map_add(map, STR("GPU"), INT(1100));
        map_add(map, STR("RAM"), INT(180));
        map_add(map, STR("SSD"), INT(200));
        map_add(map, STR("HDD"), INT(100));
        map_add(map, STR("Motherboard"), INT(250));
        map_add(map, STR("Case"), INT(160));
        print_map(map);

        const int *ram_price = map_value(map, STR("RAM"));
        printf("\nPrice of the ram is %d€\n", *ram_price);

        const struct pair *hdd_pair = map_pair(map, STR("HDD"));
        printf("\nPair for the HDD is :\n");
        print_pair(hdd_pair);

        map_remove(map, STR("SSD"));
        map_remove(map, STR("Case"));
        print_map(map);

        map_clear(map);
        print_map(map);

        map_destroy(map);
}
```

Output :
```
map = {
        <'GPU', 1100>
        <'Case', 160>
        <'RAM', 180>
        <'HDD', 100>
        <'Motherboard', 250>
        <'CPU', 300>
        <'SSD', 200>
}

Price of the ram is 180€

Pair for the HDD is :
        <'HDD', 100>

map = {
        <'GPU', 1100>
        <'RAM', 180>
        <'HDD', 100>
        <'Motherboard', 250>
        <'CPU', 300>
}

map = {
}
```
