#include "VkEngine/Renderer/PresentHandler.h"

namespace VkEngine
{

	void PresentHandler::init()
	{
		// do nothing
	}

	void PresentHandler::release()
	{
		if (_swapChainHandle != nullptr)
		{
			delete _swapChainHandle;
		}

		if (_descriptorHandle != nullptr)
		{
			delete _descriptorHandle;
		}

		if (_renderPassHandle != nullptr)
		{
			delete _renderPassHandle;
		}
	}
}