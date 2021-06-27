#pragma once

#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{
	
	class PresentHandler
	{
	public:

		PresentHandler()
		{
			init();
		}

		~PresentHandler()
		{
			release();
		};

		std::optional<uint32_t> graphicFamilyIndex;
		std::optional<uint32_t> presentFamilyIndex;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;

		VkSwapchainKHR _swapChain;
		std::vector<VkImage> _swapChainImages;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		std::vector<VkImageView> _swapChainImageViews;

		DeletionQueue _deletionQueue;

		void init();
		void release();
	};
}