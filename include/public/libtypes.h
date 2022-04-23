/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

#ifndef LIB_TYPES_H
#define LIB_TYPES_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>

/* Definitions ---------------------------------------------------------------*/

typedef void (*type_copy_cb)(void *, const void *);
typedef int (*type_comp_cb)(const void *, const void *);
typedef unsigned long (*type_hash_cb)(const void *);
typedef void (*type_destroy_cb)(const void *);

/**
 * @brief Callbacks for types manipulation.
 *
 * @param size : Size in bytes to allocate for an element of this type.
 * @param copy : Callback to copy an element in another one.
 * @param comp : Callback to compare two elements in order to classify them.
 * @param destroy : Callback to destroy eventual data inside an instance of this
 * type. This callback MUST NOT destroy the element itself, only data
 * contained inside it (for exemple pointers), if there is nothing to destroy
 * inside an instance of this type, this callback must do nothing.
 */
struct type_info {
        size_t size;
        type_copy_cb copy;
        type_comp_cb comp;
        type_hash_cb hash;
        type_destroy_cb destroy;
};

struct type_pointer {
        void *pointer;
};

struct type_string {
        char *string;
};

/* API -----------------------------------------------------------------------*/

const struct type_info *type_char();
const struct type_info *type_uchar();

const struct type_info *type_short();
const struct type_info *type_ushort();

const struct type_info *type_int();
const struct type_info *type_uint();

const struct type_info *type_long();
const struct type_info *type_ulong();

const struct type_info *type_long_long();
const struct type_info *type_ulong_long();

const struct type_info *type_float();
const struct type_info *type_double();
const struct type_info *type_long_double();

/**
 * @param auto_free : If 'true', the address pointed by the pointer will be
 * automatically free()'d on destruction. If 'false' nothing will be done.
 */
const struct type_info *type_pointer(bool auto_free);

/**
 * @param auto_free : If 'true', the address pointed by the pointer will be
 * automatically free()'d on destruction. If 'false' nothing will be done.
 */
const struct type_info *type_string(bool auto_free);

/**
 * @brief Default destroy function. The default behavior is to do nothing.
 */
void type_default_destroy(const void *);

#endif /* LIB_TYPES_H */
