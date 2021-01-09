#pragma once

#include <cstdint>
#include "RomFSStructs.h"
#include "../entities/DirectoryEntry.h"

namespace romfs {

   inline romfs_direntry_t *GetDirEntry(romfs_direntry_t *directories, uint32_t offset) {
      return (romfs_direntry_t *) ((char *) directories + offset);
   }

   inline romfs_fentry_t *GetFileEntry(romfs_fentry_t *files, uint32_t offset) {
      return (romfs_fentry_t *) ((char *) files + offset);
   }

   uint32_t CalcPathHash(uint32_t parent, const unsigned char *path, uint32_t start, size_t path_len);
   DirectoryEntry *CreateFolderFromPath(filepath_t &dirpath, const char *name);
   void CreateArchive(DirectoryEntry *root, const char *outputFilePath);

}
