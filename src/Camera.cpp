#include "Camera.h"
#include "Kbooth.h"
#include "Printer.h"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "SDL3_ttf/SDL_textengine.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

using namespace Kbooth;


Camera::Camera() : 
	texture(nullptr),
	capture_texture(nullptr),
	capture_surface(nullptr),
	camera(nullptr),
	image_count(0) {
    cameras = SDL_GetCameras(&cameras_size);
    char font[] = "../assets/fonts/SimplyMono-Bold.ttf"; // default font
    countdown_font = TTF_OpenFont(font, 800);
    countdown_border_font = TTF_OpenFont(font, 800);
    TTF_SetFontOutline(countdown_border_font, 8);
    logo_image = IMG_Load("./logo.jpg");
    if (logo_image == NULL) {
        std::cout << SDL_GetError() << std::endl;
    }
}

void Camera::setFontColor(float *color) {
    countdown_color = {
        .r = static_cast<Uint8>(color[0] * 255), 
        .g = static_cast<Uint8>(color[1] * 255), 
        .b = static_cast<Uint8>(color[2] * 255), 
        .a = static_cast<Uint8>(color[3] * 255), 
    };
}

void Camera::setFont(const char *font_file) {
    std::string font_path = "../assets/fonts/";
    font_path += font_file;
    TTF_CloseFont(countdown_font);
    TTF_CloseFont(countdown_border_font);
    countdown_font = TTF_OpenFont(font_path.c_str(), 800);
    countdown_border_font = TTF_OpenFont(font_path.c_str(), 800);
    TTF_SetFontOutline(countdown_border_font, 8);
}

const char ** Camera::getAvailCameraNames(int *size) {
	const char ** camera_names = new const char*[cameras_size];
    for (int i = 0; i < cameras_size; i++) {
		camera_names[i] = SDL_GetCameraName(cameras[i]);
	}
	*size = cameras_size;
	return camera_names;
}

void Camera::setAspectRatio(SDL_Renderer *renderer, int aspect_x, int aspect_y) {
        frame = {0};
        framing_bar_start = {0};
        framing_bar_end = {0};
		int win_w, win_h;
		SDL_GetRenderOutputSize(renderer, &win_w, &win_h);
        float aspect_win = ((float) win_w) / win_h;
        float aspect_frame = ((float) aspect_x) / aspect_y;
        if (aspect_frame > aspect_win) {
            frame.x = 0;
            frame.w = win_w;
            frame.y = (int) ((win_h - win_w / aspect_frame) / 2.0f);
            frame.h = (int) (win_w / aspect_frame);
            framing_bar_start.h = (win_h - frame.h) / 2.0f;
            framing_bar_start.w = win_w;
            framing_bar_end.x = 0;
            framing_bar_end.y = frame.h + frame.y;
        } else {
            frame.y = 0;
            frame.h = win_h;
            frame.x = (int) ((win_w - win_h * aspect_frame) / 2.0f);
            frame.w = (int) (win_h * aspect_frame);
            framing_bar_start.w = (win_w - frame.w) / 2.0f;
            framing_bar_start.h = win_h;
            framing_bar_end.y = 0;
            framing_bar_end.x = frame.w + frame.x;
        }
        framing_bar_start.x = 0;
        framing_bar_start.y = 0;
        framing_bar_end.w = framing_bar_start.w;
        framing_bar_end.h = framing_bar_start.h;
}

const char ** Camera::getAvailFormatNames(int camera_index, int *formats_size) {
    SDL_CameraSpec **specs = SDL_GetCameraSupportedFormats(cameras[camera_index], formats_size);
    const char **specs_description = new const char*[*formats_size];

    for (int i = 0; i < *formats_size; i++) {
        float framerate = (float) specs[i]->framerate_numerator / specs[i]->framerate_denominator;
        std::stringstream description_stream;
        description_stream << specs[i]->width << "x" << specs[i]->height
                          << " " << framerate << "fps (" << specs[i]->colorspace << ")";

        std::string description = description_stream.str();
        specs_description[i] = new char[description.size() + 1];
        strcpy(const_cast<char*>(specs_description[i]), description.c_str());
    }
    SDL_free(specs);
    return specs_description;
}

bool * Camera::getCountdownStatus() {
    return &countdown.active;
}


int Camera::getOpendedCameraID() {
	return SDL_GetCameraID(camera);
}

void Camera::cleanup() {
	if (capture_surface != nullptr) {
		SDL_DestroySurface(capture_surface);
		capture_surface = nullptr;
	}
	if (capture_texture != nullptr) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
	if (texture != nullptr) { // destroy old texture
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
    if (camera != nullptr) { //close old camera
        SDL_CloseCamera(camera);
		camera = nullptr;
    }
    if (cameras != nullptr) {
        SDL_free(cameras);
    }
}

Camera::~Camera() {
    TTF_CloseFont(countdown_border_font);
    TTF_CloseFont(countdown_font);
    SDL_DestroySurface(logo_image);
	cleanup();
	std::cout << "Closing Camera Resources" << std::endl;
}

bool Camera::open(int camera_index, int format_index) {
	countdown = {.active = false, .update = true, .position = 3, .start_time = 0};
	cleanup();
    cameras = SDL_GetCameras(&cameras_size);
    if (cameras_size == 0 || cameras == nullptr) {
        std::cout << "No available Cameras" << std::endl;
        return false;
    }

    if (camera_index >= cameras_size || camera_index < 0) {
        std::cout << "Selected Camera cannot be opended" << std::endl;
        return false;
    }

	// Get selected format
	int count;
	SDL_CameraSpec **specs = SDL_GetCameraSupportedFormats(cameras[camera_index], &count);
	
	if (format_index >= 0 && format_index < count) {
    	camera = SDL_OpenCamera(cameras[camera_index], specs[format_index]);
	} else if (format_index < 0) {
    	camera = SDL_OpenCamera(cameras[camera_index], NULL);
	} else {
		SDL_free(specs);
        std::cout << "Selected Format not available: " << format_index << std::endl;
        return false;
	}
	SDL_free(specs);

    if (camera == nullptr) {
        std::cout << "Could not open Camera" << cameras[camera_index] << std::endl;
        return false;
    }

	// wait for permission 
	int permission = 0;
	while(permission == 0) {
		permission = SDL_GetCameraPermissionState(camera);
	}
	if (permission == 1) {
    	std::cout << "Opened Camera with ID: " << cameras[camera_index] << std::endl;
		return true;
	}
    return false;
}

void Camera::saveAndPrintImage(Printer *printer, PrintSettings *print_set) {
    if (print_set->save_images) {
		std::string filename = print_set->save_folder + "/"; 
        filename += getDateAndTime() + "_" + std::to_string(++image_count) + ".jpg";
        IMG_SaveJPG(capture_surface, filename.c_str(), 100);
    }
	if (print_set->print_images && capture_surface != nullptr && logo_image != nullptr) {
        float scale = (float) capture_surface->w / logo_image->w;
        SDL_Surface *logo_surf = SDL_CreateSurface(
            capture_surface->w,
            capture_surface->h + logo_image->h * scale,
            capture_surface->format
        );

        SDL_Rect capt_r = {
            .x = 0, .y = 0,
            .w = capture_surface->w, .h = capture_surface->h
        };

        SDL_Rect logo_r = {
            .x = 0, .y = capture_surface->h,
            .w = (int) (logo_image->w * scale), .h = (int) (logo_image->h * scale)
        };

        SDL_BlitSurface(capture_surface, NULL, logo_surf, &capt_r);
        SDL_BlitSurfaceScaled(logo_image, NULL, logo_surf, &logo_r, SDL_SCALEMODE_NEAREST);
        printer->printSdlSurface(logo_surf, print_set);
        IMG_SaveJPG(logo_surf, "images/test.jpg", 100);
        SDL_DestroySurface(logo_surf);
        
	}
	if (capture_texture != nullptr) {
		SDL_DestroyTexture(texture);
		capture_texture = nullptr;
	}
	if (texture != nullptr) { // destroy old texture
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}


void Camera::createCountdownTexture(SDL_Renderer *renderer) {
    SDL_Surface *cd_surface = TTF_RenderGlyph_Blended(
        countdown_font, 
        (Uint32) (48 + countdown.position), 
        countdown_color); 
    SDL_Color cd_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface *cd_border_surface = TTF_RenderGlyph_Blended(
        countdown_border_font, 
        (Uint32) (48 + countdown.position), 
        cd_color); 
    SDL_BlitSurface(cd_border_surface, NULL, cd_surface, NULL);
    if (cd_surface) {
		countdown_texture = SDL_CreateTextureFromSurface(renderer, cd_surface);
        SDL_DestroySurface(cd_surface);
        SDL_DestroySurface(cd_border_surface);
		if (countdown_texture == NULL) {
			std::cerr << "Couldn't create cd_texture: " << SDL_GetError() << std::endl;
		}
    } else {
        std::cerr << "Couldn't create cd_surface: " << SDL_GetError() << std::endl;
    }
	std::cout << "created cd_texture: " << std::endl;
}

void Camera::renderCountdown(SDL_Renderer *renderer) {
    if (countdown.update) {
        SDL_DestroyTexture(countdown_texture);
        createCountdownTexture(renderer);
        countdown.update = false;
    }
    int win_w, win_h;
    SDL_GetRenderOutputSize(renderer, &win_w, &win_h);
    float relative_font_scale = 0.5f + countdown.progression * countdown.progression * 0.9f;
    SDL_FRect d;
    d.h = win_h * relative_font_scale;
    d.w = win_h * (float) countdown_texture->w / countdown_texture->h * relative_font_scale;
    d.y = (win_h / 2.0f) - (d.h / 2.0f);
    d.x = (win_w / 2.0f) - (d.w / 2.0f);
    SDL_SetTextureAlphaMod(countdown_texture, (1 - countdown.progression * countdown.progression) * 255);
    SDL_RenderTexture(renderer, countdown_texture, NULL, &d);
}

bool Camera::renderFrame(SDL_Renderer *renderer, Settings *settings) {
    if (countdown.position < 1 && countdown.active) {
        // render image capture animation 
        return renderImageCapture(renderer, settings); 
    } else if (countdown.active) {
        bool res = renderCameraFeed(renderer, &settings->framing, true);
        renderCountdown(renderer); // overlay countdown
        return res;
    } else {
        return renderCameraFeed(renderer, &settings->framing, true);
    }
}

bool Camera::renderCameraFeed(SDL_Renderer *renderer, Framing *framing, bool renderBorder) {
    Uint64 timestampNS;
    SDL_Surface *frame = SDL_AcquireCameraFrame(camera, &timestampNS);

    setAspectRatio(renderer, framing->aspect_x, framing->aspect_y);
    if (frame != NULL) {
        if (texture != nullptr) {
            SDL_UpdateTexture(texture, NULL, frame->pixels, frame->pitch);
        } else {

			std::cout << "created texture: " << std::endl;
            SDL_Colorspace colorspace = SDL_GetSurfaceColorspace(frame);
            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, frame->format);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER, colorspace);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STREAMING);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, frame->w);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, frame->h);
            texture = SDL_CreateTextureWithProperties(renderer, props);
            SDL_DestroyProperties(props);
            if (texture == NULL) {
				std::cerr << "Couldn't create texture: " << SDL_GetError() << std::endl;
				texture = nullptr;
				return false;
            }
        }
        SDL_ReleaseCameraFrame(camera, frame);
    }

	if (!texture) return true;
	renderTexture(renderer, texture, framing, renderBorder);
	return true;
}

bool Camera::renderImageCapture(SDL_Renderer *renderer, Settings *settings) {
	if (capture_texture == nullptr) {
		Uint64 timestampNS;

        bool err = !renderCameraFeed(renderer, &settings->framing, false);
		if (err) return false;

        setAspectRatio(renderer, settings->framing.aspect_x, settings->framing.aspect_y);
		capture_surface = SDL_RenderReadPixels(renderer, &frame);
		if (capture_surface == NULL) return true;

		SDL_Colorspace colorspace = SDL_GetSurfaceColorspace(capture_surface);
		SDL_PropertiesID props = SDL_CreateProperties();
		SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, capture_surface->format);
		SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER, colorspace);
		SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STREAMING);
		SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, capture_surface->w);
		SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, capture_surface->h);
		capture_texture = SDL_CreateTextureWithProperties(renderer, props);
		SDL_DestroyProperties(props);
		if (capture_texture == NULL) {
			std::cerr << "Couldn't create capture_texture: " << SDL_GetError() << std::endl;
			return false;
		}

		std::cout << "created capture_texture: " << std::endl;
    }

	if (!capture_texture) return true;

    SDL_UpdateTexture(capture_texture, NULL, capture_surface->pixels, capture_surface->pitch);
	
	// Animate Image Capture
 	Framing new_frame = {.zoom = 0.8, .pos_x = 0.0, .pos_y = 0.0, .mirror = false, .rotation = 0.0f};
    float ms_since_start = SDL_GetTicks() - countdown.start_time;
	float ms_since_zero = (float) (ms_since_start - ((settings->countdown.len) * settings->countdown.pace));
	float inverse_lerp = (ms_since_zero / (float) (settings->countdown.pace * 2));
    inverse_lerp *= inverse_lerp;
	new_frame.zoom = 1.0 - (inverse_lerp * 0.5);
	new_frame.pos_y = inverse_lerp * 4;
    
    bool err = !renderTexture(renderer, capture_texture, &new_frame, false);
    if (err) return err;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, inverse_lerp * 255);
    err = SDL_RenderFillRect(renderer, NULL);
    if (err) return err;
    return false;
}

bool Camera::renderTexture(
    SDL_Renderer *renderer, 
    SDL_Texture *texture, 
    Framing *framing, 
    bool renderBorder) {

	int win_w, win_h;
	SDL_GetRenderOutputSize(renderer, &win_w, &win_h);

	float aspect_win = (float) win_w / win_h;
	float aspect_tex = (float) texture->w / texture->h;
    
	float scale = aspect_tex > aspect_win ? 
        (float) win_w / texture->w : 
        (float) win_h / texture->h;

	float zoom_crop_y = scale * texture->h / 2.0f * (framing->zoom - 1);
    float zoom_crop_x = scale * texture->w / 2.0f * (framing->zoom - 1);

	float black_bar_x = (win_w - texture->w * scale) / 2.0f; 
    float black_bar_y = (win_h - texture->h * scale) / 2.0f; 
    black_bar_x = aspect_tex > aspect_win ? 0.0f : black_bar_x;	
    black_bar_y = aspect_tex <= aspect_win ? 0.0f : black_bar_y;	

	SDL_FRect d;
    d.x = black_bar_x - ((framing->pos_x + 1) * zoom_crop_x);  // black bar - zoom crop +/- zoom offset
    d.y = black_bar_y - ((framing->pos_y + 1) * zoom_crop_y);
	d.w = texture->w * scale + zoom_crop_x * 2;
	d.h = texture->h * scale + zoom_crop_y * 2;

	bool error = !SDL_RenderTextureRotated(
        renderer, 
        texture, 
        NULL, 
        &d, 
        (double) framing->rotation, 
        NULL, 
        framing->mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    if (renderBorder) { 
        // render the border of what will printed
        // this is defined by the aspect ratio of print_settings
        SDL_FRect frame_F;
        SDL_RectToFRect(&frame, &frame_F);
        SDL_SetRenderDrawColor(renderer, 219, 23, 137, 255);
        error = error || !SDL_RenderRect(renderer, &frame_F);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        error = error || !SDL_RenderFillRect(renderer, &framing_bar_start);
        error = error || !SDL_RenderFillRect(renderer, &framing_bar_end);
    }
    if (error) {
        const char * error_msg = SDL_GetError();
        std::cout << "ERROR: could not render texture: " << error_msg << std::endl;
        return false;
    }
    return true;
}

bool Camera::updateCountdown(CountdownSettings *cd_set) {
    if (!countdown.active) return false;
    
	Uint64 time_to_next_num = (cd_set->len - countdown.position + 1) * cd_set->pace;
	Uint64 time_curr = SDL_GetTicks() - countdown.start_time;
    countdown.progression = 1.0f - (float) (time_to_next_num - time_curr) / (float) cd_set->pace;
	if (time_curr >= time_to_next_num) {
		countdown.position--;
        countdown.progression = 0.0f;
        countdown.update = true;
		std::cout << "  --  COUNTDOWN  --  " << countdown.position;
        std::cout << " since: " << time_curr << " > " << time_to_next_num << std::endl;
	}
	if (countdown.position < -1) {
		countdown.active = false;
        countdown.update = true;
		countdown.position = cd_set->len;
		std::cout << "  --  COUNTDOWN  END --  " << std::endl;
        return true;
	}
    return false;
}

void Camera::startCountdown(CountdownSettings *cd_set) {
    if (countdown.active) return;
    countdown.update = true;
	countdown.position = cd_set->len;
	countdown.start_time = SDL_GetTicks();
	countdown.active = true;
	std::cout << "Started Countdown at time " << countdown.start_time << "ms." << std::endl;
}

