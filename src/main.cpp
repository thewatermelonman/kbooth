#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include "SimpleIni.h"
#include "Camera.h"
#include "Kbooth.h"
#include "UIWindow.h"

void EXIT_WITH_ERROR(std::string error_message);
void handle_user_input(UIWindow *ui);

bool FULLSCREEN = false;
bool IN_IMAGE_CAPTURE = false;
int IMAGE_CAPTURE_FRAME = 0;

bool WINDOW_SHOULD_CLOSE = false;


SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
Kbooth::Settings settings;

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
	settings = {
		.Framing = {
			.zoom = 1.0f,
			.pos_x = 0.0f, 
			.pos_y = 0.0f, 
			.mirror = true, 
		},
		.Capture_Button = SDLK_SPACE, 
		.Capture_Duration = 60, 
	};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
        EXIT_WITH_ERROR("could not initialize SDL.");
    }
    if (!SDL_CreateWindowAndRenderer("Kbooth", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        EXIT_WITH_ERROR("could not create window and renderer.");
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
    {
        Kbooth::Camera camera;
        if (!camera.open(0, -1)) {
        	EXIT_WITH_ERROR("Could not open Default Camera.");
        }
    	UIWindow ui = UIWindow(window, renderer, &settings, &camera);
        while (!WINDOW_SHOULD_CLOSE) {

            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 0.0, 1.0);

			WINDOW_SHOULD_CLOSE = camera.renderFrame(renderer, window, &settings.Framing);
            ui.render();

            SDL_RenderPresent(renderer);
			handle_user_input(&ui);
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

void handle_user_input(UIWindow *ui) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if ((event.type == SDL_EVENT_QUIT) ||
			(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
			event.window.windowID == SDL_GetWindowID(window))) {
			WINDOW_SHOULD_CLOSE = true;
			break;
		}
		ui->processEvent(&event);


		if (event.type != SDL_EVENT_KEY_DOWN) break;
		// Toggle Fullscreen
		if (event.key.key == SDLK_F) {
			FULLSCREEN = !FULLSCREEN;
			if (!SDL_SetWindowFullscreen(window, FULLSCREEN)) WINDOW_SHOULD_CLOSE = true;

		// Capture Image
		} else if(event.key.key == settings.Capture_Button) { 
				IMAGE_CAPTURE_FRAME = 0;
				IN_IMAGE_CAPTURE = true;
		}
	}
}
