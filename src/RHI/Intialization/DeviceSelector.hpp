#pragma once

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <cstdint>
#include <cstring>
#include <vector>

#include <vulkan/vulkan.h>

class DeviceSelector {
public:
    static VkPhysicalDevice selectPhysicalDevice(VkInstance instance, std::vector<const char*> requiredExtensions) {
        uint32_t physicalDevicesCount;
        VkPhysicalDevice *physicalDevices;
        vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr);
        vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices);

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

        VkPhysicalDevice selectedDevice = devicePickList.front().second;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);
                
        std::cout << "Chosen Device: "          << deviceProperties.deviceName      << "\n";
        std::cout << "Device ID: "              << deviceProperties.deviceID        << "\n";
        std::cout << "Device Type: "            << deviceProperties.deviceType      << "\n";
        std::cout << "Device Driver Version: "  << deviceProperties.driverVersion   << "\n";
        std::cout << "Device Api Version: "     << deviceProperties.apiVersion      << "\n";

        return selectedDevice;
    }

private:
    static int32_t evaluateDevice(VkPhysicalDevice device, std::vector<const char*> requiredExtensions) {
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