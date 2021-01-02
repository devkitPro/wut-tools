#pragma once

#include <cstdint>
#include "RomFSStructs.h"

class RomFSService {
public:
    static romfs_direntry_t *romfs_get_direntry(romfs_direntry_t *directories, uint32_t offset) {
        return (romfs_direntry_t *) ((char *) directories + offset);
    }

    static romfs_fentry_t *romfs_get_fentry(romfs_fentry_t *files, uint32_t offset) {
        return (romfs_fentry_t *) ((char *) files + offset);
    }

    static uint32_t calc_path_hash(uint32_t parent, const unsigned char *path, uint32_t start, size_t path_len);

    static uint32_t romfs_get_hash_table_count(uint32_t num_entries);
};
