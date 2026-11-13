#include "utils.h"
#include "memory.h"

// From: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
int PowerOf2Ceil(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

void ByteBufferInit(ByteBuffer* buffer)
{
    buffer->data = NULL;
    buffer->capacity = 0;
    buffer->count = 0;
}

void ByteBufferClear(GC* gc, ByteBuffer* buffer)
{
    reallocate(gc, buffer->data, 0, 0);
    ByteBufferInit(buffer);
}

void ByteBufferFill(GC* gc, ByteBuffer* buffer, uint8_t data,
    int count)
{
    if (buffer->capacity < buffer->count + count)
    {
        int capacity = PowerOf2Ceil(buffer->count + count);
        buffer->data = (uint8_t*)reallocate(gc, buffer->data,
            buffer->capacity * sizeof(uint8_t), capacity * sizeof(uint8_t));
        buffer->capacity = capacity;
    }

    for (int i = 0; i < count; i++)
    {
        buffer->data[buffer->count++] = data;
    }
}

void ByteBufferWrite(GC* gc, ByteBuffer* buffer, uint8_t data)
{
    ByteBufferFill(gc, buffer, data, 1);
}

//

int Utf8EncodeNumBytes(int value)
{
    //ASSERT(value >= 0, "Cannot encode a negative value.");

    if (value <= 0x7f) return 1;
    if (value <= 0x7ff) return 2;
    if (value <= 0xffff) return 3;
    if (value <= 0x10ffff) return 4;
    return 0;
}


int Utf8Encode(int value, uint8_t* bytes)
{
    if (value <= 0x7f)
    {
        // Single byte (i.e. fits in ASCII).
        *bytes = value & 0x7f;
        return 1;
    }
    else if (value <= 0x7ff)
    {
        // Two byte sequence: 110xxxxx 10xxxxxx.
        *bytes = 0xc0 | ((value & 0x7c0) >> 6);
        bytes++;
        *bytes = 0x80 | (value & 0x3f);
        return 2;
    }
    else if (value <= 0xffff)
    {
        // Three byte sequence: 1110xxxx 10xxxxxx 10xxxxxx.
        *bytes = 0xe0 | ((value & 0xf000) >> 12);
        bytes++;
        *bytes = 0x80 | ((value & 0xfc0) >> 6);
        bytes++;
        *bytes = 0x80 | (value & 0x3f);
        return 3;
    }
    else if (value <= 0x10ffff)
    {
        // Four byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
        *bytes = 0xf0 | ((value & 0x1c0000) >> 18);
        bytes++;
        *bytes = 0x80 | ((value & 0x3f000) >> 12);
        bytes++;
        *bytes = 0x80 | ((value & 0xfc0) >> 6);
        bytes++;
        *bytes = 0x80 | (value & 0x3f);
        return 4;
    }

    // Invalid Unicode value. See: http://tools.ietf.org/html/rfc3629
    //UNREACHABLE();
    return 0;
}
