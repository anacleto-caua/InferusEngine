#pragma once

#include <optional>
#include <string>
#include <vulkan/vulkan.h>
#include "../RHI/DeviceContext.hpp"
#include "Image.hpp"

class Texture {

public:
    Texture(
        DeviceContext &deviceCtx, 
        const std::string& filepath
    );
    
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    
    std::optional<Image> m_image;

    void generateMipmaps();
    
private:
    DeviceContext& m_deviceCtx;

    void recordGenerateMipmapsCmd(VkCommandBuffer cmd);
};