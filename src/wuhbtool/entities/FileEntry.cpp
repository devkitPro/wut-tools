#include "FileEntry.h"
#include "DirectoryEntry.h"
#include "../utils/utils.h"
#include "../services/RomFSService.h"
#include <cstring>

void FileEntry::calculateFileOffsets(romfs_ctx_t *romfs_ctx, uint32_t *entry_offset) {
    romfs_ctx->file_partition_size = align<uint64_t>(romfs_ctx->file_partition_size, 0x10);
    this->offset = romfs_ctx->file_partition_size;
    romfs_ctx->file_partition_size += this->size;
    if (entry_offset) {
        this->entry_offset = *entry_offset;
        *entry_offset += 0x20 + align<uint32_t>(getName().size(), 4);
    }
}

void FileEntry::populate(romfs_infos_t * romfs_infos) {
    romfs_fentry_t *cur_entry = romfs::GetFileEntry(romfs_infos->file_table, this->entry_offset);

    cur_entry->parent = be_word(this->getParent()->entry_offset);
    cur_entry->sibling = be_word(this->sibling == nullptr ? ROMFS_ENTRY_EMPTY : this->sibling->entry_offset);
    cur_entry->offset = be_dword(this->offset);
    cur_entry->size = be_dword(this->size);

    uint32_t name_size = getName().length();
    uint32_t hash = romfs::CalcPathHash(this->getParent()->entry_offset, reinterpret_cast<const unsigned char *>(("/" + getName()).c_str()), 1, name_size);
    cur_entry->hash = romfs_infos->file_hash_table[hash % romfs_infos->file_hash_table_entry_count];
    romfs_infos->file_hash_table[hash % romfs_infos->file_hash_table_entry_count] = be_word(this->entry_offset);

    cur_entry->name_size = name_size;
    memcpy(cur_entry->name, getName().c_str(), name_size);
    cur_entry->name_size = be_word(cur_entry->name_size);
}

void FileEntry::fillRomFSInformation(romfs_ctx_t *romfs_ctx) {
    romfs_ctx->num_files++;
    romfs_ctx->file_table_size += 0x20 + align<uint64_t>(getName().size(), 4);
}
