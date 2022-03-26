/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

/* Definitions ---------------------------------------------------------------*/

#define DECL_TYPE(name, type) \
\
static void *copy_##name(void *dest, const void *src) \
{ \
        *((type *)dest) = *((type *)src); \
} \
\
static int comp_##name(const void *first, const void *second) \
{ \
        return *((type *)first) - *((type *)second); \
} \
\
static struct type_info info_##name = { \
        .size = sizeof(type), \
        .copy = copy_##name, \
        .comp = comp_##name, \
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

type_destroy_cb type_default_destroy()
{
        return default_destroy;
}
