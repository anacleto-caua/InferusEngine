#pragma once

#include "Engine/Renderer/Buffer/Buffer.hpp"

struct BufferCreateDescription {
    size_t size = 0;
    BufferMemoryType memType;
    BufferUsage usage;
};

