#include <stdlib.h>
#include "BufferFileEntry.h"

void BufferFileEntry::write(FILE *f_out, off_t base_offset) {
    printf("Writing %s...\n", getFullPath().c_str());

    if(fseeko64(f_out, base_offset + this->offset + ROMFS_FILEPARTITION_OFS, SEEK_SET) != 0){
        fprintf(stderr, "Failed to seek!\n");
        exit(EXIT_FAILURE);
    }

    if (fwrite(&this->buffer[0], 1, this->size , f_out) != this->size) {
        fprintf(stderr, "Failed to write to output!\n");
        exit(EXIT_FAILURE);
    }
}
