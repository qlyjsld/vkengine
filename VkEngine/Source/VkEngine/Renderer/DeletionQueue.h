#pragma once

#include <deque>
#include <functional>

namespace VkEngine
{

	struct DeletionQueue
	{
		static std::deque<std::function<void()>> deletors;

		static void push_function(std::function<void()>&& func)
		{
			deletors.push_back(func);
		}

		static void flush() {
			for (auto it = deletors.end() - 1; it > deletors.begin(); it--)
			{
				(*it)(); // call function
			}
			deletors.clear();
		}
	};
}