#include "IO.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace IO {
    std::vector<char> binaryRead(const std::string &filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filename);
        }

        uint32_t fileSize = (uint32_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
}
