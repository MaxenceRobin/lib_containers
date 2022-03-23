/**
 * @author Maxence ROBIN
 * @brief Types descriptors
 */

#ifndef LIB_TYPES_H
#define LIB_TYPES_H

/* Includes ------------------------------------------------------------------*/

#include <stddef.h>

/* Definitions ---------------------------------------------------------------*/

typedef void *(*type_copy_cb)(void *, const void *, size_t);
typedef int (*type_comp_cb)(const void *, const void *, size_t);
typedef void (*type_destroy_cb)(void *);

/**
 * @brief Callbacks for types manipulation.
 *
 * @param size : Size in bytes to allocate for an element of this type.
 * @param copy : Callback to copy an element in another one. The signature
 * follows the one of memcpy.
 * @param comp : Callback to compare two elements in order to classify them.
 * The signature follows the one of memcmp.
 * @param destroy : Callback to destroy eventual data inside an instance of this
 * this type. This callback MUST NOT destroy the element itself, only data
 * contained inside it (for exemple pointers), if there is nothing to destroy
 * inside an instance of this type, this callback must do nothing.
 */
struct type_info {
        size_t size;
        type_copy_cb copy;
        type_comp_cb comp;
        type_destroy_cb destroy;
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
 * @brief Returns a default copy function.
 */
type_copy_cb type_default_copy();

/**
 * @brief Returns a default comp function.
 */
type_comp_cb type_default_comp();

/**
 * @brief Returns a default destroy function.
 */
type_destroy_cb type_default_destroy();

#endif /* LIB_TYPES_H */
