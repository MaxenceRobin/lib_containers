/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

/* Includes ------------------------------------------------------------------*/

#include "libtypes.h"

#include <stdlib.h>
#include <string.h>

/* Definitions ---------------------------------------------------------------*/

/* Copy and comp callbacks -----------*/

#define DECL_COPY_COMP(name, type) \
\
static void copy_##name(void *dest, const void *src) \
{ \
        *((type *)dest) = *((type *)src); \
} \
\
static int comp_##name(const void *first, const void *second) \
{ \
        return *((type *)first) - *((type *)second); \
}

/* Hashable types --------------------*/

#define DECL_TYPE_HASH(name, type) \
\
DECL_COPY_COMP(name, type) \
\
static unsigned long hash_##name(const void *data) \
{ \
        unsigned long value = *((type *)data); \
\
        value = ((value >> 16) ^ value) * 0x45d9f3b; \
        value = ((value >> 16) ^ value) * 0x45d9f3b; \
        value = (value >> 16) ^ value; \
\
        return value; \
} \
\
static struct type_info info_##name = { \
        .size = sizeof(type), \
        .copy = copy_##name, \
        .comp = comp_##name, \
        .hash = hash_##name, \
        .destroy = type_default_destroy \
}; \
\
const struct type_info *type_##name() \
{ \
        return &info_##name; \
}

/* Non hashable types ----------------*/

#define DECL_TYPE_NO_HASH(name, type) \
\
DECL_COPY_COMP(name, type) \
\
static struct type_info info_##name = { \
        .size = sizeof(type), \
        .copy = copy_##name, \
        .comp = comp_##name, \
        .hash = NULL, \
        .destroy = type_default_destroy \
}; \
\
const struct type_info *type_##name() \
{ \
        return &info_##name; \
}

/* API -----------------------------------------------------------------------*/

DECL_TYPE_HASH(char, char)
DECL_TYPE_HASH(uchar, unsigned char)

DECL_TYPE_HASH(short, short)
DECL_TYPE_HASH(ushort, unsigned short)

DECL_TYPE_HASH(int, int)
DECL_TYPE_HASH(uint, unsigned int)

DECL_TYPE_HASH(long, long)
DECL_TYPE_HASH(ulong, unsigned long)

DECL_TYPE_HASH(long_long, long long)
DECL_TYPE_HASH(ulong_long, unsigned long long)

DECL_TYPE_NO_HASH(float, float)
DECL_TYPE_NO_HASH(double, double)
DECL_TYPE_NO_HASH(long_double, long double)

void type_default_destroy(const void *data)
{
}

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

static void destroy_pointer(const void *data)
{
        const struct type_pointer *value = data;
        free(value->pointer);
}

static struct type_info info_pointer = {
        .size = sizeof(struct type_pointer),
        .copy = copy_pointer,
        .comp = comp_pointer,
        .hash = NULL,
        .destroy = type_default_destroy
};

static struct type_info info_auto_pointer = {
        .size = sizeof(struct type_pointer),
        .copy = copy_pointer,
        .comp = comp_pointer,
        .hash = NULL,
        .destroy = destroy_pointer
};

const struct type_info *type_pointer(enum type_destroy_policy policy)
{
        return (policy == DESTROY_POLICY_AUTO_FREE ?
                        &info_auto_pointer : &info_pointer);
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

static unsigned long hash_string(const void *data)
{
        unsigned long hash = 5381;
        unsigned char c;
        const char *string = ((struct type_string *)data)->string;

        if (!string)
                return (unsigned long)-1;

        while (c = *string++)
                hash = (hash << 5) + hash + c;

        return hash;
}

static void destroy_string(const void *data)
{
        const struct type_string *value = data;
        free(value->string);
}

static struct type_info info_string = {
        .size = sizeof(struct type_string),
        .copy = copy_string,
        .comp = comp_string,
        .hash = hash_string,
        .destroy = type_default_destroy
};

static struct type_info info_auto_string = {
        .size = sizeof(struct type_string),
        .copy = copy_string,
        .comp = comp_string,
        .hash = hash_string,
        .destroy = destroy_string
};

const struct type_info *type_string(enum type_destroy_policy policy)
{
        return (policy == DESTROY_POLICY_AUTO_FREE ?
                        &info_auto_string : &info_string);
}
