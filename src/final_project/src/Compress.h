#pragma once

#include <iostream>
#include <string>
#include <cstdint>

class LZMA {
public:
    static void decompressStream(std::istream& input, std::ostream& output);
    static void compressStream(std::istream& input, std::ostream& output, uint32_t compressionLevel = 6);
};
