#include "VulkanDoodler.h"
#include "GLFW/glfw3.h"
#include "glm/common.hpp"
#include <vector>
#include <cassert>

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

std::vector<const char*> GetRequiredInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanDoodler::Init(GLFWwindow* window)
{
	CreateInstance();
	SetupMessengerCallback();
	CreateSurface(window);
	GetBestGraphicsDevice();
	CreateLogicalDevice();
	CreateSwapChain(window);
}

void VulkanDoodler::CreateSurface(GLFWwindow* window)
{
	assert(glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface) == VK_SUCCESS);
}

void VulkanDoodler::CreateSwapChain(GLFWwindow* window)
{
	auto format = ChooseSurfaceFormat();
	auto mode = ChoosePresentMode();
	auto extent = ChooseSwapExtent(window);

	VkSurfaceCapabilitiesKHR cap;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &cap);

	//we want to request the minimum image count + 1, unless it exceeds the maximum
	uint32_t minImage = glm::clamp(cap.minImageCount + 1, cap.minImageCount, cap.maxImageCount);


	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.minImageCount = minImage;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t index;
	if (GetQueueFamilyFromFlag(mPhysicalDevice, index))
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	//else
	{
		assert(false && "If you're reading this, I took a shortcut and your gpu is not supported lmao");
	}
	createInfo.preTransform = cap.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = mode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain);
	assert(res == VK_SUCCESS);

	GetSwapChainImages(mSwapImages);
	mSwapFormat = format.format;
	mSwapMode = mode;
	mSwapExtent = extent;
}

void VulkanDoodler::GetSwapChainImages(std::vector<VkImage>& images)
{
	uint32_t count;
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &count, nullptr);
	images.resize(count);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &count, images.data());

}

bool VulkanDoodler::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t supportedcount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedcount, nullptr);
	std::vector<VkExtensionProperties> supported(supportedcount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedcount, supported.data());

	for (auto& extension : deviceExtensions)
	{
		bool found = false;
		for (auto& support : supported)
		{
			if (strcmp(extension, support.extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			printf("unsupported extension, prolly crash: %s\n", extension);
			return false;
		}

	}
	return true;
}

bool VulkanDoodler::CheckDeviceSwapChainSupport(VkPhysicalDevice device)
{
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formatcount;
	uint32_t modescount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatcount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatcount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatcount, formats.data());

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &modescount, nullptr);
	std::vector<VkPresentModeKHR> modes(modescount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &modescount, modes.data());

	return (formatcount != 0 && modescount != 0);
}

VkSurfaceFormatKHR VulkanDoodler::ChooseSurfaceFormat()
{
	uint32_t formatcount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatcount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatcount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatcount, formats.data());

	for (const auto& format : formats)
	{
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}
	return formats[0];
}

VkPresentModeKHR VulkanDoodler::ChoosePresentMode()
{
	uint32_t modescount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modescount, nullptr);
	std::vector<VkPresentModeKHR> modes(modescount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modescount, modes.data());
	
	for (const auto& mode : modes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanDoodler::ChooseSwapExtent(GLFWwindow* window)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);
	if (capabilities.currentExtent.width != 0xffffffff)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D extent;
		glfwGetFramebufferSize(window, (int*)&extent.width, (int*)&extent.height); //this is the wild wild west, yeehaw

		extent.width = glm::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = glm::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}
	return VkExtent2D();
}

void VulkanDoodler::CreateInstance()
{
	assert(!enableValidationLayers || CheckValidationLayerSupported());
	std::vector<const char*> glfwExtensions = GetRequiredInstanceExtensions();

	VkApplicationInfo appinfo{};
	appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pApplicationName = "WinOrb";
	appinfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appinfo.pEngineName = "Powered by Runes";
	appinfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appinfo.apiVersion = VK_API_VERSION_1_1;
	appinfo.pNext = nullptr;

	VkInstanceCreateInfo createinfo{};
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &appinfo;
	createinfo.enabledExtensionCount = (uint32_t)glfwExtensions.size();
	createinfo.ppEnabledExtensionNames = glfwExtensions.data();
	createinfo.flags = 0;
	if (enableValidationLayers)
	{
		createinfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createinfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createinfo.enabledLayerCount = 0;
		createinfo.ppEnabledLayerNames = nullptr;
	}
	createinfo.pNext = nullptr; //default initialization should set this to nullptr
	VkResult result = vkCreateInstance(&createinfo, nullptr, &mInstance);
	assert(result == VK_SUCCESS);

	uint32_t extensioncount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensioncount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensioncount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensioncount, extensions.data());

#ifndef NDEBUG
	printf("vulkan initialized with %d extensions:\n", extensioncount);
	for (uint32_t i = 0; i < extensioncount; ++i)
	{
		printf("%d, %s\n", i, extensions[i].extensionName);
	}
#endif // !NDEBUG
}

void VulkanDoodler::SetupMessengerCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createinfo{};
	createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createinfo.pfnUserCallback = validationCallback;
	createinfo.pUserData = nullptr;
	createinfo.pNext = nullptr;
	createinfo.flags = 0;

	
	assert(CreateDebugUtilsMessengerEXT(mInstance, &createinfo, nullptr, &mDebugMessenger) == VK_SUCCESS);
}

void VulkanDoodler::GetBestGraphicsDevice()
{
	mPhysicalDevice = VK_NULL_HANDLE;
	uint32_t devicecount = 0;
	vkEnumeratePhysicalDevices(mInstance, &devicecount, nullptr);
	assert(devicecount != 0);
	std::vector<VkPhysicalDevice> devices(devicecount);
	vkEnumeratePhysicalDevices(mInstance, &devicecount, devices.data());
	int bestscore = 0;
	for (auto& device : devices)
	{
		int score = ScoreDevice(device);
		if(score > bestscore)
		{
			mPhysicalDevice = device;
			bestscore = score;
		}
	}
	assert(mPhysicalDevice != VK_NULL_HANDLE);
}

void VulkanDoodler::CreateLogicalDevice()
{
	uint32_t indices;
	assert(GetQueueFamilyFromFlag(mPhysicalDevice, indices));
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices;
	queueCreateInfo.queueCount = 1;
	//queueCreateInfo.pNext = nullptr;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	VkPhysicalDeviceFeatures features{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &features;
	createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	//createInfo.pNext = nullptr;

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	assert(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) == VK_SUCCESS);

	vkGetDeviceQueue(mDevice, indices, 0, &mDeviceQueue);
}

bool VulkanDoodler::GetQueueFamilyFromFlag(VkPhysicalDevice device, uint32_t& index, VkQueueFlagBits flag)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
	for (uint32_t i = 0; i < count; ++i)
	{
		if (families[i].queueFlags & flag)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
			if (presentSupport)
			{
				//TODO do all the fancy shit where there are separate queues in case the user has a shit gpu
				index = i;
				return true; 
			}
		}
	}
	return false;
}

void VulkanDoodler::Update()
{
}

void VulkanDoodler::Destroy()
{
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr); //MAKE SURE THIS ONE COMES LAST YA SILLY
}

bool VulkanDoodler::CheckValidationLayerSupported()
{
	uint32_t layercount = 0;
	vkEnumerateInstanceLayerProperties(&layercount, nullptr);
	std::vector<VkLayerProperties> available(layercount);
	vkEnumerateInstanceLayerProperties(&layercount, available.data());

	auto checkavailable = [&](const char* name) ->bool
	{
		for (auto& layer : available)
			if (strcmp(layer.layerName, name) == 0)
				return true;
		return false;
	};

	for (auto name : validationLayers)
	{
		if(!checkavailable(name))
			return false;
	}

	return true;
}

int VulkanDoodler::ScoreDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceProperties(device, &properties);
	vkGetPhysicalDeviceFeatures(device, &features);

	if (!features.geometryShader)
		return 0;
	uint32_t familyindex = 0;
	if (!GetQueueFamilyFromFlag(device, familyindex))
	{
		return 0;
	}
	if (!CheckDeviceExtensionSupport(device))
	{
		return 0;
	}
	else if (!CheckDeviceSwapChainSupport(device))
	{
		return 0;
	}

	int score = 0;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	score += properties.limits.maxImageDimension2D;
	return score;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDoodler::validationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

VkResult VulkanDoodler::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	VkResult CreateDebugUtilsMessengerEXT();
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	return VkResult();
}

void VulkanDoodler::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
