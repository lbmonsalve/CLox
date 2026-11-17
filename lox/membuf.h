#pragma once
#ifndef membuf_h
#define membuf_h

#include <stdio.h>

#ifdef _WIN32
#ifndef MAX_MEM_BUFFER
    #define MAX_MEM_BUFFER 4096 //16384
#endif
#endif

// FILE* stream memory buffer.
typedef struct {
    char* buf;
    FILE* fptr;
} MemBuffer;

void initMemBuffer(MemBuffer* mb);
void freeMemBuffer(MemBuffer* mb);

#endif