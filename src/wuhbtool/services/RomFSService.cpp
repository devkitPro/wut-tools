#include "RomFSService.h"

uint32_t RomFSService::calc_path_hash(uint32_t parent, const unsigned char *path, uint32_t start, size_t path_len) {
    uint32_t hash = parent ^ 123456789;
    for (uint32_t i = 0; i < path_len; i++) {
        hash = (hash >> 5) | (hash << 27);
        hash ^= path[start + i];
    }

    return hash;
}

uint32_t RomFSService::romfs_get_hash_table_count(uint32_t num_entries) {
    if (num_entries < 3) {
        return 3;
    } else if (num_entries < 19) {
        return num_entries | 1;
    }
    uint32_t count = num_entries;
    while (count % 2 == 0 || count % 3 == 0 || count % 5 == 0 || count % 7 == 0 || count % 11 == 0 || count % 13 == 0 || count % 17 == 0) {
        count++;
    }
    return count;
}
