#ifndef GRAE_ENGINE_WINDOW_H
#define GRAE_ENGINE_WINDOW_H

#include "graphicscontext.h"

#include <string>
#include <mutex>

namespace GRAE {
class InputContext;

struct WindowProperties {
    std::string title;
    int width;
    int height;
};

class Window {
public:
    class Window_Impl;

private:
    Window_Impl *Impl;
    InputContext *input;
    WindowProperties properties;
    std::mutex lock;
public:
    Window();

    Window(GraphicsContext *g, WindowProperties &windowData);//NOTE: acquire()s the window

    ~Window();

    Window_Impl *getImpl();

    WindowProperties getProperties();

    void setTitle(std::string title);

    void release();//releases Window render context

    void acquire();//acquires Window render context

    bool closeRequested();

    void swap();

    InputContext *getInput();

    //static Window* getWindow(Window_Impl);
};
}


#endif //GRAE_ENGINE_WINDOW_H
