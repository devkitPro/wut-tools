#include <cstring>
#include <algorithm>
#include <excmd.h>

#include "entities/RootEntry.h"
#include "entities/OSFileEntry.h"
#include "entities/BufferFileEntry.h"

#include "services/RomFSService.h"
#include "services/TgaGzService.h"

static void deinitializeFreeImage() {
   FreeImage_DeInitialise();
}

static inline void addFolderIfNotEmpty(DirectoryEntry *parent, DirectoryEntry *child) {
   if (!child->getChildren().empty()) {
      parent->addChild(child);
   } else {
      delete child;
   }
}

static void addImageResource(DirectoryEntry *parent, const char* name, int width, int height, int bpp, excmd::option_state &options, const char *optName) {
   if (!options.has(optName))
      return;

   std::string path = options.get<std::string>(optName);
   FileEntry *file = createTgaGzFileEntry(path.c_str(), width, height, bpp, name);
   if (file) {
      parent->addChild(file);
   }
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
            .add_option("name",
                     description{"Long name of the application"},
                     value<std::string>{})
            .add_option("short-name",
                     description{"Short name of the application"},
                     value<std::string>{})
            .add_option("author",
                     description{"Author of the application"},
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

   auto codeFolder = new DirectoryEntry("code");
   auto metaFolder = new DirectoryEntry("meta");

   std::string rpxFilePath = options.get<std::string>("rpx-file");
   auto rpxFile = OSFileEntry::fromPath(rpxFilePath.c_str(), "root.rpx");
   codeFolder->addChild(rpxFile);

   {
      std::string long_name = options.has("name") ? options.get<std::string>("name") : (
         options.has("short-name") ? options.get<std::string>("short-name") : ""
      );
      std::string short_name = options.has("short-name") ? options.get<std::string>("short-name") : (
         options.has("name") ? options.get<std::string>("name") : ""
      );
      std::string author = options.has("author") ? options.get<std::string>("author") : "Built with devkitPPC & wut";

      if (long_name.empty() || short_name.empty()) {
         size_t startpos = 0, endpos = rpxFilePath.length();

#ifndef _WIN32
         size_t slash = rpxFilePath.find_last_of('/');
#else
         size_t slash = rpxFilePath.find_last_of("/\\");
#endif

         if (slash != std::string::npos) {
            startpos = slash+1;
         }

         size_t dot = rpxFilePath.find_last_of('.');
         if (dot != std::string::npos) {
            endpos = dot;
         }

         long_name = short_name = rpxFilePath.substr(startpos, endpos-startpos);
      }

#define MAKE_META_INI(_buf,_size) snprintf((_buf),(_size), \
         "[menu]\n" \
         "longname=%s\n" \
         "shortname=%s\n" \
         "author=%s\n", \
         long_name.c_str(), \
         short_name.c_str(), \
         author.c_str())

      std::vector<uint8_t> metaIniData;
      metaIniData.reserve(MAKE_META_INI(NULL, 0)+1);
      metaIniData.resize(metaIniData.capacity()-1);
      MAKE_META_INI(reinterpret_cast<char*>(metaIniData.data()), metaIniData.capacity());

#undef MAKE_META_INI

      FileEntry * metaIni = new BufferFileEntry("meta.ini", std::move(metaIniData));
      if(!metaIni){
         return EXIT_FAILURE;
      }

      metaFolder->addChild(metaIni);
   }

   addImageResource(metaFolder, "iconTex.tga.gz",     128, 128, 32, options, "icon");
   addImageResource(metaFolder, "bootTvTex.tga.gz",  1280, 720, 24, options, "tv-image");
   addImageResource(metaFolder, "bootDrcTex.tga.gz",  854, 480, 24, options, "drc-image");

   addFolderIfNotEmpty(root, codeFolder);
   addFolderIfNotEmpty(root, metaFolder);

   if (options.has("content")) {
      std::string contentPath = options.get<std::string>("content");

      filepath_t dirpath;
      filepath_init(&dirpath);
      filepath_set(&dirpath, contentPath.c_str());

      auto contentFolder = romfs::CreateFolderFromPath(dirpath, "content");
      addFolderIfNotEmpty(root, contentFolder);
   }

   std::string outputPath = options.get<std::string>("output");
   romfs::CreateArchive(root, outputPath.c_str());

   delete root;

   return EXIT_SUCCESS;
}
