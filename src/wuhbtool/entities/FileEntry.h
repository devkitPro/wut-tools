#pragma once

#include "NodeEntry.h"
#include "../services/RomFSStructs.h"

class FileEntry : public NodeEntry {
public:
    explicit FileEntry(std::string &&name) : NodeEntry(std::move(name), false) {
    }

    void calculateFileOffsets(romfs_ctx_t *ptr, uint32_t *entry_offset) override;

    void populate(romfs_infos_t *romfs_infos) override;

    void fillRomFSInformation(romfs_ctx_t *romfs_ctx) override;

    FileEntry *sibling = nullptr;
    uint64_t size = 0;

};
