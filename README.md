# LIBCONTAINERS

Simple C lib for containers and data structures manipulation.

This lib provides multiple data structures and containers as well as utility functions to manipulate them.

## Example

```C
#include "libcontainers.h"
#include "libvectors.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void print_error(const char *msg, int error_code)
{
        fprintf(stderr, "%s : %s\n", msg, strerror(error_code));
}

static void print_vector(const int *vector)
{
        ssize_t len = vector_len(vector);

        printf("\nlen = %ld\ncapacity = %ld\n", len, vector_capacity(vector));

        printf("[ ");
        for (unsigned int i = 0; i < len; ++i) {
                if (i > 0)
                        printf(", ");

                printf("%d", vector[i]);
        }
        printf(" ]\n");
}

static int increase(void *data, void *ctx)
{
        int *value = data;
        int *seq = ctx;

        *value = (*seq)++;
        return 0;
}

static int reset(void *data, void *ctx)
{
        int *value = data;

        *value = 0;
        return 0;
}

int main()
{
        int res;

        int *vector = vector_create(sizeof(*vector), 5);
        print_vector(vector);

        for (int i = 0; i < vector_len(vector); ++i)
                vector[i] = i;
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector = vector_push(vector, &i, NULL);
        print_vector(vector);

        for (int i = 0; i < 3; ++i)
                vector_remove(vector, 0);
        print_vector(vector);

        vector = vector_reserve(vector, 100, NULL);
        print_vector(vector);

        for (int i = 0; i < 10; ++i)
                vector = vector_insert(vector, 3, &(int){i + 100}, NULL);
        print_vector(vector);

        for (int i = 0; i < 5; ++i)
                vector_pop(vector);
        print_vector(vector);

        vector = vector_fit(vector, NULL);
        print_vector(vector);

        const struct container *ctx = vector_container(vector);
        container_for_each_r(ctx, increase, &(int){0});
        print_vector(vector);

        container_for_each(ctx, reset, NULL);
        print_vector(vector);

        vector_destroy(vector);
        return 0;
}
```

Expected output :
```
len = 5
capacity = 5
[ 0, 0, 0, 0, 0 ]

len = 5
capacity = 5
[ 0, 1, 2, 3, 4 ]

len = 10
capacity = 12
[ 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 ]

len = 7
capacity = 12
[ 3, 4, 0, 1, 2, 3, 4 ]

len = 7
capacity = 100
[ 3, 4, 0, 1, 2, 3, 4 ]

len = 17
capacity = 100
[ 3, 4, 0, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 1, 2, 3, 0 ]

len = 12
capacity = 100
[ 3, 4, 0, 109, 108, 107, 106, 105, 104, 103, 102, 101 ]

len = 12
capacity = 12
[ 3, 4, 0, 109, 108, 107, 106, 105, 104, 103, 102, 101 ]

len = 12
capacity = 12
[ 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 ]

len = 12
capacity = 12
[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
```
