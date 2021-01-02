#pragma once

#include "FileEntry.h"

class OSFileEntry final : public FileEntry {
public:
    OSFileEntry(filepath_t &path, std::string &&name) : FileEntry(std::move(name)) {
        this->osPath = path;
    }

    void write(FILE *pIobuf, off_t offset) override;

    static FileEntry* fromPath(const char* inputPath, const char* filename);

private:
    filepath_t osPath;
};
