/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DECL_TYPE(name, type) \
static struct type_info info_##name = { \
        .size = sizeof(type), \
        .copy = memcpy, \
        .comp = memcmp, \
        .destroy = default_destroy \
}; \
\
const struct type_info *type_##name() \
{ \
        return &info_##name; \
}

/* Static functions ----------------------------------------------------------*/

/**
 * @brief The default destruction behavior is to do nothing
 */
static void default_destroy(void *data)
{
}

/* API -----------------------------------------------------------------------*/

DECL_TYPE(char, char)
DECL_TYPE(uchar, unsigned char)

DECL_TYPE(short, short)
DECL_TYPE(ushort, unsigned short)

DECL_TYPE(int, int)
DECL_TYPE(uint, unsigned int)

DECL_TYPE(long, long)
DECL_TYPE(ulong, unsigned long)

DECL_TYPE(long_long, long long)
DECL_TYPE(ulong_long, unsigned long long)

DECL_TYPE(float, float)
DECL_TYPE(double, double)
DECL_TYPE(long_double, long double)

void *(*type_default_copy())(void *, const void *, size_t)
{
        return memcpy;
}

int (*type_default_comp())(const void *, const void *, size_t)
{
        return memcmp;
}

void (*type_default_destroy())(void *)
{
        return default_destroy;
}
