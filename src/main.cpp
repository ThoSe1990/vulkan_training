#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "util.hpp"
#include "window.hpp"
#include "vulkan_renderer.hpp"


int main(int argc, char* argv[])
{
	cwt::window window;
	cwt::vulkan_renderer renderer(window.get_window());

	while (!glfwWindowShouldClose(window.get_window()))
	{
		glfwPollEvents();
	}
	

	return 0;
}