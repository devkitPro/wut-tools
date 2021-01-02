#include "RootEntry.h"
#include "../services/RomFSService.h"
#include <cstring>

void RootEntry::updateEntryOffset(uint32_t *entry_offset) {
    if (entry_offset) {
        this->entry_offset = *entry_offset;
        *entry_offset += 0x18;
    }
}

void RootEntry::populate(romfs_infos_t *romfs_infos) {
    romfs_direntry_t *cur_entry = RomFSService::romfs_get_direntry(romfs_infos->dir_table, this->entry_offset);

    cur_entry->parent = be_word(this->entry_offset);
    cur_entry->sibling = be_word(this->sibling == nullptr ? ROMFS_ENTRY_EMPTY : this->sibling->entry_offset);
    cur_entry->child = be_word(this->dirChild == nullptr ? ROMFS_ENTRY_EMPTY : this->dirChild->entry_offset);
    cur_entry->file = be_word(this->fileChild == nullptr ? ROMFS_ENTRY_EMPTY : this->fileChild->entry_offset);

    uint32_t name_size = 0;
    uint32_t hash = RomFSService::calc_path_hash(0, (unsigned char *) this->getName().length(), 1, name_size);
    cur_entry->hash = romfs_infos->dir_hash_table[hash % romfs_infos->dir_hash_table_entry_count];
    romfs_infos->dir_hash_table[hash % romfs_infos->dir_hash_table_entry_count] = be_word(this->entry_offset);

    cur_entry->name_size = name_size;
    memcpy(cur_entry->name, this->getName().c_str(), name_size);

    cur_entry->name_size = be_word(cur_entry->name_size);

    for (auto const &e : children) {
        e->populate(romfs_infos);
    }
}

std::string RootEntry::getPath() {
    return "";
}
