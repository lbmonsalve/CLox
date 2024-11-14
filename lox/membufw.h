#pragma once
#ifndef membufw_h
#define membufw_h

#include <stdio.h>

#define MAX_MEM_BUFFER 2048

// FILE* stream memory buffer.
typedef struct {
    char* buf;
    FILE* fptr;
} MemBuffer;

void initMemBuffer(MemBuffer* mb);
void freeMemBuffer(MemBuffer* mb);

#endif