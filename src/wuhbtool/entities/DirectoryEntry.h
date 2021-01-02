#pragma once

#include "NodeEntry.h"
#include "FileEntry.h"
#include "../services/RomFSStructs.h"

class DirectoryEntry : public NodeEntry {
public:
    ~DirectoryEntry() override {
        for (auto const &e : children) {
            delete e;
        }
        children.clear();
    }

    explicit DirectoryEntry(std::string &&name) : NodeEntry(std::move(name), true) {

    }

    const std::vector<NodeEntry *> &getChildren() const {
        return children;
    }

    std::string getFullPath() override;

    bool addChild(NodeEntry *file);

    void printRecursive(int indentation) override {
        NodeEntry::printRecursive(indentation);

        for (auto const &e : children) {
            e->printRecursive(indentation + 3);
        }
    }

    void calculateFileOffsets(romfs_ctx_t *ptr, uint32_t *entry_offset) override;

    virtual void calculateDirOffsets(romfs_ctx_t *ptr, uint32_t *entry_offset);

    void populate(romfs_infos_t *romfs_infos) override;

    virtual void updateSiblingAndChildEntries();

    void write(FILE *pIobuf, off_t offset) override;

    virtual void moveChildren(DirectoryEntry &dirInput);

    void clearChildren();

    void fillRomFSInformation(romfs_ctx_t *romfs_ctx);
protected:
    virtual void updateEntryOffset(uint32_t *entry_offset);

    std::vector<NodeEntry *> children;

    DirectoryEntry *sibling = nullptr;
    DirectoryEntry *dirChild = nullptr;
    FileEntry *fileChild = nullptr;

    bool entry_offset_set = false;
};
