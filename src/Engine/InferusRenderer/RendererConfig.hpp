#pragma once

#define CONFIG static constexpr

#include <cstdint>

namespace RendererConfig {
    namespace BufferSystem {
        CONFIG uint32_t DATA_RESERVE_CAPACITY = 100;
        CONFIG uint32_t FREE_INDICES_RESERVE_CAPACITY = 10;
    };
    namespace ImageSystem {
        CONFIG uint32_t DATA_RESERVE_CAPACITY = 100;
        CONFIG uint32_t FREE_INDICES_RESERVE_CAPACITY = 10;
    };
};
