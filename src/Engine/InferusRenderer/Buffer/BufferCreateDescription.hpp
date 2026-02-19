#pragma once

#include "Engine/InferusRenderer/Buffer/Buffer.hpp"

struct BufferCreateDescription {
    size_t size = 0;
    BufferMemoryType memType;
    BufferUsage usage;
};

