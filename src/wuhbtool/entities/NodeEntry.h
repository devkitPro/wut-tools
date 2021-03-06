#pragma once

#include <cstdio>
#include <utility>
#include <string>
#include <vector>

#include "../services/RomFSStructs.h"

class DirectoryEntry;

class NodeEntry {
public:
    virtual ~NodeEntry() = default;

    explicit NodeEntry(std::string &&name, bool is_dir_node) {
        this->name = std::move(name);
        this->is_dir_node = is_dir_node;
    }

    virtual const std::string &getName() const {
        return name;
    }

    bool isDirNode() const {
        return is_dir_node;
    }

    bool isFileNode() const {
        return !is_dir_node;
    }

    void setParent(DirectoryEntry *_parent);

    DirectoryEntry * getParent();

    virtual void printRecursive(int indentation) {
        printf("%s%s\n", std::string(indentation, ' ').c_str(), getName().c_str());
    }

    virtual std::string getPath();

    virtual std::string getFullPath();

    virtual void populate(romfs_infos_t *romfs_infos) = 0;

    virtual void calculateFileOffsets(romfs_ctx_t *romfs_ctx, uint32_t *entry_offset) = 0;

    virtual void write(FILE *pIobuf, off_t offset) = 0;

    virtual void fillRomFSInformation(romfs_ctx_t *romfs_ctx) = 0;


    uint64_t offset = 0;
    uint64_t entry_offset = 0;

private:
    NodeEntry *parent = nullptr;
    std::string name;
    bool is_dir_node;
};
