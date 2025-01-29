#include "Camera.h"
#include <iostream>
#include "Kbooth.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
using namespace Kbooth;


Camera::Camera() {
    this->getDevices();
    this->texture = NULL;
	this-> camera = NULL;
}

Camera::~Camera() {
    if (this->cameraIDs != nullptr) {
        SDL_free(this->cameraIDs);
    }
    if (this->camera != nullptr) {
        SDL_CloseCamera(this->camera);
    }
	std::cout << "Closing Camera Resources" << std::endl;
}

void Camera::getDevices() {
    this->cameraIDs = SDL_GetCameras(&(this->cameraCount));
    if (this->cameraCount == 0 || this->cameraIDs == nullptr) {
        std::cout << "No available Cameras" << std::endl;
    }
    for (int i = 0; i < this->cameraCount; i++) {
        std::cout << SDL_GetCameraName(this->cameraIDs[i]) << " with ID: " << this->cameraIDs[i] << "\n";
    }
}

const char ** Camera::getAvailCameraNames(int *size) {
	const char ** cameras = new const char*[this->cameraCount];
    for (int i = 0; i < this->cameraCount; i++) {
		cameras[i] = SDL_GetCameraName(this->cameraIDs[i]);
	}
	*size = this->cameraCount;
	return cameras;
}

bool Camera::open(int device) {
    if (this->camera != nullptr) { //close old camera
        SDL_CloseCamera(this->camera);
    }

    if (this->cameraCount == 0) {
        return false;
    }
    if (device >= this->cameraCount || device < 0) {
        return false;
    }
    if (this->cameraIDs == nullptr) {
        std::cout << "No available Cameras" << std::endl;
        return false;
    }

    this->camera = SDL_OpenCamera(this->cameraIDs[device], NULL);
    if (this->camera == nullptr) {
        std::cout << "Could not open Camera " << this->cameraIDs[device] << std::endl;
        return false;
    }
	int permission = 0;
	while(permission == 0) {
		permission = SDL_GetCameraPermissionState(this->camera);
	}
	if (permission == 1) {
    	std::cout << "Opened Camera with ID: " << this->cameraIDs[device] << std::endl;
		return true;
	}
    return false;
}

void Camera::renderFrame(SDL_Renderer *renderer, SDL_Window *window, KB_framing *framing, bool *window_should_close) {
    Uint64 timestampNS;
    SDL_Surface *frame = SDL_AcquireCameraFrame(this->camera, &timestampNS);

    if (frame != NULL) {
        if (this->texture) {
            SDL_UpdateTexture(this->texture, NULL, frame->pixels, frame->pitch);
        } else {

			std::cout << "created texture: " << std::endl;
            SDL_SetWindowSize(window, frame->w, frame->h); /* Resize the window to match */
            SDL_Colorspace colorspace = SDL_GetSurfaceColorspace(frame);
            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, frame->format);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER, colorspace);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STREAMING);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, frame->w);
            SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, frame->h);
            this->texture = SDL_CreateTextureWithProperties(renderer, props);
            SDL_DestroyProperties(props);
            if (!this->texture) {
				std::cerr << "Couldn't create texture: " << SDL_GetError() << std::endl;
				*window_should_close = true;
            }
        }
        SDL_ReleaseCameraFrame(this->camera, frame);
    }

	SDL_FRect d;
	int win_w, win_h;
	if (this->texture) {
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
		
		SDL_RenderTextureRotated(renderer, this->texture, NULL, &d, 0.0, NULL, SDL_FLIP_NONE);
	}
}


