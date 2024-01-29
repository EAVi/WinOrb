#ifndef VULKAN_DOODLER_H
#define VULKAN_DOODLER_H

#define GLFW_INCLUDE_VULKAN
#include "WindowManager.h"
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include <vector>

class VulkanDoodler : virtual public WindowManager
{
public:
	typedef WindowManager Super;
	virtual void Init() override;
	virtual void Update() override;
	virtual void Destroy() override;
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
	VkCommandPool mCommandPool;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	std::vector<VkCommandBuffer> mCommandBuffer;
	std::vector<VkSemaphore> mSemaphoreImageAvailable;
	std::vector<VkSemaphore> mSemaphoreRenderFinish;
	std::vector<VkFence> mFenceInFlight;
	uint32_t mCurrentFrame = 0;
private:
	//init
	void CreateInstance();
	void SetupMessengerCallback();
	void GetBestGraphicsDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFrameBuffers();
	void CreateCommandPool();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void ReCreateSwapChain();

	//destroy
	void DestroySwapChain();

	bool IsMinimized();

	//writing/drawing
	void RecordCommandBuffer(VkCommandBuffer commandbuffer, uint32_t imageIndex);
	void CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);

	//init helpers / callbacks
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void GetSwapChainImages(std::vector<VkImage>& images);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool CheckDeviceSwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSurfaceFormat();
	VkPresentModeKHR ChoosePresentMode();
	VkExtent2D ChooseSwapExtent();
	bool GetQueueFamilyFromFlag(VkPhysicalDevice device, uint32_t& index, VkQueueFlagBits flag = VK_QUEUE_GRAPHICS_BIT);
	bool CheckValidationLayerSupported();
	int ScoreDevice(VkPhysicalDevice device);
	uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags propFlags);
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
};

#endif //! VULKAN_DOODLER_H