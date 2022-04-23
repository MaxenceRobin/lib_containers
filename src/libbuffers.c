/**
 * @author Maxence ROBIN
 * @brief Provides simple circular buffers manipulation
 */

/* Includes ------------------------------------------------------------------*/

#include "libbuffers.h"

#include <errno.h>
#include <stdlib.h>

/* Definitions ---------------------------------------------------------------*/

struct buffer {
        const struct type_info *type;
        size_t count;
        unsigned int read;
        unsigned int write;
        enum { BUFFER_NONE, BUFFER_EMPTY, BUFFER_FULL } status;
};

/* Static functions ----------------------------------------------------------*/

static char *data_offset(const struct buffer *buffer, unsigned int pos)
{
        return (char *)(buffer + 1) + buffer->type->size * pos;
}

/**
 * @brief Adds 'data' to 'buffer'.
 *
 * @return 0 or ENOBUFS, respectively if 'buffer' is not full or if 'buffer' is
 * full after this call.
 */
static int push_value(struct buffer *buffer, const void *data)
{
        char *offset = data_offset(buffer, buffer->write);

        buffer->type->copy(offset, data);
        buffer->write = (buffer->write + 1) % buffer->count;

        if (buffer->write == buffer->read) {
                buffer->status = BUFFER_FULL;
                return ENOBUFS;
        }

        buffer->status = BUFFER_NONE;
        return 0;
}

static void destroy_values(const struct buffer *buffer)
{
        if (buffer->status == BUFFER_EMPTY)
                return;

        for (unsigned int i = buffer->read;
                        i != buffer->write;
                        i = (i + 1) % buffer->count)
                buffer->type->destroy(data_offset(buffer, i));
}

/* API -----------------------------------------------------------------------*/

struct buffer *buffer_create(const struct type_info *type, size_t count)
{
        if (!type || count == 0)
                return NULL;

        if (type->size == 0 || !type->copy || !type->destroy)
                return NULL;

        struct buffer *buffer = malloc(sizeof(*buffer) + type->size * count);
        if (!buffer)
                return NULL;

        buffer->type = type;
        buffer->count = count;
        buffer->read = 0;
        buffer->write = 0;
        buffer->status = BUFFER_EMPTY;

        return buffer;
}

void buffer_destroy(const struct buffer *buffer)
{
        if (!buffer)
                return;

        destroy_values(buffer);
        free((void *)buffer);
}

int buffer_push(struct buffer *buffer, const void *data)
{
        if (!buffer || !data)
                return -EINVAL;

        if (buffer->status == BUFFER_FULL)
                return -ENOBUFS;

        return push_value(buffer, data);
}

int buffer_f_push(struct buffer *buffer, const void *data)
{
        if (!buffer || !data)
                return -EINVAL;

        if (buffer->status == BUFFER_FULL)
                buffer->read = (buffer->read + 1) % buffer->count;

        return push_value(buffer, data);
}

int buffer_pop(struct buffer *buffer)
{
        if (!buffer)
                return -EINVAL;

        if (buffer->status == BUFFER_EMPTY)
                return -ENOMEM;

        buffer->type->destroy(data_offset(buffer, buffer->read));
        buffer->read = (buffer->read + 1) % buffer->count;

        if (buffer->read == buffer->write) {
                buffer->status = BUFFER_EMPTY;
                return ENOMEM;
        }

        buffer->status = BUFFER_NONE;
        return 0;
}

int buffer_clear(struct buffer *buffer)
{
        if (!buffer)
                return -EINVAL;

        destroy_values(buffer);
        buffer->read = buffer->write;
        buffer->status = BUFFER_EMPTY;

        return 0;
}

const void *buffer_data(const struct buffer *buffer)
{
        if (!buffer)
                return NULL;

        if (buffer->status == BUFFER_EMPTY)
                return NULL;

        return data_offset(buffer, buffer->read);
}

bool buffer_is_empty(const struct buffer *buffer)
{
        if (!buffer)
                return false;

        return (buffer->status == BUFFER_EMPTY);
}

bool buffer_is_full(const struct buffer *buffer)
{
        if (!buffer)
                return false;

        return (buffer->status == BUFFER_FULL);
}

ssize_t buffer_count(const struct buffer *buffer)
{
        if (!buffer)
                return -EINVAL;

        return (ssize_t)buffer->count;
}
