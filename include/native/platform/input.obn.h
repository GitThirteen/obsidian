#pragma once
#include <external/glfw/glfw3.h>
#include <native/platform/window.obn.h>

/*class MouseHandler
{
    inline bool pressed(int button);
    inline bool released(int button);
    inline void position();
};

class KeyboardHandler
{
    inline bool pressed(int button);
    inline bool released(int button);
};*/

class EventManager
{
public:
    //MouseHandler mouse;
    //KeyboardHandler keyboard;

    EventManager(WindowManager& window) : m_window(window) { register_callbacks(); }

    void poll();

private:
    WindowManager& m_window;

    void register_callbacks();
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};