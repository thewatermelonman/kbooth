#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include "SimpleIni.h"
#include "Camera.h"
#include "Kbooth.h"
#include "UIWindow.h"

using namespace Kbooth;
void EXIT_WITH_ERROR(std::string error_message);

bool FULLSCREEN = false;

int main() {
    std::cout << "config" << std::endl;
    CSimpleIniA ini;
    ini.SetUnicode();
	int window_width = 1900;
	int window_height = 1080;
    if (ini.LoadFile("assets/settings/config.ini") < 0) {
 		std::cout << "NO SETTINGS FILE FOUND. RUNNING WITH DEFAULT."<< std::endl;
	} else {
		std::cout << ini.GetValue("config", "PrintSize", "a6") << std::endl;
		window_width = (int)ini.GetLongValue("config", "CameraWidth", 1900);
		window_height = (int)ini.GetLongValue("config", "CameraHeight", 1080);
	}
    SDL_Window *window;
    SDL_Renderer *renderer = NULL;
    Kbooth::Settings *settings = {0};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
        EXIT_WITH_ERROR("could not initialize SDL.");
    }
    if (!SDL_CreateWindowAndRenderer("Kbooth", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        EXIT_WITH_ERROR("could not create window and renderer.");
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
    bool window_should_close = false;
    {
        Kbooth::Camera camera;
        if (!camera.open(0, -1)) {
            window_should_close = true;
        }
    	UIWindow ui = UIWindow(window, renderer, settings, &camera);
        while (!window_should_close) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if ((event.type == SDL_EVENT_QUIT) ||
                    (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                     event.window.windowID == SDL_GetWindowID(window))) {
                    window_should_close = true;
                    break;
                }
                ui.processEvent(&event);
				if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F) {
					FULLSCREEN = !FULLSCREEN;
					if (!SDL_SetWindowFullscreen(window, FULLSCREEN)) window_should_close = true;
				}
            }
            if (window_should_close) break;
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColorFloat(renderer, 0.3, 0.3, 0.3, 1.0);
			camera.renderFrame(renderer, window, ui.getFraming(), &window_should_close);
            ui.render();
            SDL_RenderPresent(renderer);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}

void EXIT_WITH_ERROR(std::string error_message) {
    std::cerr << "[ERROR] " << error_message << std::endl;
    exit(EXIT_FAILURE);
}
