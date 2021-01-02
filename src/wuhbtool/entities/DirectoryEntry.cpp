#include "DirectoryEntry.h"
#include <cstring>
#include "../utils/utils.h"
#include "../services/RomFSService.h"

bool DirectoryEntry::addChild(NodeEntry *file) {
    if (file) {
        children.push_back(file);
        file->setParent(this);
        return true;
    }
    return false;
}

void DirectoryEntry::calculateDirOffsets(romfs_ctx_t *romfs_ctx, uint32_t *entry_offset) {
    if (!this->entry_offset_set) {
        this->updateEntryOffset(entry_offset);
        this->entry_offset_set = true;
    }

    for (auto const &e : children) {
        if (instanceof<DirectoryEntry>(e)) {
            auto curDirEntry = dynamic_cast<DirectoryEntry *>(e);
            curDirEntry->calculateDirOffsets(romfs_ctx, entry_offset);
        }
    }
}

void DirectoryEntry::calculateFileOffsets(romfs_ctx_t *romfs_ctx, uint32_t *entry_offset) {
    for (auto const &e : children) {
        if (instanceof<DirectoryEntry>(e)) {
            e->calculateFileOffsets(romfs_ctx, entry_offset);
        }
    }
    for (auto const &e : children) {
        if (instanceof<FileEntry>(e)) {
            e->calculateFileOffsets(romfs_ctx, entry_offset);
        }
    }
}

void DirectoryEntry::updateEntryOffset(uint32_t *entry_offset) {
    if (entry_offset) {
        this->entry_offset = *entry_offset;
        *entry_offset += 0x18 + align<uint32_t>(getName().size(), 4);
    }
}

void DirectoryEntry::populate(romfs_infos_t *romfs_infos) {
    romfs_direntry_t *cur_entry = RomFSService::romfs_get_direntry(romfs_infos->dir_table, this->entry_offset);
    cur_entry->parent = be_word(this->getParent()->entry_offset);
    cur_entry->sibling = be_word(this->sibling == nullptr ? ROMFS_ENTRY_EMPTY : this->sibling->entry_offset);
    cur_entry->child = be_word(this->dirChild == nullptr ? ROMFS_ENTRY_EMPTY : this->dirChild->entry_offset);
    cur_entry->file = be_word(this->fileChild == nullptr ? ROMFS_ENTRY_EMPTY : this->fileChild->entry_offset);

    uint32_t name_size = getName().size();
    uint32_t hash = RomFSService::calc_path_hash(this->getParent()->entry_offset, reinterpret_cast<const unsigned char *>(("/" + getName()).c_str()), 1, name_size);

    cur_entry->hash = romfs_infos->dir_hash_table[hash % romfs_infos->dir_hash_table_entry_count];
    romfs_infos->dir_hash_table[hash % romfs_infos->dir_hash_table_entry_count] = be_word(this->entry_offset);

    cur_entry->name_size = name_size;

    memcpy(cur_entry->name, getName().c_str(), name_size);

    cur_entry->name_size = be_word(cur_entry->name_size);

    for (auto const &e : children) {
        e->populate(romfs_infos);
    }
}

void DirectoryEntry::updateSiblingAndChildEntries() {
    FileEntry *lastChild = nullptr;
    DirectoryEntry *lastDir = nullptr;
    for (auto const &e : children) {
        if (instanceof<DirectoryEntry>(e)) {
            auto curDirEntry = dynamic_cast<DirectoryEntry *>(e);

            if (lastDir == nullptr) {
                this->dirChild = curDirEntry;
            } else {
                lastDir->sibling = curDirEntry;
            }
            lastDir = curDirEntry;
            curDirEntry->updateSiblingAndChildEntries();
        } else if (instanceof<FileEntry>(e)) {
            auto curFileEntry = dynamic_cast<FileEntry *>(e);
            if (lastChild == nullptr) {
                this->fileChild = curFileEntry;
            } else {
                lastChild->sibling = curFileEntry;
            }
            lastChild = curFileEntry;
        }
    }
}

std::string DirectoryEntry::getFullPath() {
    return NodeEntry::getFullPath() + OS_PATH_SEPARATOR;
}

void DirectoryEntry::write(FILE *pIobuf, off_t offset) {
    for (auto const &e : children) {
        e->write(pIobuf, offset);
    }
}

void DirectoryEntry::clearChildren() {
    this->children.clear();
}

void DirectoryEntry::moveChildren(DirectoryEntry &dirInput) {
    for (auto const &e : dirInput.getChildren()) {
        this->addChild(e);
    }
    dirInput.clearChildren();
}

void DirectoryEntry::fillRomFSInformation(romfs_ctx_t *romfs_ctx) {
    romfs_ctx->num_dirs++;
    romfs_ctx->dir_table_size += 0x18 + align<uint64_t>(getName().size(), 4);

    for (auto const &e : getChildren()) {
        e->fillRomFSInformation(romfs_ctx);
    }
}
