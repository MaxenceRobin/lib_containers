# Libbuffers example

Code :
```C
/* Includes ------------------------------------------------------------------*/

#include "libbuffers.h"
#include "libtypes.h"

#include <stdio.h>

/* Definitions ---------------------------------------------------------------*/

#define INT(VALUE) (&(int) { VALUE })

/* Main ----------------------------------------------------------------------*/

void main()
{
        struct buffer *buffer = buffer_create(type_int(), 10);

        for (int i = 10; !buffer_is_full(buffer); ++i)
                buffer_push(buffer, &i);
        printf("Top value = %d\n", *((int *)buffer_data(buffer)));

        for (int i = 0; i < 3; ++i)
                buffer_f_push(buffer, INT(1 + 100));

        while (!buffer_is_empty(buffer)) {
                const int *value = buffer_data(buffer);
                printf("value = %d\n", *value);
                buffer_pop(buffer);
        }

        buffer_destroy(buffer);
}
```

Output :
```
Top value = 10
value = 13
value = 14
value = 15
value = 16
value = 17
value = 18
value = 19
value = 101
value = 101
value = 101
```
