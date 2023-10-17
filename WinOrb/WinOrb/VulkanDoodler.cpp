#include "VulkanDoodler.h"
#include "File.h"
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

const int MAX_FRAMES_IN_FLIGHT = 2;

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

void VulkanDoodler::Init()
{
	Super::Init();
	CreateInstance();
	SetupMessengerCallback();
	CreateSurface();
	GetBestGraphicsDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateCommandBuffer();
	CreateSyncObjects();
}

void VulkanDoodler::CreateSurface()
{
	assert(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) == VK_SUCCESS);
}

void VulkanDoodler::CreateSwapChain()
{
	auto format = ChooseSurfaceFormat();
	auto mode = ChoosePresentMode();
	auto extent = ChooseSwapExtent();

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
	else
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

void VulkanDoodler::CreateImageViews()
{
	mImageViews.resize(mSwapImages.size());

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = mSwapFormat;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	for (size_t i = 0; i < mImageViews.size(); ++i)
	{
		createInfo.image = mSwapImages[i];
		assert(vkCreateImageView(mDevice, &createInfo, nullptr, &mImageViews[i]) == VK_SUCCESS);
	}
}

void VulkanDoodler::CreateGraphicsPipeline()
{
	auto vertCode = readfile("shaders/vert.spv");
	auto fragCode = readfile("shaders/frag.spv");

	VkShaderModule vertModule = CreateShaderModule(vertCode);
	VkShaderModule fragModule = CreateShaderModule(fragCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo assemblyInfo{};
	assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapExtent.width;
	viewport.height = (float)mSwapExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapExtent;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	//viewportInfo.pViewports = &viewport;//comment away for dynamic state
	viewportInfo.scissorCount = 1;
	//viewportInfo.pScissors = &scissor;//comment away for dynamic state

	VkPipelineRasterizationStateCreateInfo rasterInfo{};
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.depthClampEnable = VK_FALSE;
	rasterInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterInfo.lineWidth = 1.0f;
	rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterInfo.depthBiasEnable = VK_FALSE;
	rasterInfo.depthBiasConstantFactor = 0.0f;
	rasterInfo.depthBiasClamp = 0.0f;
	rasterInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo msInfo{};
	msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msInfo.sampleShadingEnable = VK_FALSE;
	msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	msInfo.minSampleShading = 1.0f;
	msInfo.pSampleMask = nullptr;
	msInfo.alphaToCoverageEnable = VK_FALSE;
	msInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlend{};
	colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlend.logicOpEnable = VK_FALSE;
	colorBlend.logicOp = VK_LOGIC_OP_COPY;
	colorBlend.attachmentCount = 1;
	colorBlend.pAttachments = &colorBlendAttachment;
	colorBlend.blendConstants[0] = 0.0f;
	colorBlend.blendConstants[1] = 0.0f;
	colorBlend.blendConstants[2] = 0.0f;
	colorBlend.blendConstants[3] = 0.0f;
	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	assert(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &assemblyInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pRasterizationState = &rasterInfo;
	pipelineInfo.pMultisampleState = &msInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlend;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	assert(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) == VK_SUCCESS);

	vkDestroyShaderModule(mDevice, vertModule, nullptr);
	vkDestroyShaderModule(mDevice, fragModule, nullptr);
}

void VulkanDoodler::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = mSwapFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	assert(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) == VK_SUCCESS);
}

void VulkanDoodler::CreateFrameBuffers()
{
	mFrameBuffers.resize(mImageViews.size());
	for (int i = 0; i < mImageViews.size(); ++i)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &mImageViews[i];
		framebufferInfo.width = mSwapExtent.width;
		framebufferInfo.height = mSwapExtent.height;
		framebufferInfo.layers = 1;

		assert(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mFrameBuffers[i]) == VK_SUCCESS);
	}
}

void VulkanDoodler::CreateCommandPool()
{
	uint32_t index;
	(void)GetQueueFamilyFromFlag(mPhysicalDevice, index);
	VkCommandPoolCreateInfo cmdpoolInfo{};
	cmdpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdpoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdpoolInfo.queueFamilyIndex = index;

	assert(vkCreateCommandPool(mDevice, &cmdpoolInfo, nullptr, &mCommandPool) == VK_SUCCESS);
}

void VulkanDoodler::CreateCommandBuffer()
{
	mCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferInfo.commandPool = mCommandPool;
	bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferInfo.commandBufferCount = (uint32_t)mCommandBuffer.size();

	assert(vkAllocateCommandBuffers(mDevice, &bufferInfo, mCommandBuffer.data()) == VK_SUCCESS);
}

void VulkanDoodler::CreateSyncObjects()
{
	mSemaphoreImageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
	mSemaphoreRenderFinish.resize(MAX_FRAMES_IN_FLIGHT);
	mFenceInFlight.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreinfo{};
	semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		assert(vkCreateSemaphore(mDevice, &semaphoreinfo, nullptr, &mSemaphoreImageAvailable[i]) == VK_SUCCESS);
		assert(vkCreateSemaphore(mDevice, &semaphoreinfo, nullptr, &mSemaphoreRenderFinish[i]) == VK_SUCCESS);
		assert(vkCreateFence(mDevice, &fenceInfo, nullptr, &mFenceInFlight[i]) == VK_SUCCESS);
	}

}

void VulkanDoodler::ReCreateSwapChain()
{
	vkDeviceWaitIdle(mDevice);
	DestroySwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateFrameBuffers();
}

void VulkanDoodler::DestroySwapChain()
{
	for (auto framebuffer : mFrameBuffers)
	{
		vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	}
	for (auto imageview : mImageViews)
	{
		vkDestroyImageView(mDevice, imageview, nullptr);
	}
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

bool VulkanDoodler::IsMinimized()
{
	int width = 0;
	int height = 0;
	glfwGetWindowSize(mWindow, &width, &height);
	return width == 0 || height == 0;
}

void VulkanDoodler::RecordCommandBuffer(VkCommandBuffer commandbuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	assert(vkBeginCommandBuffer(commandbuffer, &beginInfo) == VK_SUCCESS);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.framebuffer = mFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = mSwapExtent;
	VkClearValue clearcolor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearcolor;

	vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapExtent.width;
	viewport.height = (float)mSwapExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapExtent;
	vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
	vkCmdDraw(commandbuffer, 3, 1, 0, 0);
	
	vkCmdEndRenderPass(commandbuffer);
	assert(vkEndCommandBuffer(commandbuffer) == VK_SUCCESS);
}


VkShaderModule VulkanDoodler::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();
	VkShaderModule shaderModule;
	assert(vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) == VK_SUCCESS);
	return shaderModule;
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

VkExtent2D VulkanDoodler::ChooseSwapExtent()
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
		glfwGetFramebufferSize(mWindow, (int*)&extent.width, (int*)&extent.height); //this is the wild wild west, yeehaw

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
	vkGetDeviceQueue(mDevice, indices, 0, &mPresentQueue);
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
	Super::Update();

	if (IsMinimized())
	{
		return;
	}

	vkWaitForFences(mDevice, 1, &mFenceInFlight[mCurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult nextImageRes = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mSemaphoreImageAvailable[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
	if (nextImageRes == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ReCreateSwapChain();
		return;
	}
	else
	{
		assert(nextImageRes == VK_SUCCESS || nextImageRes == VK_SUBOPTIMAL_KHR);
	}

	vkResetFences(mDevice, 1, &mFenceInFlight[mCurrentFrame]);


	assert(vkResetCommandBuffer(mCommandBuffer[mCurrentFrame], 0) == VK_SUCCESS);
	RecordCommandBuffer(mCommandBuffer[mCurrentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { mSemaphoreImageAvailable[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer[mCurrentFrame];
	VkSemaphore signalSemaphores[] = { mSemaphoreRenderFinish[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	assert(vkQueueSubmit(mDeviceQueue, 1, &submitInfo, mFenceInFlight[mCurrentFrame]) == VK_SUCCESS);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapChain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	VkResult resPresent = vkQueuePresentKHR(mPresentQueue, &presentInfo);
	if (resPresent == VK_ERROR_OUT_OF_DATE_KHR || resPresent == VK_SUBOPTIMAL_KHR || mResize)
	{
		ReCreateSwapChain();
	}
	else
	{
		assert(resPresent == VK_SUCCESS);
	}
	++mCurrentFrame %= MAX_FRAMES_IN_FLIGHT;
}

void VulkanDoodler::Destroy()
{
	Super::Destroy();
	vkDeviceWaitIdle(mDevice);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(mDevice, mSemaphoreImageAvailable[i], nullptr);
		vkDestroySemaphore(mDevice, mSemaphoreRenderFinish[i], nullptr);
		vkDestroyFence(mDevice, mFenceInFlight[i], nullptr);
	}
	DestroySwapChain();
	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
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
