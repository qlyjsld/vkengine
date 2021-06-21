#include "VkEngine/Renderer/PipelineHandler.h"

namespace VkEngine
{

	void PipelineHandler::init()
	{

	}

	void PipelineHandler::release()
	{
		_deletionQueue.flush();
	}
}