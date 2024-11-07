#pragma once
#ifndef membufw_h
#define membufw_h

#include <stdio.h>

#define MAX_MEM_BUFFER 1024
#define MAX_LINE_BUFFER 256

// FILE* stream memory buffer.
typedef struct {
    char* buf;
    FILE* fptr;
} MemBuffer;

void initMemBuffer(MemBuffer* mb);
void freeMemBuffer(MemBuffer* mb);

#endif