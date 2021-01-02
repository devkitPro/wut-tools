#pragma once

#include "FileEntry.h"

class OSFileEntry : public FileEntry {
public:
    OSFileEntry(const std::string &path, const std::string &name) : FileEntry(name) {
        this->osPath = path;
    }

    std::string getOSPath() const {
        return osPath;
    }

    void write(FILE *pIobuf, off_t offset) override;

    static FileEntry* fromPath(const std::string& inputPath);

private:
    std::string osPath;
};
