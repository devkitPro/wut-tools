#include "../utils/filepath.h"

#include <sys/stat.h>
#include <sys/types.h>
#include "OSFileEntry.h"

void OSFileEntry::write(FILE *f_out, off_t base_offset) {
    std::cout << "Writing " << getFullPath() << std::endl;

#ifdef WIN32
    std::wstring widestr = std::wstring(this->osPath.begin(), this->osPath.end());
    FILE *f_in = os_fopen(widestr.c_str(), OS_MODE_READ);
#else
    FILE *f_in = os_fopen(this->osPath.c_str(), OS_MODE_READ);
#endif

    if (f_in == nullptr) {
        fprintf(stderr, "Failed to open %s!\n", getFullPath().c_str());
        exit(EXIT_FAILURE);
    }

    /* Write files. */
    auto *buffer = static_cast<unsigned char *>(malloc(0x400000));
    if (buffer == nullptr) {
        fprintf(stderr, "Failed to allocate work buffer!\n");
        exit(EXIT_FAILURE);
    }

    if(fseeko64(f_out, base_offset + this->offset + ROMFS_FILEPARTITION_OFS, SEEK_SET) != 0){
        fprintf(stderr, "Failed to seek!\n");
        exit(EXIT_FAILURE);
    }
    uint64_t offset = 0;
    uint64_t read_size = 0x400000;
    while (offset < this->size) {
        if (this->size - offset < read_size) {
            read_size = this->size - offset;
        }

        if (fread(buffer, 1, read_size, f_in) != read_size) {
            fprintf(stderr, "Failed to read from %s!\n", this->osPath);
            exit(EXIT_FAILURE);
        }

        if (fwrite(buffer, 1, read_size, f_out) != read_size) {
            fprintf(stderr, "Failed to write to output!\n");
            exit(EXIT_FAILURE);
        }

        offset += read_size;
    }

    os_fclose(f_in);
    free(buffer);
}

FileEntry *OSFileEntry::fromPath(const std::string &inputPath) {
    filepath_t cur_path;
    filepath_init(&cur_path);
    filepath_set(&cur_path, inputPath.c_str());
    os_stat64_t cur_stats;

    if (os_stat(cur_path.os_path, &cur_stats) == -1) {
        fprintf(stderr, "Failed to stat %s\n", cur_path.char_path);
        exit(EXIT_FAILURE);
    }

    std::string filename = inputPath;

    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        filename.erase(0, last_slash_idx + 1);
    }

    auto res = new OSFileEntry(cur_path.char_path, filename);
    res->size = cur_stats.st_size;

    return res;
}
