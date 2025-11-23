#include <native/managers/event.h>

void EventManager::poll()
{
	return glfwPollEvents();
}

void EventManager::register_callbacks()
{
	glfwSetKeyCallback(this->m_window.get_window(), EventManager::key_callback);
}

void EventManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}