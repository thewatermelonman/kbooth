#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <SDL3/SDL.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <vector>
#include "Kbooth.h"
#include "Camera.h"
#include "SimpleIni.h"

using namespace Kbooth;
class UIWindow {
    private:
        ImVec4 button_color_confirm = ImVec4(0.1, 0.7, 0.35, 1.0);
        ImVec4 button_color_confirm_hover = ImVec4(0.3, 0.9, 0.45, 1.0);
        ImVec4 button_color_confirm_active = ImVec4(0.1, 1.0, 0.1, 1.0);
        ImVec4 button_color_gray = ImVec4(0.1, 0.1, 0.4, 1.0);
        ImVec4 button_color_gray_hover = ImVec4(0.3, 0.3, 0.6, 1.0);
        ImVec4 button_color_gray_active = ImVec4(0.3, 0.6, 0.7, 1.0);
        ImVec4 kbooth_primary_color = ImVec4(219 / 255.0f, 23 / 255.0f, 137 / 255.0f, 1.0f);
        ImVec4 kbooth_primary_color_hovered = ImVec4((219 + 36) / 255.0f, (23 + 60) / 255.0f, (137 + 40) / 255.0f, 1.0f);
        ImVec4 kbooth_secondary_color = ImVec4(0.1, 0.2, 0.3, 1.0f);        
        ImVec4 kbooth_secondary_color_hovered = ImVec4(0.2, 0.3, 0.4, 1.0f);
        ImVec4 kbooth_bg_color = ImVec4(0.0, 0.1, 0.2, 1.0);
        float countdown_color[4] = {kbooth_primary_color.x, kbooth_primary_color.y, kbooth_primary_color.z, 1.0};
    
        SDL_Renderer *renderer;
        SDL_Window *window;
        Camera *camera;
        ImFont *font_regular;
        ImFont *font_countdown;
        bool ui_visible; // show/hide entire ui
        bool settings_opened; // show/hide settings
        float alpha;

        // Settings
        Settings *settings;
        ImVec2 settings_window_size;
        bool settings_window_size_set;

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
        void fontSelector();
    public:
        UIWindow(SDL_Window *window, SDL_Renderer *renderer, Settings *settings,
                 Camera *camera, std::vector<UsbDevice> *usb_devices);
        ~UIWindow();
        void processEvent(SDL_Event *event);
        void render();
        bool renderStartup();
        void renderGlobalButtons();
        bool openSelectedPrinterUsbDevice(Printer *printer, CSimpleIniA *ini);
};
#endif // UIWINDOW_H
