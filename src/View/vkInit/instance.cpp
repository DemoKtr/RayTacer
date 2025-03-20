#include "View/vkInit/instance.h"
#include <GLFW/glfw3.h>

namespace vkInit {

	bool supported(std::vector<const char*>& requiredExtensions, std::vector<const char*>& requiredLayers, bool debug) {
        // Sprawdzenie wsparcia rozszerzeñ
        uint32_t availableExtensionCount = 0;
        VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
        if (res != VK_SUCCESS) {
            std::cerr << "error number of extensions!\n";
            return false;
        }

        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        res = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
        if (res != VK_SUCCESS) {
            std::cerr << "Error extensions recieve!\n";
            return false;
        }

        if (debug) {
            std::cout << "Device support extensions:\n";
            for (const auto& ext : availableExtensions) {
                std::cout << "\t" << ext.extensionName << "\n";
            }
        }

        // Weryfikacja wymaganych rozszerzeñ
        for (const char* reqExt : requiredExtensions) {
            bool found = false;
            for (const auto& ext : availableExtensions) {
                if (strcmp(reqExt, ext.extensionName) == 0) {
                    found = true;
                    if (debug) {
                        std::cout << "extension \"" << reqExt << "\" is supported!\n";
                    }
                    break;
                }
            }
            if (!found) {
                if (debug) {
                    std::cout << "extension \"" << reqExt << "\" is not supported!\n";
                }
                return false;
            }
        }

        // Sprawdzenie wsparcia warstw
        uint32_t availableLayerCount = 0;
        res = vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
        if (res != VK_SUCCESS) {
            std::cerr << "error number of layers!\n";
            return false;
        }

        std::vector<VkLayerProperties> availableLayers(availableLayerCount);
        res = vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());
        if (res != VK_SUCCESS) {
            std::cerr << "Error layers recieve!\n";
            return false;
        }

        if (debug) {
            std::cout << "Device support layers:\n";
            for (const auto& layer : availableLayers) {
                std::cout << "\t" << layer.layerName << "\n";
            }
        }

        // Weryfikacja wymaganych warstw
        for (const char* reqLayer : requiredLayers) {
            bool found = false;
            for (const auto& layer : availableLayers) {
                if (strcmp(reqLayer, layer.layerName) == 0) {
                    found = true;
                    if (debug) {
                        std::cout << "Layer \"" << reqLayer << "\" is supported!\n";
                    }
                    break;
                }
            }
            if (!found) {
                if (debug) {
                    std::cout << "Layer \"" << reqLayer << "\" is not supported!\n";
                }
                return false;
            }
        }

        return true;
	}
	void make_instance(VkInstance& instance,bool debugMode, const char* appName) {
        if (debugMode) {
            std::cout << "Making Instance" << std::endl;
        }

        uint32_t version{ 0 };
        vkEnumerateInstanceVersion(&version);
        if (debugMode) {
            std::cout << "System can support vulkan Variant: " << VK_API_VERSION_VARIANT(version)
                << ", Major: " << VK_API_VERSION_MAJOR(version)
                << ", Minor: " << VK_API_VERSION_MINOR(version)
                << ", Patch: " << VK_API_VERSION_PATCH(version) << '\n';
        }
        version &= ~(0xFFFU);
        version = VK_MAKE_API_VERSION(0, 1, 0, 0);
        
        VkApplicationInfo appInfo = VkApplicationInfo();
       
        appInfo.pApplicationName = appName;
        appInfo.applicationVersion = 1.0f;
        appInfo.engineVersion = 1.0f;
        appInfo.pEngineName = "RayTracer";
        appInfo.apiVersion = VK_API_VERSION_1_3;
     
        uint32_t glfwExtensionCount{ 0 };
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (debugMode) {
            extensions.push_back("VK_EXT_debug_utils");
            extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            //extensions.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
        }

        if (debugMode) {
            std::cout << "extensions to be requested:\n";

            for (const char* extensionName : extensions) {
                std::cout << "\t\"" << extensionName << "\"\n";
            }
        }

        std::vector<const char*> layers;
        layers.push_back("VK_LAYER_KHRONOS_validation");
        if (!supported(extensions, layers, debugMode)) {



            return ;
        }
        
        VkInstanceCreateInfo createInfo = VkInstanceCreateInfo();
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = VkInstanceCreateFlags();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.pApplicationInfo = &appInfo;
        
         
        
            /*
            * from vulkan_funcs.h:
            *
            * createInstance( const VULKAN_HPP_NAMESPACE::InstanceCreateInfo &          createInfo,
                    Optional<const VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr,
                    Dispatch const &                                          d = ::vk::getDispatchLoaderStatic())

            */
        VkResult result =  vkCreateInstance(&createInfo,nullptr,&instance);
       
       
          if (debugMode && result != VK_SUCCESS) {
              std::cout << "Failed to create Instance!\n";
          }
            return ;
        }
	

}

