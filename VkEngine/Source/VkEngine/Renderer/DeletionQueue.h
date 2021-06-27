#pragma once

#include <deque>
#include <functional>

namespace VkEngine
{

	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void push_function(std::function<void()>&& func)
		{
			deletors.push_back(func);
		}

		void flush() {
			for (auto it = deletors.end() - 1; it > deletors.begin(); it--)
			{
				(*it)(); // call function
			}
			deletors.clear();
		}
	};
}