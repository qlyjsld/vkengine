#include "VkEngine/Renderer/PresentHandler.h"

namespace VkEngine
{

	void PresentHandler::init()
	{

	}

	void PresentHandler::release()
	{
		_deletionQueue.flush();
	}
}