#pragma once
#define GLFW_INCLUDE_VULKAN // GLFW automatically includes vulkan.h
#include <GLFW/glfw3.h> // GLFW loads its own definitions and automatically loads the vulkan header with it
#include <stdexcept>


class IVRWindow {

private:
	GLFWwindow* Window_;
	int Width_;
	int Height_;
	VkSurfaceKHR Surface_;

public:
	IVRWindow(int width, int height);

	GLFWwindow* GetGLFWWindow();
	VkSurfaceKHR GetVulkanSurface();

	void InitWindow();
	void CreateWindowSurface(VkInstance instance);
};