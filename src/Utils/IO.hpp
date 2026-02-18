#pragma once

#include <vector>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace IO {
    static inline void BinaryRead(const std::string &Filename, std::vector<char> &Buffer, uint32_t &ShaderSize) {
        std::ifstream File(Filename, std::ios::ate | std::ios::binary);

        if (!File.is_open()) {
            throw std::runtime_error("Failed to open file: " + Filename);
        }

        ShaderSize = (uint32_t)File.tellg();
        Buffer.reserve(ShaderSize);

        File.seekg(0);
        File.read(Buffer.data(), ShaderSize);
        File.close();
    }
}
