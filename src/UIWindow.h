#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <SDL3/SDL.h>
#include <imgui.h>
#include <vector>
#include "Kbooth.h"
#include "Camera.h"
class UIWindow {
    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
        Kbooth::Camera *camera;
        ImFont *font_regular;
        ImFont *font_countdown;
        bool opened;
        float alpha;

        // Settings
        Kbooth::Settings *settings;

        const char ** cameras;
        int cameras_size;
        int camera_index;

        const char ** formats;
        int formats_size;
        int format_index;

        // Methods
        void setStyleOptions();
        void renderSettingsWindow();
    public:
        UIWindow(SDL_Window *window, SDL_Renderer *renderer,
                 Kbooth::Settings *settings, Kbooth::Camera *camera);
        ~UIWindow();
        void processEvent(SDL_Event *event);
        void render();
};
#endif // UIWINDOW_H
