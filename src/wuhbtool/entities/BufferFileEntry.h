#pragma once

#include "FileEntry.h"

class BufferFileEntry : public FileEntry {
public:
    BufferFileEntry(const std::string &name, std::vector<uint8_t> &data) : FileEntry(name) {
        this->buffer = data;
        this->size = buffer.size();
    }

    void write(FILE *f_out, off_t base_offset) override;

private:
    std::vector<uint8_t> buffer;

};
