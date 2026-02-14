#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

// Physical Device Type Formatter
template <>
struct fmt::formatter<VkPhysicalDeviceType> : fmt::formatter<std::string_view> {
    constexpr auto parse(format_parse_context& ctx) {
        return formatter<std::string_view>::parse(ctx);
    }

    auto format(VkPhysicalDeviceType type, format_context& ctx) const {
        std::string_view name = "Unknown";
        switch (type) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER: name = "Other"; break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: name = "Integrated GPU"; break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: name = "Discrete GPU"; break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: name = "Virtual GPU"; break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU: name = "CPU"; break;
            default: break;
        }
        return formatter<std::string_view>::format(name, ctx);
    }
};

// Vulkan Version Formatter
struct VkVersion { uint32_t version; };

template <>
struct fmt::formatter<VkVersion> : fmt::formatter<std::string> {
    constexpr auto parse(format_parse_context& ctx) {
        return formatter<std::string>::parse(ctx);
    }

    auto format(VkVersion v, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "{}.{}.{}",
            VK_API_VERSION_MAJOR(v.version),
            VK_API_VERSION_MINOR(v.version),
            VK_API_VERSION_PATCH(v.version));
    }
};

// Vulkan validation layers message type bitmask
//  --  Wrapper to avoid "specialization after instanciation" errors
struct VkMessageType {
    VkDebugUtilsMessageTypeFlagsEXT flags;
};
template <>
struct fmt::formatter<VkMessageType> : fmt::formatter<std::string_view> {
    auto format(VkMessageType wrapper, format_context& ctx) const {
        auto type = wrapper.flags;
        std::string name;
        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)     name += "General|";
        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  name += "Validation|";
        if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) name += "Performance|";

        if (!name.empty()) name.pop_back();
        else name = "Unknown";

        return formatter<std::string_view>::format(name, ctx);
    }
};
