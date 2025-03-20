#include "View/vkInit/attachment.h"
#include <View/vkUtil/memory.h>
#include <cassert>


void VkInit::create_attachment(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkFormat f, VkImageUsageFlagBits u, VkExtent2D swapchainExten, VkImage& image, VkDeviceMemory& mem, VkImageView& view) {
    // Ustawienie maski aspektu (aspect mask) oraz (opcjonalnie) layoutu obrazu
    VkImageAspectFlags aspectMask = 0;
    // Mo¿esz zapisaæ layout obrazu, jeœli bêdzie potrzebny póŸniej:
    VkImageLayout imageLayout;

    if (u & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if (u & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        // Dla formatów, które zawieraj¹ tak¿e stencil
        if (f >= VK_FORMAT_D16_UNORM_S8_UINT) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // Upewnij siê, ¿e mamy ustawion¹ maskê aspektu
    assert(aspectMask != 0);

    // Tworzenie obrazu
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = f;
    imageInfo.extent.width = swapchainExten.width;
    imageInfo.extent.height = swapchainExten.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // U¿ywamy podanych flag, dodajemy jeszcze VK_IMAGE_USAGE_SAMPLED_BIT, je¿eli chcemy móc u¿ywaæ obrazu w shaderach
    imageInfo.usage = u | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = nullptr;

    VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
    assert(result == VK_SUCCESS);

    // Pobieramy wymagania pamiêci dla obrazu
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(logicalDevice, image, &req);

    // Alokujemy pamiêæ
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = vkUtil::findMemoryTypeIndex(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    result = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &mem);
    assert(result == VK_SUCCESS);

    // Powi¹zanie pamiêci z obrazem
    result = vkBindImageMemory(logicalDevice, image, mem, 0);
    assert(result == VK_SUCCESS);

    // Tworzenie widoku obrazu (Image View)
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = f;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &view);
    assert(result == VK_SUCCESS);
}
