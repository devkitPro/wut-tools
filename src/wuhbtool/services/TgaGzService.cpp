#include "TgaGzService.h"
#include "../entities/BufferFileEntry.h"
#include <vector>
#include <zlib.h>

namespace {

   struct FiMemoryFile {
      std::vector<uint8_t> data;
      long pos;

      FiMemoryFile() : data{}, pos{} { }

      long getSize() {
         return pos > data.size() ? pos : data.size();
      }

      void ensureSize() {
         if (pos > data.size()) {
            data.resize(pos);
         }
      }

      /*
      unsigned read(void *buffer, unsigned size, unsigned count) {
         // etc
      }
      */

      unsigned write(const void *buffer, unsigned size, unsigned count) {
         long writepos = pos;
         long writesize = size*count;

         pos += writesize;
         ensureSize();

         memcpy(&data[writepos], buffer, writesize);
         return count;
      }

      int seek(long offset, int origin) {
         if (origin == SEEK_SET) {
            pos = offset;
         } else if (origin == SEEK_CUR) {
            pos += offset;
         } else if (origin == SEEK_END) {
            pos = getSize() + offset;
         }

         if (pos < 0) {
            pos = 0;
         }

         return 0;
      }

      long tell() {
         return pos;
      }
   };

   /*
   unsigned DLL_CALLCONV FiMemoryFileReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
      FiMemoryFile *f = static_cast<FiMemoryFile*>(handle);
      return f->read(buffer, size, count);
   }
   */

   unsigned DLL_CALLCONV FiMemoryFileWriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
      FiMemoryFile *f = static_cast<FiMemoryFile*>(handle);
      return f->write(buffer, size, count);
   }

   int DLL_CALLCONV FiMemoryFileSeekProc(fi_handle handle, long offset, int origin) {
      FiMemoryFile *f = static_cast<FiMemoryFile*>(handle);
      return f->seek(offset, origin);
   }

   long DLL_CALLCONV FiMemoryFileTellProc(fi_handle handle) {
      FiMemoryFile *f = static_cast<FiMemoryFile*>(handle);
      return f->tell();
   }

   constexpr FreeImageIO FiMemoryFileIO = {
      NULL, //FiMemoryFileReadProc,
      FiMemoryFileWriteProc,
      FiMemoryFileSeekProc,
      FiMemoryFileTellProc,
   };

   std::vector<uint8_t> gzCompress(const void* data, size_t size) {
      std::vector<uint8_t> buffer;
      uint8_t chunk[16*1024];

      z_stream z = {};
      deflateInit2(&z, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS | 16, 8, Z_DEFAULT_STRATEGY);

      z.avail_in = size;
      z.next_in = static_cast<Bytef*>(const_cast<void*>(data));

      do {
         z.avail_out = sizeof(chunk);
         z.next_out = static_cast<Bytef*>(chunk);

         int ret = deflate(&z, Z_FINISH);
         if (ret == Z_STREAM_ERROR) {
            deflateEnd(&z);
            fprintf(stderr, "Zlib compression error\n");
            exit(EXIT_FAILURE);
         }

         buffer.insert(buffer.end(), chunk, static_cast<uint8_t*>(z.next_out));
      } while (z.avail_out == 0);

      deflateEnd(&z);
      return buffer;
   }

}

FileEntry* createTgaGzFileEntry(const char* inputFile, int width, int height, const char* filename) {
   FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(inputFile);
   if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif)) {
      fprintf(stderr, "Unknown or unsupported image format: %s\n", inputFile);
      return nullptr;
   }

   FIBITMAP* bmp = FreeImage_Load(fif, inputFile, 0);

   if (bmp && (FreeImage_GetImageType(bmp) != FIT_BITMAP || FreeImage_GetBPP(bmp) != 24)) {
      FIBITMAP* newbmp = FreeImage_ConvertTo24Bits(bmp);
      FreeImage_Unload(bmp);
      bmp = newbmp;
   }

   if (bmp && (FreeImage_GetWidth(bmp) != width || FreeImage_GetHeight(bmp) != height)) {
      fprintf(stderr, "Warning: Image %s has incorrect size (expected %dx%d), resizing...\n", inputFile, width, height);
      FIBITMAP* newbmp = FreeImage_Rescale(bmp, width, height, FILTER_BILINEAR);
      FreeImage_Unload(bmp);
      bmp = newbmp;
   }

   if (!bmp) {
      fprintf(stderr, "Failed to load image: %s\n", inputFile);
      return nullptr;
   }

   FiMemoryFile tga;
   FreeImage_SaveToHandle(FIF_TARGA, bmp, const_cast<FreeImageIO*>(&FiMemoryFileIO), &tga, TARGA_DEFAULT);
   FreeImage_Unload(bmp);
   tga.ensureSize();

   std::vector<uint8_t> data = gzCompress(tga.data.data(), tga.data.size());

   return new BufferFileEntry(filename, std::move(data));
}
