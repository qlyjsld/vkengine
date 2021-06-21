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

		DeletionQueue _deletionQueue;

		void init();
		void release();
	};
}