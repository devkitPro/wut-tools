#include "utils.h"
#include "rplwrap.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <functional>
#include <fstream>
#include <locale>
#include <vector>
#include <string>
#include <zlib.h>

enum class ReadMode
{
   INVALID,
   TEXT,
   TEXT_WRAP,
   DATA,
   DATA_WRAP,
};

void
writeExports(std::ofstream &out,
             const std::string &moduleName,
             bool isData,
             const std::vector<std::string> &exports)
{
   if (isData) {
      out << ".section .dimport_" << moduleName << ", \"a\", @0x80000002" << std::endl;
   } else {
      out << ".section .fimport_" << moduleName << ", \"ax\", @0x80000002" << std::endl;
   }

   out << ".align 4" << std::endl;
   out << std::endl;

   // Usually the symbol count, but isn't checked on hardware.
   // Spoofed to allow ld to garbage-collect later.
   out << ".long 1" << std::endl;
   // Supposed to be a crc32 of the imports. Again, not actually checked.
   out << ".long 0x00000000" << std::endl;
   out << std::endl;

   // Align module name up to 8 bytes
   auto moduleNameSize = (moduleName.length() + 1 + 7) & ~7;

   // Setup name data
   std::vector<uint32_t> secData;
   secData.resize(moduleNameSize / 4, 0);
   memcpy(secData.data(), moduleName.c_str(), moduleName.length());

   // Add name data
   for (uint32_t data : secData) {
      out << ".long 0x" << std::hex << byte_swap(data) << std::endl;
   }
   out << std::endl;

   const char *type = isData ? "@object" : "@function";

   for (auto i = 0; i < exports.size(); ++i) {
      if (i < exports.size()) {
         // Basically do -ffunction-sections
         if (isData) {
            out << ".section .dimport_" << moduleName << "." << exports[i] << ", \"a\", @0x80000002" << std::endl;
         } else {
            out << ".section .fimport_" << moduleName << "." << exports[i] << ", \"ax\", @0x80000002" << std::endl;
         }
         out << ".global " << exports[i] << std::endl;
         out << ".type " << exports[i] << ", " << type << std::endl;
         out << exports[i] << ":" << std::endl;
      }
      out << ".long 0x0" << std::endl;
      out << ".long 0x0" << std::endl;
      out << std::endl;
   }
}

int main(int argc, char **argv)
{
   std::string moduleName;
   std::vector<std::string> funcExports, dataExports;
   ReadMode readMode = ReadMode::INVALID;

   if (argc < 3) {
      std::cout << argv[0] << " <exports.def> <output.S>" << std::endl;
      return 0;
   }

   {
      std::ifstream in;
      in.open(argv[1]);

      if (!in.is_open()) {
         std::cout << "Could not open file " << argv[1] << " for reading" << std::endl;
         return -1;
      }

      std::string line;
      while (std::getline(in, line)) {
         // Trim comments
         std::size_t commentOffset = line.find("//");
         if (commentOffset != std::string::npos) {
            line = line.substr(0, commentOffset);
         }

         // Trim whitespace
         line = trim(line);

         // Skip blank lines
         if (line.length() == 0) {
            continue;
         }

         // Look for section headers
         if (line[0] == ':') {
            if (line.substr(1) == "TEXT") {
               readMode = ReadMode::TEXT;
            } else if (line.substr(1) == "TEXT_WRAP") {
               readMode = ReadMode::TEXT_WRAP;
            } else if (line.substr(1) == "DATA") {
               readMode = ReadMode::DATA;
            } else if (line.substr(1) == "DATA_WRAP") {
               readMode = ReadMode::DATA_WRAP;
            } else if (line.substr(1, 4) == "NAME") {
               moduleName = line.substr(6);
            } else {
               std::cout << "Unexpected section type" << std::endl;
               return -1;
            }
            continue;
         }

         if (readMode == ReadMode::TEXT) {
            funcExports.push_back(line);
         } else if (readMode == ReadMode::TEXT_WRAP) {
            funcExports.push_back(std::string(RPLWRAP_PREFIX) + line);
         } else if (readMode == ReadMode::DATA) {
            dataExports.push_back(line);
         } else if (readMode == ReadMode::DATA_WRAP) {
            dataExports.push_back(std::string(RPLWRAP_PREFIX) + line);
         } else {
            std::cout << "Unexpected section data" << std::endl;
            return -1;
         }
      }
   }

   {
      std::ofstream out;
      out.open(argv[2]);

      if (!out.is_open()) {
         std::cout << "Could not open file " << argv[2] << " for writing" << std::endl;
         return -1;
      }

      if (funcExports.size() > 0) {
         writeExports(out, moduleName, false, funcExports);
      }

      if (dataExports.size() > 0) {
         writeExports(out, moduleName, true, dataExports);
      }
   }

   return 0;
}
