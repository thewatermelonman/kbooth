#include "Camera.h"
#include <iostream>
#include "Kbooth.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include <cmath>
using namespace Kbooth;

Camera::Camera() {
    this->getDevices();
    this->texture = NULL;
}

Camera::~Camera() {
    if (this->cameraIDs != nullptr) {
        SDL_free(this->cameraIDs);
    }
    if (this->camera != nullptr) {
        SDL_CloseCamera(this->camera);
    }
}

void Camera::getDevices() {
    this->cameraIDs = SDL_GetCameras(&(this->cameraCount));
    if (this->cameraCount == 0 || this->cameraIDs == nullptr) {
        std::cout << "No available Cameras" << std::endl;
    }
    std::cout << "Available Cameras" << std::endl;
    for (int i = 0; i < this->cameraCount; i++) {
        std::cout << SDL_GetCameraName(this->cameraIDs[i]) << "\n";
    }
}

bool Camera::open(int device) {
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

    // std::cout << "Available:" << this->cameraCount << " Chosen: " << SDL_GetCameraName(device) << " - " << device << std::endl;
    this->camera = SDL_OpenCamera(this->cameraIDs[device], NULL);
    if (this->camera == nullptr) {
        std::cout << "Could not open Camera" << std::endl;
        return false;
    }
    std::cout << "Opened Camera" << std::endl;
    return true;
}

void Camera::renderFrame(SDL_Renderer *renderer, SDL_Window *window, KB_framing *framing) {
    Uint64 timestampNS;
    SDL_Surface *frame = SDL_AcquireCameraFrame(this->camera, &timestampNS);

    if (frame != NULL) {
        if (this->texture) {
            SDL_UpdateTexture(this->texture, NULL, frame->pixels, frame->pitch);
        } else {
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
				std::cout << "Couldn't create texture: " << SDL_GetError() << std::endl;
            }
        }
        SDL_ReleaseCameraFrame(this->camera, frame);
    }

	SDL_FRect d;
	int win_w, win_h;
		float scale;
	if (this->texture) {
		SDL_GetRenderOutputSize(renderer, &win_w, &win_h);
		int tex_w = this->texture->w;
		int tex_h = this->texture->h;
		float zoom_crop_x = tex_w * (framing->zoom - 1);
		float zoom_crop_y = tex_h * (framing->zoom - 1);
		float aspect_win = (float) win_w / win_h;
		float aspect_tex = (float) tex_w / tex_h;
		if (aspect_tex > aspect_win) {
			scale = (float) win_w / tex_w;
			d.x = 0.0f + (framing->pos_x - 1) * zoom_crop_x;
			d.y = ( (win_h - tex_h * scale) / 2.0f ) + (framing->pos_y - 1) * zoom_crop_y;
		} else {
			scale = (float) win_h / tex_h;
			d.y = 0.0f + (framing->pos_y - 1) * zoom_crop_y ;
			d.x = ( (win_w - tex_w * scale) / 2.0f ) + (framing->pos_x - 1) * zoom_crop_x;
		}
			d.w =  tex_w * scale + zoom_crop_x * 2;
			d.h =  tex_h * scale + zoom_crop_y * 2;
		
		SDL_RenderTextureRotated(renderer, this->texture, NULL, &d, 0.0, NULL, SDL_FLIP_NONE);
	}
}


void flipVertical() {

}
