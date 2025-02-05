#ifndef IUWINDOW_H
#define IUWINDOW_H

#include <SDL3/SDL.h>
#include <imgui.h>
#include "Kbooth.h"
#include "Camera.h"
class UIWindow {
private:
    SDL_Renderer *renderer;
    SDL_Window *window;
	Kbooth::Camera *camera;
    ImFont *font_regular;
    ImFont *font_title;
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
public:
    UIWindow(SDL_Window *window, SDL_Renderer *renderer,
             Kbooth::Settings *settings, Kbooth::Camera *camera);
    ~UIWindow();
    void processEvent(SDL_Event *event);
    void render();
};
#endif // IUWINDOW_H
