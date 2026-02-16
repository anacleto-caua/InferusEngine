#pragma once

#include <cstddef>

namespace RHIConfig {
    namespace ImageSystemConfig {
        constexpr size_t ImageReserveCapacity = 100;
        constexpr size_t FreeIndicesReserveCapacity = 100;
    }

    namespace BufferManagerConfig {
        constexpr size_t BufferReserveCapacity = 100;
        constexpr size_t FreeIndicesReserveCapacity = 10;
    }
}
