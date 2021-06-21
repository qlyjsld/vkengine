#pragma once
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

	class PipelineHandler
	{
	public:

		PipelineHandler()
		{
			init();
		}

		~PipelineHandler()
		{
			release();
		};

		DeletionQueue _deletionQueue;

		void init();
		void release();
	};
}