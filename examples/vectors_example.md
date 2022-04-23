# Libvectors example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "libvectors.h"
#include "libtypes.h"

#include <stdio.h>

/* Definitions ---------------------------------------------------------------*/

#define INT(VALUE) (&(int) { VALUE })

/* Static functions ----------------------------------------------------------*/

static void print_vector(const int *vector)
{
        const ssize_t len = vector_len(vector);
        const ssize_t capacity = vector_capacity(vector);
        printf("\nlen = %ld, capacity = %ld\n", len, capacity);
        printf("[");

        for (int i = 0; i < len; ++i) {
                if (i > 0)
                        printf(", ");

                printf("%d", vector[i]);
        }

        printf("]\n");
}

/* Main ----------------------------------------------------------------------*/

void main()
{
        int *vector = vector_create(type_int(), 5);
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector[i] = 1;
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector = vector_push(vector, INT(i + 10), NULL);
        print_vector(vector);

        for (int i = 0; i < 5; i++)
                vector = vector_insert(vector, 0, INT(i + 100), NULL);
        print_vector(vector);

        vector_sort(vector);
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector_remove(vector, 0);
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector_pop(vector);
        print_vector(vector);

        vector = vector_fit(vector, NULL);
        print_vector(vector);

        vector_clear(vector);
        print_vector(vector);

        vector_destroy(vector);
}
```

Output :
```
len = 5, capacity = 5
[0, 0, 0, 0, 0]

len = 5, capacity = 5
[1, 1, 1, 1, 1]

len = 10, capacity = 12
[1, 1, 1, 1, 1, 10, 11, 12, 13, 14]

len = 15, capacity = 26
[104, 103, 102, 101, 100, 1, 1, 1, 1, 1, 10, 11, 12, 13, 14]

len = 15, capacity = 26
[1, 1, 1, 1, 1, 10, 11, 12, 13, 14, 100, 101, 102, 103, 104]

len = 10, capacity = 26
[10, 11, 12, 13, 14, 100, 101, 102, 103, 104]

len = 5, capacity = 26
[10, 11, 12, 13, 14]

len = 5, capacity = 5
[10, 11, 12, 13, 14]

len = 0, capacity = 5
[]
```
