#ifndef IUWINDOW_H
#define IUWINDOW_H

#include <SDL3/SDL.h>
#include <imgui.h>
#include "Kbooth.h"
class UIWindow {
private:
    Kbooth::Settings *settings;
    SDL_Renderer *renderer;
    SDL_Window *window;
    ImFont *font_regular;
    ImFont *font_title;
    void setStyleOptions();

public:
    UIWindow(SDL_Window *window, SDL_Renderer *renderer,
             Kbooth::Settings *settings);
    ~UIWindow();
    void processEvent(SDL_Event *event);
    int render();
};
#endif // IUWINDOW_H