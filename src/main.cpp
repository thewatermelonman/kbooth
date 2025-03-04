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
void handle_user_input(UIWindow *ui, Camera *camera);
void handle_user_input(UIWindow *ui);
void load_settings_config();
void initializePrinter();

int window_width;
int window_height;
bool window_should_close = false;
bool fullscreen = false;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
Printer printer;
UsbDevice default_printer;
bool default_printer_configured;
Settings settings;
CSimpleIniA ini;

int main() {
	std::cout << "STARTING >> KBOOTH <<" << std::endl;
	load_settings_config();
    initializePrinter();   

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
        EXIT_WITH_ERROR("could not initialize SDL.");
    }

    if (!TTF_Init()) {
        EXIT_WITH_ERROR("Couldn't initialise SDL_ttf");
    }

    if (!SDL_CreateWindowAndRenderer("Kbooth", window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        EXIT_WITH_ERROR("could not create window and renderer.");
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
    {
        Camera camera;
        if (!camera.open(0, 0)) {
        	EXIT_WITH_ERROR("Could not open Default Camera.");
        }
        camera.setAspectRatio(renderer, settings.framing.aspect_x, settings.framing.aspect_y);
    	UIWindow ui = UIWindow(window, renderer, &settings, &camera, printer.getAvailUsbDevices());

        if (settings.print_settings.print_images && !default_printer_configured) {
            UsbDevice *printer_dev = nullptr;
            while (!window_should_close) { // Startup
                SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 0.0, 1.0);
                SDL_RenderClear(renderer);
                window_should_close = ui.renderStartup();
                SDL_RenderPresent(renderer);
                handle_user_input(&ui);
            }
            ui.openSelectedPrinterUsbDevice(&printer, &ini);
            window_should_close = false;
        }

        while (!window_should_close) { // Main Loop
            SDL_SetRenderDrawColorFloat(renderer, 0.0, 0.0, 0.0, 1.0);
            SDL_RenderClear(renderer);

            window_should_close = !camera.renderFrame(renderer, &settings);
			if (camera.updateCountdown(&settings.countdown)) {
                camera.saveAndPrintImage(&printer, &settings.print_settings);
            }
			ui.render();

            SDL_RenderPresent(renderer);
			handle_user_input(&ui, &camera);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

void load_settings_config() {
    ini.SetUnicode();
	settings = {
		.framing = {
			.zoom = 1.0f,
			.pos_x = 0.0f, 
			.pos_y = 0.0f, 
            .aspect_x = 1,
            .aspect_y = 1,
			.mirror = true, 
            .rotation = 0.0f
		},
		.countdown =  {
			.len = 3,
			.pace = 1500,
		},
        .print_settings = {
		    .save_folder = "images",
            .save_images = true,
            .print_images = true,
            .usb_port = 7,
            .brightness = 30.0,
            .contrast = 100.0,
            .landscape = true
        },
		.capture_button = SDLK_SPACE, 
	};
    if (ini.LoadFile("../assets/settings/config.ini") < 0) { // assuming run from build directory
 		std::cout << "No settings file found. Running with default."<< std::endl;
	} else {
		window_width = (int)ini.GetLongValue("config", "WindowWidth", 1900 / 2);
		window_height = (int)ini.GetLongValue("config", "WindowHeight", 1080 / 2);
        if (ini.KeyExists("config", "PrinterUsbVendorId") 
            && ini.KeyExists("config", "PrinterUsbProductId")) {
            uint16_t vendor_id = (uint16_t) ini.GetLongValue("config", "PrinterUsbVendorId", 1);
            uint16_t product_id = (uint16_t) ini.GetLongValue("config", "PrinterUsbProductId", 1);
            default_printer = {
                .vendor_id = vendor_id, 
                .product_id = product_id, 
                .description = "Default Printer"
            };
            default_printer_configured = true;
        } else {
            default_printer_configured = false;
        }
		settings.framing.mirror =(bool)ini.GetBoolValue("config", "MirrorH", true, NULL);
		settings.framing.aspect_x = (int) ini.GetLongValue("config", "aspectX", 16);
		settings.framing.aspect_y = (int) ini.GetLongValue("config", "aspectY", 9);
		settings.capture_button = (Uint32) ini.GetLongValue("config", "CaptureButton", SDLK_SPACE);
		settings.print_settings.save_images = ini.GetBoolValue("config", "SaveImages", true, NULL);
		settings.print_settings.print_images = ini.GetBoolValue("config", "PrintImages", true, NULL);
		settings.print_settings.usb_port = (int) ini.GetLongValue("config", "PrinterUsbPort", 7);
		settings.countdown.len = (int) ini.GetLongValue("config", "CountdownLen", 3);
		settings.countdown.pace = (int) ini.GetLongValue("config", "CountdownPace", 1500);

	}
	bool created_output_folder_dir = createDirectory(settings.print_settings.save_folder.c_str());
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
    }
}

void handle_user_input(UIWindow *ui, Camera *camera) {
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
			fullscreen = !fullscreen;
			if (!SDL_SetWindowFullscreen(window, fullscreen)) window_should_close = true;

		// Capture Image
		} else if(event.key.key == settings.capture_button) { 
			if (camera != nullptr) camera->startCountdown(&settings.countdown);
		} else if (event.key.key == SDLK_ESCAPE) {
			window_should_close = true;
		}	
	}
}

void initializePrinter() {
    if (!settings.print_settings.print_images) return;
    bool err;
    if (default_printer_configured) {
        err = !printer.initAndOpen(&default_printer);
        if (!err) return;

        std::cout << "Deleting Default Printer Device" << std::endl;
        ini.Delete("config", "PrinterUsbVendorId");
        ini.Delete("config", "PrinterUsbProductId");
        ini.SaveFile("../assets/settings/config.ini");
        default_printer_configured = false;
        printer.cleanup();
    }
    err = !printer.init(); 
    if (err) EXIT_WITH_ERROR("Could not initialize Printer.");
}

void EXIT_WITH_ERROR(std::string error_message) {
    std::cerr << "[ERROR] " << error_message << std::endl;
    exit(EXIT_FAILURE);
}
