#include "Camera.h"
#include "Kbooth.h"
#include "Printer.h"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3_image/SDL_image.h"
#include "libdither.h"
#include <iostream>
#include <string>
#include <ctime>

using namespace Kbooth;

Camera::Camera() : 
	texture(nullptr),
	capture_texture(nullptr),
	capture_surface(nullptr),
	camera(nullptr),
	image_count(0) {
    cameras = SDL_GetCameras(&cameras_size);
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
	cleanup();
	std::cout << "Closing Camera Resources" << std::endl;
}

bool Camera::open(int camera_index, int format_index) {
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

int Camera::getOpendedCameraID() {
	return SDL_GetCameraID(camera);
}

const char ** Camera::getAvailCameraNames(int *size) {
	const char ** camera_names = new const char*[cameras_size];
    for (int i = 0; i < cameras_size; i++) {
		camera_names[i] = SDL_GetCameraName(cameras[i]);
	}
	*size = cameras_size;
	return camera_names;
}

const char ** Camera::getAvailFormatNames(int camera_index, int *formats_size) {
	SDL_CameraSpec **specs = SDL_GetCameraSupportedFormats(cameras[camera_index], formats_size);
	const char ** specs_description = new const char*[*formats_size];
	float framerate;
	for (int i = 0; i < *formats_size; i++) {
		framerate = (float) specs[i]->framerate_numerator / specs[i]->framerate_denominator;
		int size = snprintf(nullptr, 
					  0, 
					 "%dx%d %.1ffps (%d)",
					  specs[i]->width, 
					  specs[i]->height,
					  framerate, 
					  specs[i]->colorspace);
		char *const description = new char[size + 1];
		snprintf(description, 
		   size + 1, 
		   "%dx%d %.1ffps (%d)", 
		   specs[i]->width, 
		   specs[i]->height, 
		   framerate, 
		   specs[i]->colorspace);
		specs_description[i] = description;
	}
	SDL_free(specs);
	return specs_description;
}

bool Camera::renderCameraFeed(SDL_Renderer *renderer, Framing *framing) {
    Uint64 timestampNS;
    SDL_Surface *frame = SDL_AcquireCameraFrame(camera, &timestampNS);

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
	renderTexture(renderer, texture, framing);
	return true;
}

void Camera::saveAndPrintImage(Printer *printer, std::string &output_folder, bool save, float brightness, float contrast) {
	if (capture_surface != nullptr) {
		std::string filename = output_folder + "/" + getDate() + "_" + std::to_string(++image_count) + ".jpg";
		int max_width = 576;
		float scaled_height = (float) capture_surface->h * max_width / (float) capture_surface->w;
		SDL_Surface *scaled_surface = SDL_ScaleSurface(capture_surface, max_width, (int) scaled_height, SDL_SCALEMODE_LINEAR);
		int w, h;
		w = scaled_surface->w;
		h = scaled_surface->h;
		Uint8 r, g, b;
    	DitherImage* dither_image = DitherImage_new(w, h);
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				SDL_ReadSurfacePixel(scaled_surface, x, y, &r, &g, &b, NULL);
				r = (int) std::min(255.0f, brightness + contrast + (float) r * ((255.0f - contrast)/255.0f));
				g = (int) std::min(255.0f, brightness + contrast + (float) g * ((255.0f - contrast)/255.0f));
				b = (int) std::min(255.0f, brightness + contrast + (float) b * ((255.0f - contrast)/255.0f));
        		DitherImage_set_pixel(dither_image, x, y, r, g, b, true);
			}
		}
        uint8_t *out_image = (uint8_t*)calloc(w * h, sizeof(uint8_t));
        ErrorDiffusionMatrix *em = get_shiaufan3_matrix();
		//error_diffusion_dither(dither_image, em, false, 0.0, out_image);
        dbs_dither(dither_image, 3, out_image);
		printer->printDitheredImage(out_image, w, h);
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				Uint8 f = out_image[x + y * w] == 0xff ? 255 : 0;
				SDL_WriteSurfacePixel(scaled_surface, x, y, f, f, f, 255);
			}
		}
		if (save) {
			IMG_SaveJPG(scaled_surface, filename.c_str(), 100);
			filename += ".dith";
			IMG_SaveJPG(capture_surface, filename.c_str(), 100);
		}
        ErrorDiffusionMatrix_free(em);
    	free(out_image);
		SDL_DestroySurface(scaled_surface);
		SDL_DestroySurface(capture_surface);
		capture_surface = nullptr;
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

bool Camera::renderImageCapture(SDL_Renderer *renderer, Framing *framing, Countdown *countdown) {
	if (capture_texture == nullptr) {
		Uint64 timestampNS;
		if (!renderCameraFeed(renderer, framing)) return false;
		int win_w, win_h;
		SDL_GetRenderOutputSize(renderer, &win_w, &win_h);
		float aspect_win = (float) win_w / win_h;
		float aspect_tex = (float) texture->w / texture->h;
		float black_bar; 
		float scale;
		SDL_Rect r;
		float zoom_crop_x = texture->w * (framing->zoom - 1);
		float zoom_crop_y = texture->h * (framing->zoom - 1);
		if (aspect_tex > aspect_win) { //TODO: REFACTOR
			// Texture wider than window
			scale = (float) win_w / texture->w;
			black_bar = (win_h - texture->h * scale) / 2.0f;
			r.x = 0;
			r.y = (int) std::max(0.0f, black_bar - (framing->pos_y * (black_bar - zoom_crop_y)) - zoom_crop_y); // black bar + position offset - zoom crop
		} else {
			// Texture taller than window
			scale = (float) win_h / texture->h;
			black_bar = (win_w - texture->w * scale) / 2.0f;
			r.y = 0;
			r.x = (int) std::max(0.0f, black_bar - (framing->pos_x * (black_bar - zoom_crop_x)) - zoom_crop_x);
		} 
		// texture size + (and push image out of frame by 2 * the zoom_crop)
		r.w = (int) std::min(texture->w * scale + zoom_crop_x * 2, (float) win_w);
		r.h = (int) std::min(texture->h * scale + zoom_crop_y * 2, (float) win_h);

		capture_surface = SDL_RenderReadPixels(renderer, &r);
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
 	Framing new_frame = {.zoom = 0.8, .pos_x = 0.0, .pos_y = 0.0, .mirror = false};
	float ms_since_zero = (float) (SDL_GetTicks() - countdown->start_time - ((countdown->len) * countdown->pace));
	float inverse_lerp = 1.0f - (ms_since_zero / (float) countdown->pace);
	new_frame.zoom = std::max(inverse_lerp * 0.5 + 0.5, 0.7); // zoom out until at 70%
	new_frame.pos_y = -1.0f + inverse_lerp * 2;
	renderTexture(renderer, capture_texture, &new_frame);
	return true;
}

void Camera::renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, Framing *framing) {

	int win_w, win_h;
	SDL_GetRenderOutputSize(renderer, &win_w, &win_h);

	float zoom_crop_x = texture->w * (framing->zoom - 1);
	float zoom_crop_y = texture->h * (framing->zoom - 1);

	float aspect_win = (float) win_w / win_h;
	float aspect_tex = (float) texture->w / texture->h;

	SDL_FRect d;
	 
	float black_bar; 
	float scale;
	if (aspect_tex > aspect_win) {//TODO: REFACTOR
		// Texture wider than window
		scale = (float) win_w / texture->w;
		black_bar = (win_h - texture->h * scale) / 2.0f;
		d.x = (framing->pos_x  - 1) * zoom_crop_x; // position offset - zoom crop
		d.y = black_bar - (framing->pos_y * (black_bar - zoom_crop_y)) - zoom_crop_y; // black bar + position offset - zoom crop
	} else {
		// Texture taller than window
		scale = (float) win_h / texture->h;
		black_bar = (win_w - texture->w * scale) / 2.0f;
		d.y = (framing->pos_y - 1) * zoom_crop_y; 
		d.x = black_bar - (framing->pos_x * (black_bar - zoom_crop_x)) - zoom_crop_x;
	} 
	// texture size + (and push image out of frame by 2 * the zoom_crop)
	d.w = texture->w * scale + zoom_crop_x * 2;
	d.h = texture->h * scale + zoom_crop_y * 2;
	
	SDL_RenderTextureRotated(renderer, texture, NULL, &d, 0.0, NULL, framing->mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
