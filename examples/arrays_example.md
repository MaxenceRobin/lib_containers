# Lib_arrays example

Code :
```C
#include "lib_arrays.h"
#include "lib_container_algos.h"
#include "lib_types.h"

#include <stddef.h>
#include <stdio.h>

/* Static functions ----------------------------------------------------------*/

static void print_array(struct array *array)
{
        ssize_t len = array_len(array);
        printf("\nlen = %ld, data = [", len);

        const int *data = array_data(array);

        for (unsigned int i = 0; i < len; ++i) {
                if (i > 0)
                        printf(", ");

                printf("%d", data[i]);
        }

        printf("]\n");
}

static int comp_reverse(const void *a, const void *b)
{
        const int *first = a;
        const int *second = b;

        return (*second - *first);
}

static void increase_by(void *data, void *arg)
{
        int *value = data;
        int *increment = arg;

        *value += *increment;
}

/* Main ----------------------------------------------------------------------*/

int main()
{
        int list[] =  { 5, 6, 7, 8, 9, 0, 1, 2, 3, 4 };
        struct array *array = array_create(type_int(), 10, list);
        print_array(array);

        array_sort(array);
        print_array(array);

        array_sort_by(array, comp_reverse);
        print_array(array);

        ctn_for_each(array_begin(array), increase_by, &(int) { 100 });
        print_array(array);

        int *value = array_value(array, 6);
        printf("\nlist[6] = %d\n", *value);

        *value = 0;
        print_array(array);

        array_destroy(array);
        return 0;
}
```

Output :
```
len = 10, data = [5, 6, 7, 8, 9, 0, 1, 2, 3, 4]

len = 10, data = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

len = 10, data = [9, 8, 7, 6, 5, 4, 3, 2, 1, 0]

len = 10, data = [109, 108, 107, 106, 105, 104, 103, 102, 101, 100]

list[6] = 103

len = 10, data = [109, 108, 107, 106, 105, 104, 0, 102, 101, 100]
```
