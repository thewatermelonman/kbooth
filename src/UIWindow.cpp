#include "UIWindow.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <iostream>

using namespace Kbooth;

void createDropdown(char *label, char *options[], int *selection);
void free_formats(const char **formats, int size);

UIWindow::UIWindow(SDL_Window *window, SDL_Renderer *renderer,
                   Settings *settings, Camera *camera)
    : renderer(renderer), settings(settings), window(window), camera(camera) {


	this->framing = {.zoom = 1.0, .pos_x = 0.0, .pos_y = 0.0, .mirror = true};
	this->camera_index = 0;
	this->cameras_size = 0;
	this->cameras = this->camera->getAvailCameraNames(&this->cameras_size);
	this->formats = this->camera->getAvailFormatNames(this->camera_index, &this->formats_size);
	this->format_index = -1;
	this->opened = false;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    setStyleOptions();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    io.Fonts->AddFontDefault();

    font_regular = io.Fonts->AddFontFromFileTTF("assets/fonts/font1.ttf", 28.0f);
    IM_ASSERT(font_regular != nullptr);
}

UIWindow::~UIWindow() {
	free_formats(this->formats, this->formats_size);
	delete[] this->cameras;
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void UIWindow::processEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);

	if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_S) {
		this->opened = !this->opened;
	}
}

void UIWindow::open() {
	this->opened = true;
}

int UIWindow::render() {
	if (!this->opened) return 0;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();

    {
        ImGui::PushFont(font_regular);
        ImGui::Begin("Kbooth  |><|  Settings", &this->opened);
        ImGui::SeparatorText("General");

		bool old_camera_index = this->camera_index;
        bool change = ImGui::Combo("Webcam", &this->camera_index, cameras, this->cameras_size);
		if (change && old_camera_index != this->camera_index) {
			this->camera->open(this->camera_index, this->format_index);
			this->framing = {.zoom = 1.0, .pos_x = 0.0, .pos_y = 0.0, .mirror = true};
			this->format_index = -1;
			free_formats(this->formats, this->formats_size);
			this->formats = this->camera->getAvailFormatNames(this->camera_index, &this->formats_size);
		}
		
		bool old_format_index = this->format_index;
        change = ImGui::Combo("Webcam Format", &this->format_index, this->formats, this->formats_size);
		if (change && old_format_index != this->format_index) {
			this->camera->open(this->camera_index, this->format_index);
			this->framing = {.zoom = 1.0, .pos_x = 0.0, .pos_y = 0.0, .mirror = true};
			std::cout << this->formats_size << std::endl;
		}


		ImGui::SliderFloat("Zoom", &this->framing.zoom, 1.0f, 1.5f, "%.2f X");

		if (this->framing.zoom == 1.0) {
			this->framing.pos_x = 0.0;
			this->framing.pos_y = 0.0;
			ImGui::BeginDisabled();
		}
		ImVec2 width = ImGui::GetContentRegionAvail();

		ImGui::PushItemWidth(width.x / 4.0f);

			ImGui::BeginGroup();
		
				ImGui::SliderFloat("X", &this->framing.pos_x, 1.0f, -1.00f, "Left\tRight");

				if (this->framing.zoom == 1.0) ImGui::EndDisabled();
				ImGui::Checkbox("Mirror", &this->framing.mirror);
				if (this->framing.zoom == 1.0) ImGui::BeginDisabled();

			ImGui::EndGroup();

		ImGui::PopItemWidth();
		ImGui::SameLine(0.0, width.x / 16.0f);
		const ImVec2 slider_size(width.x / 10.0f, 100.0);
		ImGui::VSliderFloat("Y", slider_size, &this->framing.pos_y, -1.0f, 1.0f, "Up\n \nDown");
		if (this->framing.zoom == 1.0) ImGui::EndDisabled();


        ImGui::End(); // End Example Window
    }
    ImGui::PopFont();
	ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    return 0;
}

KB_framing* UIWindow::getFraming() {
	return &this->framing;
}

int UIWindow::getWebcamIndex() {
	return this->camera_index;	
}

void UIWindow::setStyleOptions() {
    // ktheme style from ImThemes
    ImGuiStyle &style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.4f;
    style.WindowPadding = ImVec2(16.10000038146973f, 16.10000038146973f);
    style.WindowRounding = 6.5f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 4.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(8.399999618530273f, 8.100000381469727f);
    style.FrameRounding = 4.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 8.100000381469727f);
    style.ItemInnerSpacing = ImVec2(11.30000019073486f, 8.399999618530273f);
    style.CellPadding = ImVec2(7.800000190734863f, 10.30000019073486f);
    style.IndentSpacing = 4.300000190734863f;
    style.ColumnsMinSpacing = 4.300000190734863f;
    style.ScrollbarSize = 15.60000038146973f;
    style.ScrollbarRounding = 4.900000095367432f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 20.0f;
    style.TabRounding = 3.900000095367432f;
    style.TabBorderSize = 1.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.9080316424369812f, 0.8283261656761169f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2274678349494934f, 0.1815837621688843f, 0.2244754284620285f, 0.4f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1587982773780823f, 0.08042144775390625f, 0.1530799120664597f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(9.999999974752427e-07f, 9.999899930335232e-07f, 9.999999974752427e-07f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1587982773780823f, 0.08042144775390625f, 0.1530799120664597f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3519313335418701f, 0.1767208725214005f, 0.3391480147838593f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3519313335418701f, 0.1767208725214005f, 0.3391480147838593f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2789675295352936f, 0.07542964816093445f, 0.278969943523407f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2789675295352936f, 0.07542964816093445f, 0.278969943523407f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3519313335418701f, 0.1767208725214005f, 0.3391480147838593f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.1587982773780823f, 0.08042144775390625f, 0.1530799120664597f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7490196228027344f, 0.7490196228027344f, 0.7490196228027344f, 0.7803921699523926f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 0.6705882549285889f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f, 0.9490196108818054f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.2789675295352936f, 0.07542964816093445f, 0.278969943523407f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1587982773780823f, 0.08042144775390625f, 0.1530799120664597f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1587982773780823f, 0.08042144775390625f, 0.1530799120664597f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.7333333492279053f, 0.2666666805744171f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.843137264251709f, 0.4823529422283173f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.7333333492279053f, 0.2666666805744171f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.843137264251709f, 0.4823529422283173f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.02947192452847958f, 0.1373390555381775f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.6000000238418579f, 0.0f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.9742489457130432f, 0.9742391705513f, 0.9742391705513f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.7333333492279053f, 0.2000000029802322f, 0.7333333492279053f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

void free_formats(const char **formats, int size) {
	for (int i = 0; i < 0; i++) delete formats[i];
	delete formats;
}
