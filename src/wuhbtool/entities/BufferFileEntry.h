#pragma once

#include "FileEntry.h"

class BufferFileEntry final : public FileEntry {
public:
    BufferFileEntry(std::string &&name, std::vector<uint8_t> &&data) : FileEntry(std::move(name)) {
        this->buffer = std::move(data);
        this->size = buffer.size();
    }

    void write(FILE *f_out, off_t base_offset) override;

private:
    std::vector<uint8_t> buffer;
};
