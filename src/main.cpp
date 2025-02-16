#include <SDL3/SDL.h>
#include "SDL3/SDL_stdinc.h"
#include "SimpleIni.h"
#include "Camera.h"
#include "Kbooth.h"
#include "UIWindow.h"
#include "Printer.h"

#include <iostream>
#include <string>

using namespace Kbooth;

void EXIT_WITH_ERROR(std::string error_message);
void countdown_start();
void countdown_update(Camera *camera);
void handle_user_input(UIWindow *ui);
void load_settings_config();

bool FULLSCREEN = false;
int WINDOW_WIDTH;
int WINDOW_HEIGHT;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
Settings settings;
Printer printer;
bool window_should_close = false;

int main() {
	std::cout << "STARTING >> KBOOTH <<" << std::endl;
	load_settings_config();

	if (!printer.init(settings.printer_usb_port)) return 1;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
        EXIT_WITH_ERROR("could not initialize SDL.");
    }
    if (!SDL_CreateWindowAndRenderer("Kbooth", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        EXIT_WITH_ERROR("could not create window and renderer.");
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
    {
        Camera camera;
        if (!camera.open(0, -1)) {
        	EXIT_WITH_ERROR("Could not open Default Camera.");
        }
    	UIWindow ui = UIWindow(window, renderer, &settings, &camera);
        while (!window_should_close) {

            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 0.0, 1.0);

			if (settings.countdown.active && settings.countdown.position <= 0) {
				window_should_close = !camera.renderImageCapture(renderer, &settings.Framing, &settings.countdown);
			} else {
				window_should_close = !camera.renderCameraFeed(renderer, &settings.Framing);
			}

			if (settings.countdown.active) {
				countdown_update(&camera);
			}

			ui.render(&settings.countdown);

            SDL_RenderPresent(renderer);
			handle_user_input(&ui);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}

void load_settings_config() {
    CSimpleIniA ini;
    ini.SetUnicode();
	settings = {
		.Framing = {
			.zoom = 1.0f,
			.pos_x = 0.0f, 
			.pos_y = 0.0f, 
			.mirror = true, 
		},
		.countdown =  {
			.len = 3,
			.pace = 1500,
			.active = false,
			.position = 0,
			.start_time = 0
		},
		.Capture_Button = SDLK_SPACE, 
		.Capture_Duration = 60, 
		.output_folder = "images",
		.printer_usb_port = 7,
		.image_brightness = 40.0,
		.image_contrast = 110.0
	};
    if (ini.LoadFile("../assets/settings/config.ini") < 0) { // assuming run from build directory
 		std::cout << "NO SETTINGS FILE FOUND. RUNNING WITH DEFAULT."<< std::endl;
	} else {
		WINDOW_WIDTH = (int)ini.GetLongValue("config", "WindowWidth", 1900 / 2);
		WINDOW_HEIGHT = (int)ini.GetLongValue("config", "WindowHeight", 1080 / 2);
		settings.Framing.mirror =(bool)ini.GetBoolValue("config", "MirrorH", true, NULL);
		settings.Capture_Button = (Uint32) ini.GetLongValue("config", "CaptureButton", SDLK_SPACE);
		settings.save_images = ini.GetBoolValue("config", "SaveImage", true, NULL);
		settings.printer_usb_port = (int) ini.GetLongValue("config", "PrinterUsbPort", 7);
	}
	bool created_output_folder_dir = createDirectory(settings.output_folder.c_str());
	assert(created_output_folder_dir);
}

void handle_user_input(UIWindow *ui) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if ((event.type == SDL_EVENT_QUIT) ||
			(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
			event.window.windowID == SDL_GetWindowID(window))) {
			window_should_close = true;
			break;
		}
		ui->processEvent(&event);


		if (event.type != SDL_EVENT_KEY_DOWN) break;
		// Toggle Fullscreen
		if (event.key.key == SDLK_F) {
			FULLSCREEN = !FULLSCREEN;
			if (!SDL_SetWindowFullscreen(window, FULLSCREEN)) window_should_close = true;

		// Capture Image
		} else if(event.key.key == settings.Capture_Button && !settings.countdown.active) { 
			countdown_start();
		} else if (event.key.key == SDLK_ESCAPE) {
			window_should_close = true;
		}	
	}
}

void countdown_update(Camera *camera) {
	SDL_assert(settings.countdown.active);

	Uint64 time_to_next_num = (settings.countdown.len - settings.countdown.position + 1) * settings.countdown.pace;
	Uint64 time_curr = SDL_GetTicks() - settings.countdown.start_time;
	if (time_curr >= time_to_next_num) {
		settings.countdown.position--;
		std::cout << "  --  COUNTDOWN  --  " << settings.countdown.position << " since: " << time_curr << " > " << time_to_next_num << std::endl;
	}
	if (settings.countdown.position < -1) {
		settings.countdown.active = false;
		settings.countdown.position = settings.countdown.len;
		camera->saveAndPrintImage(&printer, settings.output_folder, settings.save_images, settings.image_brightness, settings.image_contrast);
		std::cout << "  --  COUNTDOWN  END --  " << settings.save_images << std::endl;
	}
}

void countdown_start() {
	SDL_assert(!settings.countdown.active);

	settings.countdown.position = settings.countdown.len;
	settings.countdown.start_time = SDL_GetTicks();
	settings.countdown.active = true;
	std::cout << "Started Countdown at time " << settings.countdown.start_time << "ms." << std::endl;
}

void EXIT_WITH_ERROR(std::string error_message) {
    std::cerr << "[ERROR] " << error_message << std::endl;
    exit(EXIT_FAILURE);
}
