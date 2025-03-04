#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <SDL3/SDL.h>
#include <imgui.h>
#include <vector>
#include "Kbooth.h"
#include "Camera.h"
#include "SimpleIni.h"

using namespace Kbooth;
class UIWindow {
    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
        Camera *camera;
        ImFont *font_regular;
        ImFont *font_countdown;
        bool opened;
        float alpha;

        // Settings
        Settings *settings;

        std::vector<UsbDevice> *printer_usb_devices;
        int printer_usb_device_index;
        bool printer_usb_device_set_as_default;

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
        UIWindow(SDL_Window *window, SDL_Renderer *renderer, Settings *settings,
                 Camera *camera, std::vector<UsbDevice> *usb_devices);
        ~UIWindow();
        void processEvent(SDL_Event *event);
        void render();
        bool renderStartup();
        bool openSelectedPrinterUsbDevice(Printer *printer, CSimpleIniA *ini);
};
#endif // UIWINDOW_H
