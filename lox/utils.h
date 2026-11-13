#pragma once
#ifndef clox_utils_h
#define clox_utils_h

#include <stdint.h>

#include "gc.h"

typedef struct
{
	uint8_t* data;
	int count;
	int capacity;
} ByteBuffer;
void ByteBufferInit(ByteBuffer* buffer);
void ByteBufferClear(GC* gc, ByteBuffer* buffer);
void ByteBufferFill(GC* gc, ByteBuffer* buffer, uint8_t data,
	int count);
void ByteBufferWrite(GC* gc, ByteBuffer* buffer, uint8_t data);

//

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

/*
// Returns the smallest power of two that is equal to or greater than [n].
int wrenPowerOf2Ceil(int n);

// We need buffers of a few different types. To avoid lots of casting between
// void* and back, we'll use the preprocessor as a poor man's generics and let
// it generate a few type-specific ones.
#define DECLARE_BUFFER(name, type)                                             \
    typedef struct                                                             \
    {                                                                          \
      type* data;                                                              \
      int count;                                                               \
      int capacity;                                                            \
    } name##Buffer;                                                            \
    void lox##name##BufferInit(name##Buffer* buffer);                          \
    void lox##name##BufferClear(VM* vm, name##Buffer* buffer);                 \
    void lox##name##BufferFill(VM* vm, name##Buffer* buffer, type data,        \
                                int count);                                    \
    void lox##name##BufferWrite(VM* vm, name##Buffer* buffer, type data)

// This should be used once for each type instantiation, somewhere in a .c file.
#define DEFINE_BUFFER(name, type)                                              \
    void lox##name##BufferInit(name##Buffer* buffer)                           \
    {                                                                          \
      buffer->data = NULL;                                                     \
      buffer->capacity = 0;                                                    \
      buffer->count = 0;                                                       \
    }                                                                          \
                                                                               \
    void lox##name##BufferClear(VM* vm, name##Buffer* buffer)                  \
    {                                                                          \
      reallocate(vm, buffer->data, 0, 0);                                   \
      lox##name##BufferInit(buffer);                                           \
    }                                                                          \
                                                                               \
    void lox##name##BufferFill(VM* vm, name##Buffer* buffer, type data,        \
                                int count)                                     \
    {                                                                          \
      if (buffer->capacity < buffer->count + count)                            \
      {                                                                        \
        int capacity = loxPowerOf2Ceil(buffer->count + count);                 \
        buffer->data = (type*)reallocate(vm, buffer->data,                  \
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
    void lox##name##BufferWrite(VM* vm, name##Buffer* buffer, type data)       \
    {                                                                          \
      lox##name##BufferFill(vm, buffer, data, 1);                              \
    }

DECLARE_BUFFER(Byte, uint8_t);
*/