#include <sys/stat.h>
#include <cstring>
#include <algorithm>
#include <excmd.h>

#include "utils/utils.h"

#include "entities/OSFileEntry.h"
#include "entities/RootEntry.h"
#include "entities/FileEntry.h"
#include "entities/BufferFileEntry.h"

#include "services/RomFSService.h"
#include "services/TgaGzService.h"

DirectoryEntry *buildDirectoryFromPath(filepath_t &dirpath, std::string &&name);

void romfs_visit_dir(DirectoryEntry *curEntry, romfs_ctx_t *romfs_ctx);

void saveBundle(RootEntry &root, const std::string &outputFilePath);

static void deinitializeFreeImage() {
   FreeImage_DeInitialise();
}

int main(int argc, char **argv) {
   excmd::parser parser;
   excmd::option_state options;
   using excmd::description;
   using excmd::value;

   try {
      parser.global_options()
            .add_option("H,help",
                     description{"Show help."})
            .add_option("content",
                     description{"Path to the /content directory"},
                     value<std::string>{})
            .add_option("icon",
                     description{"Application icon (128x128)"},
                     value<std::string>{})
            .add_option("tv-image",
                     description{"Splash Screen image shown on the TV (1280x720)"},
                     value<std::string>{})
            .add_option("drc-image",
               description{"Splash Screen image shown on the DRC (854x480)"},
               value<std::string>{});

      parser.default_command()
            .add_argument("rpx-file",
                       description{"Path to RPX file"},
                       value<std::string>{})
            .add_argument("output",
                       description{"Path to WUHB file"},
                       value<std::string>{});

      options = parser.parse(argc, argv);
   } catch (excmd::exception &ex) {
      fprintf(stderr, "Error parsing options: %s\n", ex.what());
      return EXIT_FAILURE;
   }

   if (options.empty() || options.has("help")) {
      printf("%s <rpx-file> <output> [options]\n\n", argv[0]);
      printf("%s\n", parser.format_help(argv[0]).c_str());
      return EXIT_SUCCESS;
   }

   // Set up FreeImage
   FreeImage_Initialise();
   atexit(deinitializeFreeImage);

   auto root = new RootEntry();

   auto content = new DirectoryEntry("content");

   if (options.has("content")) {
      std::string contentPath = options.get<std::string>("content");

      filepath_t dirpath;
      filepath_init(&dirpath);
      filepath_set(&dirpath, contentPath.c_str());

      DirectoryEntry * contentDir = buildDirectoryFromPath(dirpath, "content");

      content->moveChildren(*contentDir);
   }

   std::string rpxFilePath = options.get<std::string>("rpx-file");
   std::string outputPath = options.get<std::string>("output");

   auto meta = new DirectoryEntry("meta");
   auto rpx = OSFileEntry::fromPath(rpxFilePath.c_str(), "boot.rpx");

   if (options.has("icon")) {
      std::string imagePath = options.get<std::string>("icon");

      FileEntry * icon = createTgaGzFileEntry(imagePath.c_str(), 128, 128, 32, "iconTex.tga.gz");
      if(!icon){
         return EXIT_FAILURE;
      }

      meta->addChild(icon);
   }

   if (options.has("tv-image")) {
      std::string imagePath = options.get<std::string>("tv-image");

      FileEntry * bootTv = createTgaGzFileEntry(imagePath.c_str(), 1280, 720, 24, "bootTvTex.tga.gz");
      if(!bootTv){
         return EXIT_FAILURE;
      }

      meta->addChild(bootTv);
   }

   if (options.has("drc-image")) {
      std::string imagePath = options.get<std::string>("drc-image");

      FileEntry * bootDrc = createTgaGzFileEntry(imagePath.c_str(), 854, 480, 24, "bootDrcTex.tga.gz");
      if(!bootDrc){
         return EXIT_FAILURE;
      }

      meta->addChild(bootDrc);
   }

   if(!meta->getChildren().empty()){
      root->addChild(meta);
   }else{
      delete meta;
   }

   if(!content->getChildren().empty()){
      root->addChild(content);
   }else{
      delete content;
   }

   root->addChild(rpx);

   saveBundle(*root, outputPath);

   delete root;

   return EXIT_SUCCESS;
}

void saveBundle(RootEntry &root, const std::string &outputFilePath) {
   romfs_ctx_t romfs_ctx;
   memset(&romfs_ctx, 0, sizeof(romfs_ctx));

   romfs_ctx.dir_table_size = 0x18; /* Root directory. */
   romfs_ctx.num_dirs = 1;

   root.fillRomFSInformation(&romfs_ctx);

   uint32_t dir_hash_table_entry_count = RomFSService::romfs_get_hash_table_count(romfs_ctx.num_dirs);
   uint32_t file_hash_table_entry_count = RomFSService::romfs_get_hash_table_count(romfs_ctx.num_files);
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
   root.calculateDirOffsets(&romfs_ctx, &entry_offset);
   entry_offset = 0;
   root.calculateFileOffsets(&romfs_ctx, &entry_offset);
   printf("Updating sibling and child entries...\n");
   root.updateSiblingAndChildEntries();
   printf("Populating data...\n");
   root.populate(&infos);

   romfs_header_t header;
   memset(&header, 0, sizeof(header));

   header.header_size = be_dword(sizeof(header));
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
   filepath_set(&outpath, outputFilePath.c_str());

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

   root.write(f_out, base_offset);

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

DirectoryEntry *buildDirectoryFromPath(filepath_t &dirpath, std::string &&name) {
   osdirent_t *cur_dirent = nullptr;
   filepath_t cur_path;
   filepath_t cur_sum_path;
   os_stat64_t cur_stats;

   auto *curDir = new DirectoryEntry(std::move(name));

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
         auto directoryEntry = buildDirectoryFromPath(cur_sum_path, cur_path.char_path);
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
