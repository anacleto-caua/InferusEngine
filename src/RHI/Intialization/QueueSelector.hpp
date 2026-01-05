#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "RHI/RHITypes.hpp"

class QueueSelector {
public:
private:
    struct QueueScoreBoard {
        uint32_t index = -1;
        int32_t score = 0;
    };

    VkQueueFlags requiredFlags = 0;
    VkQueueFlags avoidedFlags = 0;
    // VkQueueFlags forbiddenFlags = 0;
    // VkQueueFlags desiredFlags = 0;

    std::vector<QueueContext*> uniqueAgainst;

    bool requiresSurfaceSupport = false;
    VkPhysicalDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    QueueContext* queueCtxToFit;

    std::vector<VkQueueFamilyProperties> candidateQueues;
    
public:
    void select() {
        std::vector<std::pair<uint32_t, int32_t>> validQueues;

        int32_t queueScore = -1;
        for (uint32_t i = 0;  i < candidateQueues.size(); i++) {
            queueScore = evaluateQueue(i);
            if(queueScore >= 0) {
                validQueues.push_back(std::make_pair(i, queueScore));
            }
        }

        if(validQueues.empty()) {
            throw std::runtime_error("no queue could fit the requirements!");
        }

        std::sort(validQueues.begin(), validQueues.end(), 
            [](const std::pair<uint32_t, int32_t>& a, const std::pair<uint32_t, int32_t>& b) {
                return a.second > b.second; 
            }
        );
        
        queueCtxToFit->index = validQueues.front().first;
    }

    static QueueSelector startCriteria(QueueContext* queueCtx, const std::vector<VkQueueFamilyProperties> &queueFamilies) {
        QueueSelector criteria;
        criteria.queueCtxToFit = queueCtx;
        criteria.candidateQueues = queueFamilies;
        return criteria;
    }

    static QueueSelector startCriteria(const QueueSelector& baseCriteria, QueueContext* queueCtx) {
        QueueSelector copyCriteria = baseCriteria;
        copyCriteria.queueCtxToFit = queueCtx;
        return copyCriteria;
    }

    QueueSelector& loadProperties(const std::vector<VkQueueFamilyProperties> &queueFamilies) {
        this->candidateQueues = queueFamilies;
        return *this;
    }

    QueueSelector& addRequiredFlags(VkQueueFlags flag) {
        requiredFlags |= flag;
        return *this;
    }

    QueueSelector& addAvoidedFlags(VkQueueFlags flag) {
        avoidedFlags |= flag;
        return *this;
    }

    QueueSelector& requireSurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        device = device;
        surface = surface;
        requiresSurfaceSupport = true;
        return  *this;
    }

    QueueSelector& desireExclusivenessAgainst(QueueContext* queueCtx) {
        uniqueAgainst.push_back(queueCtx);
        return *this;
    }

    QueueSelector& clearExclusiveness() {
        uniqueAgainst.clear();
        return *this;
    }

private:
    bool checkForSurfaceSupport(uint32_t index) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);
        return static_cast<bool>(presentSupport);
    }

    int32_t evaluateQueue(uint32_t index) {
        if (requiresSurfaceSupport && !checkForSurfaceSupport(index)) {
            return -1;
        }

        VkQueueFamilyProperties canditdateProperties = candidateQueues[index];
        VkQueueFlags candidateFlags = canditdateProperties.queueFlags;
        if ((candidateFlags & requiredFlags) != requiredFlags) {
            return -1;
        }

        int score = 0;
        if ((candidateFlags & avoidedFlags) == 0) {
            score += 100; 
        }

        for (QueueContext* queueCtx : uniqueAgainst) {
            if (
                (queueCtx->index != index) &&
                (queueCtx->index != -1)
            ) {
                score += 10;
            }
        }

        return score;
    }
};