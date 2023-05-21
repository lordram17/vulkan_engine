#include "ivr_window.h"

IVRWindow::IVRWindow(int width, int height) :
	Width_(width),
	Height_(height)
{
}

GLFWwindow* IVRWindow::GetGLFWWindow()
{
	return Window_;
}

VkSurfaceKHR IVRWindow::GetVulkanSurface()
{
	return Surface_;
}

void IVRWindow::InitWindow()
{
	glfwInit(); //initializes the glfw library
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //glfw was originally designed to create an OpenGL context. This function tells it to not create an OpenGL context.
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporarily disable resizing
	
	Window_ = glfwCreateWindow(Width_, Height_, "Vulkan", nullptr, nullptr);

	glfwSetInputMode(Window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //disable cursor. This is for camera movement
}

void IVRWindow::CreateWindowSurface(VkInstance instance)
{
	//create vulkan surface
	if (glfwCreateWindowSurface(instance, Window_, nullptr, &Surface_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}
