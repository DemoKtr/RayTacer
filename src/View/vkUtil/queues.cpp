#include "View/vkUtil/queues.h"



    vkUtil::QueueFamilyIndices vkUtil::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool debugMode)
    {
        vkUtil::QueueFamilyIndices indices;
        // Pobieramy liczbê dostêpnych rodzin kolejek
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        if (debugMode) {
            std::cout << "There are " << queueFamilyCount << " supported queue families." << std::endl;
        }

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            // Sprawdzamy, czy rodzina kolejek obs³uguje operacje graficzne
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !indices.graphicsFamily.has_value()) {
                indices.graphicsFamily = i;
                if (debugMode) {
                    std::cout << "Queue Family " << i << " is suitable for graphics and has "
                        << queueFamily.queueCount << " queues." << std::endl;
                }
            }

            // Sprawdzamy wsparcie dla prezentacji na danym surface
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport && !indices.presentFamily.has_value()) {
                indices.presentFamily = i;
                if (debugMode) {
                    std::cout << "Queue Family " << i << " is suitable for present and has "
                        << queueFamily.queueCount << " queues." << std::endl;
                }
            }

            // Sprawdzamy, czy rodzina kolejek obs³uguje operacje obliczeniowe,
            // przy za³o¿eniu, ¿e nie jest to ju¿ grafika lub prezentacja
            if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                (!indices.graphicsFamily.has_value() || indices.graphicsFamily.value() != i) &&
                (!indices.presentFamily.has_value() || indices.presentFamily.value() != i) &&
                !indices.computeFamily.has_value()) {
                indices.computeFamily = i;
                if (debugMode) {
                    std::cout << "Queue Family " << i << " is suitable for compute and has "
                        << queueFamily.queueCount << " queues." << std::endl;
                }
            }

            // Sprawdzamy, czy rodzina kolejek obs³uguje operacje transferu,
            // przy za³o¿eniu, ¿e nie jest to ju¿ grafika, prezentacja lub compute
            if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                (!indices.graphicsFamily.has_value() || indices.graphicsFamily.value() != i) &&
                (!indices.presentFamily.has_value() || indices.presentFamily.value() != i) &&
                (!indices.computeFamily.has_value() || indices.computeFamily.value() != i) &&
                !indices.transferFamily.has_value()) {
                indices.transferFamily = i;
                if (debugMode) {
                    std::cout << "Queue Family " << i << " is suitable for transfer and has "
                        << queueFamily.queueCount << " queues." << std::endl;
                }
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

