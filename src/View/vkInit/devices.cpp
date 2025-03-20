#include "View/vkInit/devices.h"
#include <View/vkInit/logging.h>
#include "View/vkUtil/queues.h"
#include <set>
void vkInit::choose_physical_device(VkInstance& instance, VkPhysicalDevice& physicalDevice, bool debugMode) {
    // Pobranie liczby dost�pnych urz�dze�
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0) {
        if (debugMode) {
            std::cerr << "Failed to find GPUs with Vulkan support!" << std::endl;
        }
        return ;
    }

    // Pobranie listy urz�dze�
    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    if (result != VK_SUCCESS) {
        if (debugMode) {
            std::cerr << "Failed to enumerate physical devices!" << std::endl;
        }
        return ;
    }

    if (debugMode) {
        std::cout << "Found " << deviceCount << " physical device(s) available." << std::endl;
    }

    // Iteracja po dost�pnych urz�dzeniach
    for (const auto& device : devices) {
        if (debugMode) {
            log_device_properties(device);
        }
        if (isSuitable(device, debugMode)) {
            physicalDevice = device;
        }
    }

    // Je�li �adne urz�dzenie nie spe�nia kryteri�w
    
}

bool vkInit::isSuitable(const VkPhysicalDevice& physicalDevice, bool debugMode) {
	if (debugMode) {
		std::cout << "Checking if device is suitable\n";
	}

	/*
	* A device is suitable if it can present to the screen, ie support
	* the swapchain extension
	*/
	const std::vector<const char*> requestedExtensions = {
		{VK_KHR_SWAPCHAIN_EXTENSION_NAME},  {VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME},
		VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME ,
		VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
		VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,

	};

	if (debugMode) {
		std::cout << "We are requesting device extensions:\n";

		for (const char* extension : requestedExtensions) {
			std::cout << "\t\"" << extension << "\"\n";
		}

	}

	if (bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice, requestedExtensions, debugMode)) {

		if (debugMode) {
			std::cout << "Device can support the requested extensions!\n";
		}
	}
	else {

		if (debugMode) {
			std::cout << "Device can't support the requested extensions!\n";
		}

		return false;
	}
	return true;
}

bool vkInit::checkDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice, const std::vector<const char*>& requestedExtensions, const bool debug) {
	/*
	* Check if a given physical device can satisfy a list of requested device
	* extensions.
	*/

	std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    uint32_t extensionCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS) {
        if (debug) {
            std::cerr << "Nie uda�o si� pobra� liczby rozszerze� urz�dzenia!\n";
        }
        return false;
    }

    // Pobieramy w�a�ciwo�ci rozszerze�
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    if (result != VK_SUCCESS) {
        if (debug) {
            std::cerr << "Nie uda�o si� pobra� w�a�ciwo�ci rozszerze� urz�dzenia!\n";
        }
        return false;
    }

    // Przegl�damy dost�pne rozszerzenia
    for (const auto& extension : availableExtensions) {
        if (debug) {
            std::cout << "\t\"" << extension.extensionName << "\"\n";
        }
        // Usuwamy znalezione rozszerzenie z listy wymaganych
        requiredExtensions.erase(extension.extensionName);
    }

    // Je�li zbi�r wymaganych rozszerze� jest pusty, to urz�dzenie wspiera wszystkie wymagane rozszerzenia.
    return requiredExtensions.empty();
}

void vkInit::create_logical_device(VkPhysicalDevice& physicalDevice, VkDevice& device,VkSurfaceKHR& surface, bool debugMode) {
    // Znajd� rodziny kolejek
    vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);

    // Utw�rz list� unikalnych indeks�w rodzin kolejek
    std::vector<uint32_t> uniqueIndices;
    if (indices.graphicsFamily.has_value())
        uniqueIndices.push_back(indices.graphicsFamily.value());
    if (indices.computeFamily.has_value())
        uniqueIndices.push_back(indices.computeFamily.value());
    if (indices.transferFamily.has_value())
        uniqueIndices.push_back(indices.transferFamily.value());
    // Je�li potrzebujesz r�wnie� rodziny prezentacji, mo�esz j� doda�, je�li nie jest r�wna grafice lub compute.
    // Przyk�ad:
    // if (indices.presentFamily.has_value() &&
    //     indices.presentFamily.value() != indices.graphicsFamily.value())
    //     uniqueIndices.push_back(indices.presentFamily.value());

    // Pobierz w�a�ciwo�ci rodzin kolejek � potrzebne do okre�lenia liczby kolejek w rodzinie
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

    // Przygotuj struktur� VkDeviceQueueCreateInfo dla ka�dej unikalnej rodziny kolejek
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // Priorytety kolejek � dla uproszczenia zak�adamy, �e maksymalnie potrzebujemy 2 priorytet�w
    std::vector<float> defaultPriorities = { 1.0f, 1.0f };
    for (uint32_t queueFamilyIndex : uniqueIndices) {
        // Je�li dost�pnych kolejek w rodzinie jest wi�cej ni� 1, wybieramy 2, w przeciwnym razie 1
        uint32_t queueCount = (families[queueFamilyIndex].queueCount > 1) ? 2 : 1;
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = queueCount;
        queueCreateInfo.pQueuePriorities = defaultPriorities.data();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Lista rozszerze� urz�dzenia
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
        //VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME, // zakomentowane, je�li niepotrzebne
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
        VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
        VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
        VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME,
        VK_KHR_SHADER_SUBGROUP_ROTATE_EXTENSION_NAME,
    };

    // Ustawienia funkcji urz�dzenia (chain struktur z funkcjami rozszerze�)
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.taskShader = VK_TRUE;
    // meshShaderFeatures.pNext = nullptr; // domy�lnie

    VkPhysicalDeviceMaintenance4FeaturesKHR maintenance4Features = {};
    maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR;
    maintenance4Features.maintenance4 = VK_TRUE;
    maintenance4Features.pNext = &meshShaderFeatures;

    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferDeviceAddressFeatures = {};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
    bufferDeviceAddressFeatures.pNext = &maintenance4Features;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    dynamicRenderingFeatures.pNext = &bufferDeviceAddressFeatures;

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &dynamicRenderingFeatures;
    deviceFeatures2.features = deviceFeatures;

    // Warstwy walidacyjne
    std::vector<const char*> enabledLayers;
    if (debugMode) {
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        enabledLayers.push_back("VK_LAYER_RENDERDOC_Capture");
    }

    // Budujemy struktur� VkDeviceCreateInfo
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.empty() ? nullptr : enabledLayers.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();
    // U�ywamy �a�cucha struktur features2 zamiast bezpo�rednio VkPhysicalDeviceFeatures:
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &deviceFeatures2;

    
    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        if (debugMode) {
            std::cout << "Device creation FAILED!!!" << std::endl;
        }
        return ;
    }
    if (debugMode) {
        std::cout << "Device is successfully created" << std::endl;
    }
    
    return ;
}
void vkInit::get_Queues(VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue queues[4], VkSurfaceKHR& surface, bool debugMode)
{
	vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);
    // Sprawdzamy i pobieramy kolejk� graficzn�
    if (indices.graphicsFamily.has_value()) {
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &queues[0]);
        if (debugMode) {
            std::cout << "Graphics queue obtained from family " << indices.graphicsFamily.value() << std::endl;
        }
    }
    else if (debugMode) {
        std::cerr << "No graphics queue family found!" << std::endl;
    }

    // Sprawdzamy i pobieramy kolejk� prezentacji
    if (indices.presentFamily.has_value()) {
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &queues[1]);
        if (debugMode) {
            std::cout << "Present queue obtained from family " << indices.presentFamily.value() << std::endl;
        }
    }
    else if (debugMode) {
        std::cerr << "No present queue family found!" << std::endl;
    }

    // Sprawdzamy i pobieramy kolejk� obliczeniow� (compute)
    if (indices.computeFamily.has_value()) {
        vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &queues[2]);
        if (debugMode) {
            std::cout << "Compute queue obtained from family " << indices.computeFamily.value() << std::endl;
        }
    }
    else if (debugMode) {
        std::cerr << "No compute queue family found!" << std::endl;
    }

    // Sprawdzamy i pobieramy kolejk� transferu
    if (indices.transferFamily.has_value()) {
        vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &queues[3]);
        if (debugMode) {
            std::cout << "Transfer queue obtained from family " << indices.transferFamily.value() << std::endl;
        }
    }
    else if (debugMode) {
        std::cerr << "No transfer queue family found!" << std::endl;
    }

} 
	
	


