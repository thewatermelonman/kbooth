#ifndef IUWINDOW_H
#define IUWINDOW_H

#include <SDL3/SDL.h>
#include <imgui.h>
#include "Kbooth.h"
#include "Camera.h"
class UIWindow {
private:
    Kbooth::Settings *settings;
	bool opened;
    SDL_Renderer *renderer;
    SDL_Window *window;
    ImFont *font_regular;
    ImFont *font_title;
    void setStyleOptions();

	// Settings
	Kbooth::KB_framing framing;

	const char ** cameras;
	int cameras_size;
	int camera_index;

	const char ** formats;
	int formats_size;
	int format_index;

	Kbooth::Camera *camera;
public:
    UIWindow(SDL_Window *window, SDL_Renderer *renderer,
             Kbooth::Settings *settings, Kbooth::Camera *camera);
    ~UIWindow();
    void processEvent(SDL_Event *event);
    int render();

	// Settings
	Kbooth::KB_framing* getFraming();
	int getWebcamIndex();
	void open();
};
#endif // IUWINDOW_H
