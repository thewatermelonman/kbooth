#include "Camera.h"
#include <iostream>
#include <cstdio>
#include "Kbooth.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
using namespace Kbooth;


Camera::Camera() {
    texture = nullptr;
	camera = nullptr;
    cameras = SDL_GetCameras(&cameras_size);
}

Camera::~Camera() {
	if (texture != nullptr) {
		SDL_DestroyTexture(texture);
	}
    if (cameras != nullptr) {
        SDL_free(cameras);
    }
    if (camera != nullptr) {
        SDL_CloseCamera(camera);
    }
	std::cout << "Closing Camera Resources" << std::endl;
}

bool Camera::open(int camera_index, int format_index) {
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
	for (int i = 0; i < *formats_size; i++) {
		int size = snprintf(nullptr, 0, "%dx%d %.1ffps (%d)",
		   specs[i]->width, specs[i]->height,
		    (0.0f + specs[i]->framerate_numerator) / specs[i]->framerate_denominator, specs[i]->colorspace);

		char *const description = new char[size + 1];
		snprintf(description, size + 1, "%dx%d %.1ffps (%d)",
		   specs[i]->width, specs[i]->height,
		    (0.0f + specs[i]->framerate_numerator) / specs[i]->framerate_denominator, specs[i]->colorspace);
		specs_description[i] = description;
		std::cout << description << i << " from " << *formats_size << std::endl;
	}
	SDL_free(specs);
	return specs_description;
}

bool Camera::renderFrame(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing *framing) {
    Uint64 timestampNS;
    SDL_Surface *frame = SDL_AcquireCameraFrame(camera, &timestampNS);

    if (frame != NULL) {
        if (texture) {
            SDL_UpdateTexture(texture, NULL, frame->pixels, frame->pitch);
        } else {

			std::cout << "created texture: " << std::endl;
            //SDL_SetWindowSize(window, frame->w, frame->h); /* Resize the window to match */
            SDL_Colorspace colorspace = SDL_GetSurfaceColorspace(frame);
            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, frame->format);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER, colorspace);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STREAMING);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, frame->w);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, frame->h);
            texture = SDL_CreateTextureWithProperties(renderer, props);
            SDL_DestroyProperties(props);
            if (!texture) {
				std::cerr << "Couldn't create texture: " << SDL_GetError() << std::endl;
				return true;
            }
        }
        SDL_ReleaseCameraFrame(camera, frame);
    }

	SDL_FRect d;
	int win_w, win_h;
	if (texture) {
		SDL_GetRenderOutputSize(renderer, &win_w, &win_h);
		float zoom_crop_x = texture->w * (framing->zoom - 1);
		float zoom_crop_y = texture->h * (framing->zoom - 1);
		float aspect_win = (float) win_w / win_h;
		float aspect_tex = (float) texture->w / texture->h;
		 float black_bar; 
		float scale;
		if (aspect_tex > aspect_win) {
			// Texture wider than window
			scale = (float) win_w / texture->w;
			black_bar = (win_h - texture->h * scale) / 2.0f;
			d.x = (framing->pos_x  - 1) * zoom_crop_x; // position offset - zoom crop
			d.y = black_bar + (framing->pos_y * (zoom_crop_y - black_bar)) - zoom_crop_y; // black bar + position offset - zoom crop
		} else {
			// Texture taller than window
			scale = (float) win_h / texture->h;
			black_bar = (win_w - texture->w * scale) / 2.0f;
			d.y = (framing->pos_y - 1) * zoom_crop_y; 
			d.x = black_bar + (framing->pos_x * (zoom_crop_x - black_bar)) - zoom_crop_x;
		} 
		// texture size + (and push image out of frame by 2 * the zoom_crop)
		d.w = texture->w * scale + zoom_crop_x * 2;
		d.h = texture->h * scale + zoom_crop_y * 2;
		
		SDL_RenderTextureRotated(renderer, texture, NULL, &d, 0.0, NULL, framing->mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
	}
	return false;
}


