#include "membuf.h"

#include <stdlib.h>

void initMemBuffer(MemBuffer* mb) {
#ifdef _WIN32
    mb->buf = (char*)calloc(MAX_MEM_BUFFER, sizeof(char*));
    mb->fptr = tmpfile();
    setvbuf(mb->fptr, mb->buf, _IOFBF, MAX_MEM_BUFFER);
#else
    mb->fptr = open_memstream(&mb->buf, &mb->size);
#endif
}

void freeMemBuffer(MemBuffer* mb) {
    fclose(mb->fptr);
    free(mb->buf);
}
