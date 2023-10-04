#ifndef VULKAN_DOODLER_H
#define VULKAN_DOODLER_H

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <vector>

class VulkanDoodler
{
public:
	void Init(GLFWwindow* window);
	void Update();
	void Destroy();
private:
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkQueue mDeviceQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapImages;
	std::vector<VkImageView> mImageViews;
	VkFormat mSwapFormat;
	VkExtent2D mSwapExtent;
	VkPresentModeKHR mSwapMode;
	VkPipelineLayout mPipelineLayout;
	VkRenderPass mRenderPass;
	VkPipeline mGraphicsPipeline;
	std::vector<VkFramebuffer> mFrameBuffers;
private:
	//init
	void CreateInstance();
	void SetupMessengerCallback();
	void GetBestGraphicsDevice();
	void CreateLogicalDevice();
	void CreateSurface(GLFWwindow* window);
	void CreateSwapChain(GLFWwindow* window);
	void CreateImageViews();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFrameBuffers();

	//init helpers / callbacks
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void GetSwapChainImages(std::vector<VkImage>& images);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool CheckDeviceSwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSurfaceFormat();
	VkPresentModeKHR ChoosePresentMode();
	VkExtent2D ChooseSwapExtent(GLFWwindow* window);
	bool GetQueueFamilyFromFlag(VkPhysicalDevice device, uint32_t& index, VkQueueFlagBits flag = VK_QUEUE_GRAPHICS_BIT);
	bool CheckValidationLayerSupported();
	int ScoreDevice(VkPhysicalDevice device);
	static VKAPI_ATTR VkBool32 VKAPI_CALL validationCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger, 
		const VkAllocationCallbacks* pAllocator);
	//figure it out
};

#endif //! VULKAN_DOODLER_H