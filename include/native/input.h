#pragma once

class MouseHandler
{
    inline bool pressed(int button);
    inline bool released(int button);
    inline void position();
};

class KeyboardHandler
{
    inline bool pressed(int button);
    inline bool released(int button);
};

class InputManager
{
public:
    MouseHandler mouse;
    KeyboardHandler keyboard;
private:
    
};