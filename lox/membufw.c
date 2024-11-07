#include "membufw.h"

#include <stdlib.h>

void initMemBuffer(MemBuffer* mb) {
    mb->buf = (char*)calloc(MAX_MEM_BUFFER, sizeof(char*));
    mb->fptr = tmpfile();
    setvbuf(mb->fptr, mb->buf, _IOFBF, MAX_MEM_BUFFER);
}

void freeMemBuffer(MemBuffer* mb) {
    fclose(mb->fptr);
    free(mb->buf);
}
