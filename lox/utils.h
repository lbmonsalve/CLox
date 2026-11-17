#pragma once
#ifndef clox_utils_h
#define clox_utils_h

#include <stdint.h>

#include "gc.h"

// Returns the smallest power of two that is equal to or greater than [n].
int PowerOf2Ceil(int n);

// We need buffers of a few different types. To avoid lots of casting between
// void* and back, we'll use the preprocessor as a poor man's generics and let
// it generate a few type-specific ones.
#define DECLARE_BUFFER(name, type)                                        \
    typedef struct                                                        \
    {                                                                     \
      type* data;                                                         \
      int count;                                                          \
      int capacity;                                                       \
    } name##Buffer;                                                       \
    void name##BufferInit(name##Buffer* buffer);                          \
    void name##BufferClear(GC* gc, name##Buffer* buffer);                 \
    void name##BufferFill(GC* gc, name##Buffer* buffer, type data,        \
                                int count);                               \
    void name##BufferWrite(GC* gc, name##Buffer* buffer, type data)
//
// This should be used once for each type instantiation, somewhere in a .c file.
#define DEFINE_BUFFER(name, type)                                              \
    void name##BufferInit(name##Buffer* buffer)                           \
    {                                                                          \
      buffer->data = NULL;                                                     \
      buffer->capacity = 0;                                                    \
      buffer->count = 0;                                                       \
    }                                                                          \
                                                                               \
    void name##BufferClear(GC* gc, name##Buffer* buffer)                  \
    {                                                                          \
      reallocate(gc, buffer->data, 0, 0);                                   \
      name##BufferInit(buffer);                                           \
    }                                                                          \
                                                                               \
    void name##BufferFill(GC* gc, name##Buffer* buffer, type data,        \
                                int count)                                     \
    {                                                                          \
      if (buffer->capacity < buffer->count + count)                            \
      {                                                                        \
        int capacity = PowerOf2Ceil(buffer->count + count);                 \
        buffer->data = (type*)reallocate(gc, buffer->data,                  \
            buffer->capacity * sizeof(type), capacity * sizeof(type));         \
        buffer->capacity = capacity;                                           \
      }                                                                        \
                                                                               \
      for (int i = 0; i < count; i++)                                          \
      {                                                                        \
        buffer->data[buffer->count++] = data;                                  \
      }                                                                        \
    }                                                                          \
                                                                               \
    void name##BufferWrite(GC* gc, name##Buffer* buffer, type data)       \
    {                                                                          \
      name##BufferFill(gc, buffer, data, 1);                              \
    }

DECLARE_BUFFER(Byte, uint8_t); // ByteBuffer

// Returns the number of bytes needed to encode [value] in UTF-8.
//
// Returns 0 if [value] is too large to encode.
int Utf8EncodeNumBytes(int value);

// Encodes value as a series of bytes in [bytes], which is assumed to be large
// enough to hold the encoded result.
//
// Returns the number of written bytes.
int Utf8Encode(int value, uint8_t* bytes);

#endif
