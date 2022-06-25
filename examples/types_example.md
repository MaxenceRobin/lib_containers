# Lib_types example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "lib_types.h"
#include "lib_vectors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

struct person {
        const char *name;
        unsigned int age;
};

#define PERSON(NAME, AGE) (&(struct person) { .name = NAME, .age = AGE })

static int comp_person(const void *a, const void *b)
{
        const struct person *pa = a;
        const struct person *pb = b;

        const int res = strcmp(pa->name, pb->name);
        if (res != 0)
                return res;

        return (pa->age - pb->age);
}

static void copy_person(void *a, const void *b)
{
        struct person *pa = a;
        const struct person *pb = b;

        pa->age = pb->age;
        pa->name = pb->name;
}

static struct type_info info_person = {
        .size = sizeof(struct person),
        .copy = copy_person,
        .comp = comp_person,
        .hash = NULL, /* Not done here, needed to be used as key for maps */
        .destroy = type_default_destroy /* Does nothing */
};

/* Static functions ----------------------------------------------------------*/

static void print_vector(const struct person *vector)
{
        const ssize_t len = vector_len(vector);
        printf("\nvector = [\n");

        for (int i = 0; i < len; ++i) {
                printf("\t<name = '%s', age = %d>\n",
                                vector[i].name, vector[i].age);
        }

        printf("]\n");
}

/* Main ----------------------------------------------------------------------*/

void main()
{
        struct person *vector = vector_create(&info_person, 0);

        vector = vector_push(vector, PERSON("John", 42), NULL);
        vector = vector_push(vector, PERSON("Marie", 32), NULL);
        vector = vector_push(vector, PERSON("Alex", 34), NULL);
        vector = vector_push(vector, PERSON("Sophia", 38), NULL);
        vector = vector_push(vector, PERSON("Alice", 27), NULL);
        vector = vector_push(vector, PERSON("Peter", 36), NULL);
        print_vector(vector);

        vector_sort(vector);
        print_vector(vector);

        vector_remove(vector, 4);
        vector_remove(vector, 3);
        print_vector(vector);

        vector_destroy(vector);
}
```

Output :
```
vector = [
        <name = 'John', age = 42>
        <name = 'Marie', age = 32>
        <name = 'Alex', age = 34>
        <name = 'Sophia', age = 38>
        <name = 'Alice', age = 27>
        <name = 'Peter', age = 36>
]

vector = [
        <name = 'Alex', age = 34>
        <name = 'Alice', age = 27>
        <name = 'John', age = 42>
        <name = 'Marie', age = 32>
        <name = 'Peter', age = 36>
        <name = 'Sophia', age = 38>
]

vector = [
        <name = 'Alex', age = 34>
        <name = 'Alice', age = 27>
        <name = 'John', age = 42>
        <name = 'Sophia', age = 38>
]
```
