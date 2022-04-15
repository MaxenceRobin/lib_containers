/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

#define DECL_TYPE(name, type) \
\
static void copy_##name(void *dest, const void *src) \
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
        .destroy = type_default_destroy \
}; \
\
const struct type_info *type_##name() \
{ \
        return &info_##name; \
}

void type_default_destroy(void *data)
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

/* Special types -------------------------------------------------------------*/

/* Pointer ---------------------------*/

static void copy_pointer(void *dest, const void *src)
{
        struct type_pointer *a = dest;
        const struct type_pointer *b = src;

        a->pointer = b->pointer;
}

static int comp_pointer(const void *first, const void *second)
{
        const struct type_pointer *a = first;
        const struct type_pointer *b = second;

        return a->pointer - b->pointer;
}

static void destroy_pointer(void *data)
{
        struct type_pointer *value = data;
        free(value->pointer);
}

static struct type_info info_pointer = {
        .size = sizeof(struct type_pointer),
        .copy = copy_pointer,
        .comp = comp_pointer,
        .destroy = type_default_destroy
};

static struct type_info info_auto_pointer = {
        .size = sizeof(struct type_pointer),
        .copy = copy_pointer,
        .comp = comp_pointer,
        .destroy = destroy_pointer
};

const struct type_info *type_pointer(bool auto_free)
{
        return (auto_free ? &info_auto_pointer : &info_pointer);
}

/* String ----------------------------*/

static void copy_string(void *dest, const void *src)
{
        struct type_string *a = dest;
        const struct type_string *b = src;

        a->string = b->string;
}

static int comp_string(const void *first, const void *second)
{
        const struct type_string *a = first;
        const struct type_string *b = second;

        if (!a->string && !b->string)
                return 0;

        if (!a->string)
                return -1;

        if (!b->string)
                return 1;

        return strcmp(a->string, b->string);
}

static void destroy_string(void *data)
{
        struct type_string *value = data;
        free(value->string);
}

static struct type_info info_string = {
        .size = sizeof(struct type_string),
        .copy = copy_string,
        .comp = comp_string,
        .destroy = type_default_destroy
};

static struct type_info info_auto_string = {
        .size = sizeof(struct type_string),
        .copy = copy_string,
        .comp = comp_string,
        .destroy = destroy_string
};

const struct type_info *type_string(bool auto_free)
{
        return (auto_free ? &info_auto_string : &info_string);
}
