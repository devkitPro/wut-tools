#pragma once

#include "DirectoryEntry.h"
#include "../services/RomFSStructs.h"

class RootEntry : public DirectoryEntry {
public:
    explicit RootEntry() : DirectoryEntry("") {
    }

    std::string getPath() override;

    void updateEntryOffset(uint32_t *entry_offset) override;

    void populate(romfs_infos_t *romfs_infos) override;

};
