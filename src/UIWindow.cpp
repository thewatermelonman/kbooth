#include "UIWindow.h"

#include "Kbooth.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <cmath>
#include <vector>
#include <iostream>
#include "SimpleIni.h"
#include <filesystem>

namespace fs = std::filesystem;

using namespace Kbooth;

void free_formats(const char **formats, int size);

UIWindow::UIWindow(SDL_Window *window, SDL_Renderer *renderer, Settings *settings,
                   Camera *camera, std::vector<UsbDevice> *usb_devices)
    : renderer(renderer), settings(settings), window(window), camera(camera), printer_usb_devices(usb_devices) {

	//get available cameras
	cameras = this->camera->getAvailCameraNames(&cameras_size);
	camera_index = 0;
    camera->setFontColor(countdown_color);

	//get available formats for the first (default) camera
	formats = this->camera->getAvailFormatNames(camera_index, &formats_size);
	format_index = 0;

	// settings window
	settings_opened = false;
	ui_visible = true;
	alpha = 0.96;


    printer_usb_device_index = 0;
    printer_usb_device_set_as_default = false;

    if (settings->optimize_rasp_pi) {
        std::cout << "[Raspberry Pi Optimization] Running with Font Oversampling = 1" << std::endl;
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 1;
        fontConfig.OversampleV = 1;
    } else {
        std::cout << "[No Raspberry Pi Optimization!] Running with default Font." << std::endl;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Set custom Kbooth Dear ImGui style
    setStyleOptions();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    io.Fonts->AddFontDefault();
	std::string font_file = "./assets/fonts/font1.ttf";	
    ImFileHandle f;
    if ((f = ImFileOpen(font_file.c_str(), "rb")) == NULL) {
		font_file = "." + font_file;
	} else {
		ImFileClose(f);
	}
    font_regular = io.Fonts->AddFontFromFileTTF(font_file.c_str(), 22.0f);
    IM_ASSERT(font_regular != nullptr);

    settings_window_size = ImGui::GetIO().DisplaySize * ImVec2(0.7f, 0.7f);
}

UIWindow::~UIWindow() {
	free_formats(formats, formats_size);
	delete[] cameras;
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void UIWindow::processEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);

	if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_H) {
		ui_visible = !ui_visible;
        if (ui_visible) settings_opened = false;
        alpha = 0.96;
	}
}

void UIWindow::render() {
    if (!ui_visible) return;
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = alpha;
    if (settings_opened) renderSettingsWindow();
    style.Alpha = 1.0f;
    if (ui_visible) renderGlobalButtons();

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}

bool UIWindow::openSelectedPrinterUsbDevice(Printer *printer, CSimpleIni *ini) {
    UsbDevice& printer_device = printer_usb_devices->at(printer_usb_device_index);
    bool res = printer->open(printer_device);
    if (printer_usb_device_set_as_default && res) {
        std::cout << "setting defaut printer usb device: ";

        std::cout << "\tconfig: " << "PrinterUsbVendorId" << (long) printer_device.vendor_id << std::endl;
        std::cout << "\tconfig: " << "PrinterUsbProductId" << (long) printer_device.product_id << std::endl;

        ini->SetLongValue("config", "PrinterUsbVendorId", (long) printer_device.vendor_id);
        ini->SetLongValue("config", "PrinterUsbProductId", (long) printer_device.product_id);
        ini->SaveFile("../assets/settings/config.ini");

    }
    return res;
}

bool ButtonCircle(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavCursor(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, size.x / 2.0f);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

void UIWindow::fontSelector() {
    static std::string directory = "../assets/fonts/";
    static std::vector<std::string> fontFiles;
    static int selectedItem = 0;

    // Populate fontFiles only once
    if (fontFiles.empty()) {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ttf") {
                fontFiles.push_back(entry.path().filename().string());
            }
        }
    }
    if (ImGui::BeginCombo("Select Font", fontFiles.empty() ? "No Fonts Found" : fontFiles[selectedItem].c_str())) {
        for (size_t i = 0; i < fontFiles.size(); ++i) {
            bool isSelected = (selectedItem == static_cast<int>(i));
            if (ImGui::Selectable(fontFiles[i].c_str(), isSelected)) {
                selectedItem = static_cast<int>(i);
                camera->setFont(fontFiles[i].c_str());
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            
        }
        ImGui::EndCombo();
    }
    if (ImGui::ColorEdit4("Countdown Color", countdown_color)) {
        camera->setFontColor(countdown_color);
    }
}

void UIWindow::renderGlobalButtons() {
	ImGui::PushFont(font_regular);
    // Settings Button
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    ImGui::Begin("Settings_Button", NULL, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize);
    if (ImGui::Button("Settings")) {
        settings_opened = !settings_opened;
        alpha = 0.96;
        settings_window_size = ImGui::GetIO().DisplaySize * ImVec2(0.7f, 0.7f);
    }
    ImGui::End();

    // Take Picture Button
    ImVec2 display_size = ImGui::GetIO().DisplaySize;
    ImVec2 button_size = ImVec2(display_size.y / 16.0f, display_size.y / 16.0f);
    ImGui::SetNextWindowSize(ImVec2(button_size.x * 2, button_size.x * 2));
    ImGui::SetNextWindowPos(ImVec2((display_size.x - button_size.x) / 2.0f, display_size.y - button_size.x - 50.0f));
    ImGui::Begin("Take Pic Button", NULL, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 1.0, 1.0, 0.7));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.6, 0.6, 0.7));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0, 1.0, 1.0, 1.0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, button_size.x / 2.0f);
    if (ButtonCircle(" ", button_size, ImGuiButtonFlags_None)) {
        camera->startCountdown(&settings->countdown);
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::End();
    ImGui::PopFont();
}


bool UIWindow::renderStartup() {

    bool output = false;
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImVec2 display_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(display_size);
	ImGui::PushFont(font_regular);
    ImGui::Begin("Kbooth - Select Printer", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    ImGui::Text("Select the Usb Device that is a valid ESC/POS Printer!");
    if (printer_usb_devices == nullptr) {
        ImGui::Text("Could not load available printer_usb_devices. Please close the Program!");
        output = false;
    } else {
        if (ImGui::BeginListBox("Select Printer"))
        {
            for (int n = 0; n < printer_usb_devices->size(); n++)
            {
                const bool is_selected = (printer_usb_device_index == n);
                std::string curr_desc = std::to_string(n) + ". " + printer_usb_devices->at(n).description;
                if (ImGui::Selectable(curr_desc.c_str(), is_selected)) {
                    printer_usb_device_index = n;
                }

                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::Checkbox("Save as Default", &printer_usb_device_set_as_default);
        if (ImGui::Button("Continue without printer")){
            settings->print_settings.print_images = false;
            output = true;
        } 
        ImVec2 buttons_size = ImGui::CalcTextSize("Continue without printer  Confirm");
        if (buttons_size.x < display_size.x * 0.7f) ImGui::SameLine(0.0f, (display_size.x * 0.7f) - buttons_size.x);

        ImGui::PushStyleColor(ImGuiCol_Button, button_color_confirm);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_color_confirm_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_color_confirm_active);
        ImGui::PushStyleColor(ImGuiCol_Border, button_color_confirm);
        if (ImGui::Button("Confirm")) {
            output = true;
        }
        ImGui::PopStyleColor(4);
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    return output;
}

void UIWindow::renderSettingsWindow() {
	ImGui::PushFont(font_regular);
    ImGui::SetWindowSize(settings_window_size);
	ImGui::Begin("Kbooth  |><|  Settings", &settings_opened);
    
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Genral - IO")) {

            bool old_camera_index = camera_index;
            bool change = ImGui::Combo("Webcam", &camera_index, cameras, cameras_size);
            if (change && old_camera_index != camera_index) {
                camera->open(camera_index, format_index);
                camera->setAspectRatio(renderer, settings->framing.aspect_x, settings->framing.aspect_y);
                settings->framing.zoom = 1.0;
                settings->framing.pos_x = 0.0;
                settings->framing.pos_x = 0.0;
                settings->framing.mirror = true;
                format_index = -1;
                free_formats(formats, formats_size);
                formats = camera->getAvailFormatNames(camera_index, &formats_size);
            }

            bool old_format_index = format_index;
            change = ImGui::Combo("Webcam Format", &format_index, formats, formats_size);
            if (change && old_format_index != format_index) {
                camera->open(camera_index, format_index);
                settings->framing.zoom = 1.0;
                settings->framing.pos_x = 0.0;
                settings->framing.pos_x = 0.0;
                settings->framing.mirror = true;
            }
            ImGui::EndTabItem();
        }
        ImVec2 width = ImGui::GetContentRegionAvail();
        if (ImGui::BeginTabItem("Image Framing")) {

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 3.0f - ImGui::CalcTextSize("X ").x);
            ImGui::SliderInt("x", &settings->framing.aspect_x, 1, 26);
            ImGui::SameLine();
            ImGui::SliderInt(" ", &settings->framing.aspect_y, 1, 16);
            ImGui::SameLine();
            ImGui::PopItemWidth();
            ImGui::Text("Aspect Ratio");

            ImGui::SliderFloat("Zoom", &settings->framing.zoom, 0.5f, 2.0f, "%.2f X");
            ImGui::SliderFloat("Rotation", &settings->framing.rotation, 0.0f, 360.0f, "%.1f");
            if (settings->framing.zoom == 1.0) {
                settings->framing.pos_x = 0.0;
                settings->framing.pos_y = 0.0;
                ImGui::BeginDisabled();
            }

            ImGui::PushItemWidth(width.x / 4.0f);

            ImGui::BeginGroup();

            ImGui::SliderFloat("X-Pos", &settings->framing.pos_x, -1.0f, 1.00f, "L\tR");

            if (settings->framing.zoom == 1.0) ImGui::EndDisabled();
            ImGui::Checkbox("Mirror", &settings->framing.mirror);
            if (settings->framing.zoom == 1.0) ImGui::BeginDisabled();

            ImGui::EndGroup();

            ImGui::PopItemWidth();
            ImGui::SameLine(0.0, width.x / 16.0f);
            const ImVec2 slider_size(width.x / 10.0f, 100.0);
            ImGui::VSliderFloat("Y-Pos", slider_size, &settings->framing.pos_y, 1.0f, -1.0f, "U\n \nD");
            if (settings->framing.zoom == 1.0) ImGui::EndDisabled();

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Image Capture")) {

            float pace = settings->countdown.pace / 1000.0;
            ImGui::SliderInt("Countdown Length", &settings->countdown.len, 0, 9, "%d");
            if (ImGui::SliderFloat("Countdown Speed", &pace, 0.8, 3.0, "%.1fsec")) {
                settings->countdown.pace = (int) (pace * 1000);
            }

            fontSelector();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Printing")) {

            ImGui::Checkbox("Save Images", &settings->print_settings.save_images);
            ImGui::BeginDisabled(!settings->print_settings.print_images);
            ImGui::Checkbox("Print Images", &settings->print_settings.print_images);

            if (ImGui::RadioButton("Landscape", settings->print_settings.landscape)) { settings->print_settings.landscape = true; } ImGui::SameLine();
            if (ImGui::RadioButton("Portrait", !settings->print_settings.landscape)) { settings->print_settings.landscape = false; }

            ImGui::SliderFloat("Image Brightness", &settings->print_settings.brightness, 20.0f, 40.0f, "");
            ImGui::SliderFloat("Image Contrast", &settings->print_settings.contrast, 140.0f, 60.0f, "");
            if (!settings->print_settings.print_images) ImGui::EndDisabled();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Extra")) {
            ImGui::SliderFloat("Transparency", &alpha, 1.0f, 0.2f, "%.2f");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
	ImGui::End(); // End Example Window
    ImGui::PopFont();
}

void UIWindow::setStyleOptions() {
    // ktheme style from ImThemes
    ImGuiStyle &style = ImGui::GetStyle();
    float rounding = 4.0f;
    float border_size = 0.5f;
    style.Alpha = alpha;
    style.DisabledAlpha = 0.4f;
    style.WindowPadding = ImVec2(16.10000038146973f, 16.10000038146973f);
    style.WindowRounding = rounding;
    style.WindowBorderSize = border_size;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = rounding;
    style.ChildBorderSize = border_size;
    style.PopupRounding = rounding;
    style.PopupBorderSize = border_size;
    style.FramePadding = ImVec2(8.399999618530273f, 8.100000381469727f);
    style.FrameRounding = rounding;
    style.FrameBorderSize = border_size;
    style.ItemSpacing = ImVec2(8.0f, 8.100000381469727f);
    style.ItemInnerSpacing = ImVec2(11.30000019073486f, 8.399999618530273f);
    style.CellPadding = ImVec2(7.800000190734863f, 10.30000019073486f);
    style.IndentSpacing = 4.300000190734863f;
    style.ColumnsMinSpacing = 4.300000190734863f;
    style.ScrollbarSize = 15.60000038146973f;
    style.ScrollbarRounding = rounding;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 20.0f;
    style.TabRounding = rounding;
    style.TabBorderSize = border_size;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.9080316424369812f, 0.8283261656761169f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2274678349494934f, 0.1815837621688843f, 0.2244754284620285f, 0.4f);
    style.Colors[ImGuiCol_WindowBg] = kbooth_bg_color;
    style.Colors[ImGuiCol_ChildBg] = kbooth_bg_color;
    style.Colors[ImGuiCol_PopupBg] = kbooth_bg_color;
    style.Colors[ImGuiCol_Border] = kbooth_secondary_color;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0, 0.0, 0.0, 0.0);
    style.Colors[ImGuiCol_FrameBg] = kbooth_bg_color;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3519313335418701f, 0.1767208725214005f, 0.3391480147838593f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3519313335418701f, 0.1767208725214005f, 0.3391480147838593f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = kbooth_primary_color;
    style.Colors[ImGuiCol_TitleBgActive] = kbooth_primary_color;
    style.Colors[ImGuiCol_TitleBgCollapsed] = kbooth_primary_color;
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
    style.Colors[ImGuiCol_ScrollbarGrab] = kbooth_primary_color;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_CheckMark] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_SliderGrab] = kbooth_primary_color;
    style.Colors[ImGuiCol_SliderGrabActive] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_Button] = kbooth_primary_color;
    style.Colors[ImGuiCol_ButtonHovered] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2789675295352936f, 0.07542964816093445f, 0.278969943523407f, 1.0f);
    style.Colors[ImGuiCol_Header] = kbooth_secondary_color;
    style.Colors[ImGuiCol_HeaderHovered] = kbooth_secondary_color_hovered;
    style.Colors[ImGuiCol_HeaderActive] = kbooth_primary_color;
    style.Colors[ImGuiCol_Separator] = kbooth_bg_color;
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7490196228027344f, 0.7490196228027344f, 0.7490196228027344f, 0.7803921699523926f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 0.6705882549285889f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f, 0.9490196108818054f);
    style.Colors[ImGuiCol_Tab] = kbooth_secondary_color;
    style.Colors[ImGuiCol_TabHovered] = kbooth_secondary_color_hovered;
    style.Colors[ImGuiCol_TabActive] = kbooth_secondary_color_hovered;
    style.Colors[ImGuiCol_TabUnfocused] = kbooth_bg_color;
    style.Colors[ImGuiCol_TabUnfocusedActive] = kbooth_bg_color;
    style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.7333333492279053f, 0.2666666805744171f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.843137264251709f, 0.4823529422283173f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.7333333492279053f, 0.2666666805744171f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.843137264251709f, 0.4823529422283173f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_TableBorderLight] = kbooth_primary_color;
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.9742489457130432f, 0.9742391705513f, 0.9742391705513f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_DragDropTarget] = kbooth_primary_color_hovered;
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

void free_formats(const char **formats, int size) {
	for (int i = 0; i < 0; i++) delete[] formats[i];
	delete[] formats;
}
