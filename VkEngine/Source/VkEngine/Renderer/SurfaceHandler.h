#pragma once

namespace VkEngine
{

	class SurfaceHandler
	{
	public:

		SurfaceHandler()
		{
			init();
		}

		~SurfaceHandler()
		{
			release();
		};

		DeletionQueue _deletionQueue;

		void init();
		void release();
	};
}