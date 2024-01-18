#ifndef WINORB_VERTEX_H
#define WINORB_VERTEX_H

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#include <vector>

struct Vertex2
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription description;
		description.stride = sizeof(Vertex2);
		description.binding = 0;
		description.inputRate =VK_VERTEX_INPUT_RATE_VERTEX;
		return description;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> description;

		VkVertexInputAttributeDescription pos{};
		pos.location = 0;
		pos.binding = 0;
		pos.format = VK_FORMAT_R32G32_SFLOAT;
		pos.offset = offsetof(Vertex2, pos);

		VkVertexInputAttributeDescription color{};
		color.location = 1;
		color.binding = 0;
		color.format = VK_FORMAT_R32G32B32_SFLOAT;
		color.offset = offsetof(Vertex2, color);

		description.push_back(pos);
		description.push_back(color);

		return description;
	}
};

struct Vertex3
{
	glm::vec3 pos;
	glm::vec3 color;
};

#endif