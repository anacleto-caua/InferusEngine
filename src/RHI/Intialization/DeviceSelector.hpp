#pragma once

#include <vector>
#include <utility>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include "Utils/LogFormatter.hpp"

class DeviceSelector {
public:
    static VkPhysicalDevice selectPhysicalDevice(VkInstance instance, std::vector<const char*> requiredExtensions) {
        uint32_t physicalDevicesCount;
        vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
        vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices.data());

        std::vector<std::pair<int32_t, VkPhysicalDevice>> devicePickList;
        int32_t score = 0;
        for (uint32_t i = 0; i < physicalDevicesCount; i++) {
            score = evaluateDevice(physicalDevices[i], requiredExtensions);
            if (score >= 0) {
                devicePickList.push_back(std::make_pair(score, physicalDevices[i]));
            }
        }

        if(devicePickList.empty()) {
            throw std::runtime_error("no valid physical devices found");
        }

        std::sort(devicePickList.begin(), devicePickList.end(), 
            [](const std::pair<int32_t, VkPhysicalDevice> &a, const std::pair<int32_t, VkPhysicalDevice> &b) {
                return a.first > b.first;
            }
        );

        VkPhysicalDevice selectedDevice = devicePickList[0].second;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);

        spdlog::info("Chosen Device: {}", deviceProperties.deviceName);
        spdlog::info("Device ID: {}", deviceProperties.deviceID);
        spdlog::info("Device Type: {}", deviceProperties.deviceType);
        spdlog::info("Device Driver Version: {}", VkVersion{deviceProperties.driverVersion});
        spdlog::info("Device Api Version: {}", VkVersion{deviceProperties.apiVersion});

        return selectedDevice;
    }

private:
    static int32_t evaluateDevice(VkPhysicalDevice device, std::vector<const char*> &requiredExtensions) {
        int32_t deviceScore = 0;

        // Are required extensions supported
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        bool extensionFound;
        for (const char* extension : requiredExtensions) {
            extensionFound = false;
            for (const VkExtensionProperties extensionProperties : availableExtensions) {
                if(std::strcmp(extension, extensionProperties.extensionName) == 0) {
                    extensionFound = true;
                    break;
                }
            }

            if(!extensionFound) {
                return -1;
            }
        }

        // Are needed features available
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Apply preference for GPUs and higher resolution
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            deviceScore += 1000;
        } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            deviceScore += 100;
        }

        deviceScore += deviceProperties.limits.maxImageDimension2D;

        return deviceScore;
    }
};