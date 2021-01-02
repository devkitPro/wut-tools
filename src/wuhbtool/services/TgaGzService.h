#pragma once
#include <cstdint>
#include <FreeImage.h>

#include "../entities/FileEntry.h"

FileEntry* createTgaGzFileEntry(const char* inputFile, int width, int height, const char* filename);
