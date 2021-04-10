#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "RomFSService.h"
#include "../utils/utils.h"
#include "../entities/OSFileEntry.h"

namespace romfs {

namespace {

   inline unsigned char NormalizeChar(unsigned char c) {
      if (c >= 'a' && c <= 'z') {
         return c + 'A' - 'a';
      } else {
         return c;
      }
   }

   uint32_t GetHashTableCount(uint32_t num_entries) {
      if (num_entries < 3) {
         return 3;
      } else if (num_entries < 19) {
         return num_entries | 1;
      }
      uint32_t count = num_entries;
      while (count % 2 == 0 || count % 3 == 0 || count % 5 == 0 || count % 7 == 0 || count % 11 == 0 || count % 13 == 0 || count % 17 == 0) {
         count++;
      }
      return count;
   }

}

uint32_t CalcPathHash(uint32_t parent, const unsigned char *path, uint32_t start, size_t path_len) {
   uint32_t hash = parent ^ 123456789;
   for (uint32_t i = 0; i < path_len; i++) {
      hash = (hash >> 5) | (hash << 27);
      hash ^= NormalizeChar(path[start + i]);
   }

   return hash;
}

DirectoryEntry *CreateFolderFromPath(filepath_t &dirpath, const char *name) {
   osdirent_t *cur_dirent = nullptr;
   filepath_t cur_path;
   filepath_t cur_sum_path;
   os_stat64_t cur_stats;

   auto *curDir = new DirectoryEntry(name);

   osdir_t *dir = nullptr;
   if ((dir = os_opendir(dirpath.os_path)) == nullptr) {
      fprintf(stderr, "Failed to open directory %s!\n", dirpath.char_path);
      exit(EXIT_FAILURE);
   }

   while ((cur_dirent = os_readdir(dir))) {
      filepath_init(&cur_path);
      filepath_set(&cur_path, "");
      filepath_os_set(&cur_path, cur_dirent->d_name);

      if (strcmp(cur_path.char_path, ".") == 0 || strcmp(cur_path.char_path, "..") == 0) {
         /* Special case . and .. */
         continue;
      }

      filepath_copy(&cur_sum_path, &dirpath);
      filepath_os_append(&cur_sum_path, cur_dirent->d_name);

      if (os_stat(cur_sum_path.os_path, &cur_stats) == -1) {
         fprintf(stderr, "Failed to stat %s\n", cur_sum_path.char_path);
         exit(EXIT_FAILURE);
      }

      if ((cur_stats.st_mode & S_IFMT) == S_IFDIR) {
         auto directoryEntry = CreateFolderFromPath(cur_sum_path, cur_path.char_path);
         curDir->addChild(directoryEntry);
      } else if ((cur_stats.st_mode & S_IFMT) == S_IFREG) {
         auto fileEntry = new OSFileEntry(cur_sum_path, cur_path.char_path);
         fileEntry->size = cur_stats.st_size;
         curDir->addChild(fileEntry);
      } else {
         fprintf(stderr, "Invalid FS object type for %s!\n", cur_path.char_path);
         exit(EXIT_FAILURE);
      }
   }

   os_closedir(dir);
   return curDir;
}

void CreateArchive(DirectoryEntry *root, const char *outputFilePath) {
   romfs_ctx_t romfs_ctx;
   memset(&romfs_ctx, 0, sizeof(romfs_ctx));

   romfs_ctx.dir_table_size = 0x18; /* Root directory. */
   romfs_ctx.num_dirs = 1;

   root->fillRomFSInformation(&romfs_ctx);

   uint32_t dir_hash_table_entry_count = GetHashTableCount(romfs_ctx.num_dirs);
   uint32_t file_hash_table_entry_count = GetHashTableCount(romfs_ctx.num_files);
   romfs_ctx.dir_hash_table_size = 4 * dir_hash_table_entry_count;
   romfs_ctx.file_hash_table_size = 4 * file_hash_table_entry_count;

   auto dir_hash_table = static_cast<uint32_t *>(malloc(romfs_ctx.dir_hash_table_size));
   if (dir_hash_table == nullptr) {
      fprintf(stderr, "Failed to allocate directory hash table!\n");
      exit(EXIT_FAILURE);
   }

   for (uint32_t i = 0; i < dir_hash_table_entry_count; i++) {
      dir_hash_table[i] = be_word(ROMFS_ENTRY_EMPTY);
   }

   auto file_hash_table = static_cast<uint32_t *>(malloc(romfs_ctx.file_hash_table_size));
   if (file_hash_table == nullptr) {
      fprintf(stderr, "Failed to allocate file hash table!\n");
      exit(EXIT_FAILURE);
   }

   for (uint32_t i = 0; i < file_hash_table_entry_count; i++) {
      file_hash_table[i] = be_word(ROMFS_ENTRY_EMPTY);
   }

   auto dir_table = static_cast<romfs_direntry_t *>(calloc(1, romfs_ctx.dir_table_size));
   if (dir_table == nullptr) {
      fprintf(stderr, "Failed to allocate directory table!\n");
      exit(EXIT_FAILURE);
   }

   auto file_table = static_cast<romfs_fentry_t *>(calloc(1, romfs_ctx.file_table_size));
   if (file_table == nullptr) {
      fprintf(stderr, "Failed to allocate file table!\n");
      exit(EXIT_FAILURE);
   }

   romfs_infos_t infos;
   infos.file_table = file_table;
   infos.dir_table = dir_table;
   infos.dir_hash_table = dir_hash_table;
   infos.file_hash_table = file_hash_table;
   infos.dir_hash_table_entry_count = dir_hash_table_entry_count;
   infos.file_hash_table_entry_count = file_hash_table_entry_count;

   printf("Calculating metadata...\n");
   uint32_t entry_offset = 0;
   root->calculateDirOffsets(&romfs_ctx, &entry_offset);
   entry_offset = 0;
   root->calculateFileOffsets(&romfs_ctx, &entry_offset);
   printf("Updating sibling and child entries...\n");
   root->updateSiblingAndChildEntries();
   printf("Populating data...\n");
   root->populate(&infos);

   romfs_header_t header;
   memset(&header, 0, sizeof(header));

   memcpy(&header.header_magic, "WUHB", sizeof(header.header_magic));
   header.header_size = be_word(sizeof(header));
   header.file_hash_table_size = be_dword(romfs_ctx.file_hash_table_size);
   header.file_table_size = be_dword(romfs_ctx.file_table_size);
   header.dir_hash_table_size = be_dword(romfs_ctx.dir_hash_table_size);
   header.dir_table_size = be_dword(romfs_ctx.dir_table_size);
   header.file_partition_ofs = be_dword(ROMFS_FILEPARTITION_OFS);

   /* Abuse of endianness follows. */
   uint64_t dir_hash_table_ofs = align<uint64_t>(romfs_ctx.file_partition_size + ROMFS_FILEPARTITION_OFS, 4);
   header.dir_hash_table_ofs = dir_hash_table_ofs;
   header.dir_table_ofs = header.dir_hash_table_ofs + romfs_ctx.dir_hash_table_size;
   header.file_hash_table_ofs = header.dir_table_ofs + romfs_ctx.dir_table_size;
   header.file_table_ofs = header.file_hash_table_ofs + romfs_ctx.file_hash_table_size;
   header.dir_hash_table_ofs = be_dword(header.dir_hash_table_ofs);
   header.dir_table_ofs = be_dword(header.dir_table_ofs);
   header.file_hash_table_ofs = be_dword(header.file_hash_table_ofs);
   header.file_table_ofs = be_dword(header.file_table_ofs);

   filepath_t outpath;
   filepath_init(&outpath);
   filepath_set(&outpath, outputFilePath);

   off_t base_offset = 0;
   FILE *f_out = nullptr;

   if ((f_out = os_fopen(outpath.os_path, OS_MODE_WRITE)) == NULL) {
      fprintf(stderr, "Failed to open %s!\n", outpath.char_path);
      exit(EXIT_FAILURE);
   }

   printf("Writing header...\n");
   if(fseeko64(f_out, base_offset, SEEK_SET) != 0){
      fprintf(stderr, "Failed to seek!\n");
      exit(EXIT_FAILURE);
   }
   fwrite(&header, 1, sizeof(header), f_out);

   root->write(f_out, base_offset);

   printf("Writing dir_hash_table...\n");
   if(fseeko64(f_out, base_offset + dir_hash_table_ofs, SEEK_SET) != 0){
      fprintf(stderr, "Failed to seek!\n");
      exit(EXIT_FAILURE);
   }
   if (fwrite(dir_hash_table, 1, romfs_ctx.dir_hash_table_size, f_out) != romfs_ctx.dir_hash_table_size) {
      fprintf(stderr, "Failed to write dir hash table!\n");
      exit(EXIT_FAILURE);
   }
   free(dir_hash_table);

   printf("Writing dir_table...\n");
   if (fwrite(dir_table, 1, romfs_ctx.dir_table_size, f_out) != romfs_ctx.dir_table_size) {
      fprintf(stderr, "Failed to write dir table!\n");
      exit(EXIT_FAILURE);
   }
   free(dir_table);

   printf("Writing file_hash_table...\n");
   if (fwrite(file_hash_table, 1, romfs_ctx.file_hash_table_size, f_out) != romfs_ctx.file_hash_table_size) {
      fprintf(stderr, "Failed to write file hash table!\n");
      exit(EXIT_FAILURE);
   }
   free(file_hash_table);

   printf("Writing file_table...\n");
   if (fwrite(file_table, 1, romfs_ctx.file_table_size, f_out) != romfs_ctx.file_table_size) {
      fprintf(stderr, "Failed to write file table!\n");
      exit(EXIT_FAILURE);
   }
   free(file_table);
   fclose(f_out);
}

}
