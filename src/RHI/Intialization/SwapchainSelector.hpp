#pragma once

#include <functional>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

class SwapchainSelector {
public:
private:

    const uint32_t SCORE_BONUS_PER_MATCH = 1000;
    const uint32_t SCORE_DECREASE_PER_INDEX = 1;

    VkPhysicalDevice physicalDevice;
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkFormat> preferableFormats;
    std::vector<VkColorSpaceKHR> preferableColorSpaces;
    
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkPresentModeKHR> preferableModes;
    
public:
    static void select(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSwapchainKHR swapchain) {
        SwapchainSelector selector = SwapchainSelector();
        selector.physicalDevice = physicalDevice;
        selector.surface = surface;
        selector.swapchain = swapchain;

        selector.checkForRequiredCapabilites();
    }

    void addPreferableFormat(VkFormat format) {
        preferableFormats.push_back(format);
    }
    
    void addPreferableColorSpace(VkColorSpaceKHR colorSpace) {
        preferableColorSpaces.push_back(colorSpace);
    }

    void addPreferableMode(VkPresentModeKHR mode) {
        preferableModes.push_back(mode);
    }

private:
    void checkForRequiredCapabilites() {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        bool hasCapabilities = false;

        // Check for required capabilities
        hasCapabilities = true;
        // ...

        if (hasCapabilities) {
            throw std::runtime_error("required capabilites not found in swapchain");
        }
    }

    VkSurfaceFormatKHR pickSurfaceFormat() {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
        
        if (formatCount <= 0) {
            throw std::runtime_error("no surface format found");
        }

        std::vector<std::pair<VkSurfaceFormatKHR, uint32_t>> scoredSurfaceFormats;

        scoredSurfaceFormats.reserve(formatCount);
        std::transform(
            surfaceFormats.begin(),
            surfaceFormats.end(),
            std::back_inserter(scoredSurfaceFormats),
            [](const VkSurfaceFormatKHR& format) -> std::pair<VkSurfaceFormatKHR, uint32_t> {
                return { format, 0 };
            }
        );

        std::function<void(VkFormat, uint32_t)> scoreByFormat =
        [&](VkFormat targetFormat, uint32_t prefFormatIndex) {
            for (std::pair<VkSurfaceFormatKHR, uint32_t> &surfFormatPair : scoredSurfaceFormats) {
                if (surfFormatPair.first.format == targetFormat) {
                    surfFormatPair.second += SCORE_BONUS_PER_MATCH - (SCORE_DECREASE_PER_INDEX * prefFormatIndex);
                }
            }
        };

        std::function<void(VkColorSpaceKHR, uint32_t)> scoreByColorSpace =
        [&](VkColorSpaceKHR targetColorSpace, uint32_t prefColorSpaceIndex) {
            for (std::pair<VkSurfaceFormatKHR, uint32_t> &surfFormatPair : scoredSurfaceFormats) {
                if (surfFormatPair.first.colorSpace == targetColorSpace) {
                    surfFormatPair.second += SCORE_BONUS_PER_MATCH - (SCORE_DECREASE_PER_INDEX * prefColorSpaceIndex);
                }
            }
        };

        for (uint32_t i = 0; i < preferableFormats.size(); i++) {
            VkFormat targetFormat = preferableFormats[i];
            scoreByFormat(targetFormat, i);
        }
        
        for (uint32_t i = 0; i < preferableColorSpaces.size(); i++) {
            VkColorSpaceKHR targetColorSpace = preferableColorSpaces[i];
            scoreByColorSpace(targetColorSpace, i);
        }

        std::sort(
            scoredSurfaceFormats.begin(), scoredSurfaceFormats.end(),
            [](const std::pair<VkSurfaceFormatKHR, uint32_t>& a, 
            const std::pair<VkSurfaceFormatKHR, uint32_t>& b) -> bool {
                return a.second > b.second; 
            }
        );

        return scoredSurfaceFormats[0].first;
    }

    VkPresentModeKHR pickPresentationMode() {
        uint32_t modeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, nullptr);
        presentModes.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, presentModes.data());

        if (modeCount <= 0) {
            throw std::runtime_error("no present mode found");
        }

        for (const VkPresentModeKHR &prefFormat : preferableModes) {
            for (const VkPresentModeKHR &avaliableFormat : presentModes) {
                if ( prefFormat == avaliableFormat ) {
                    return prefFormat;
                }
            }
        }

        return presentModes[0];
    }

};