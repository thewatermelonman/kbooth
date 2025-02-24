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
	renderTexture(renderer, texture, framing, true);
	return true;
}

int brightnessContrast(float b, float c, float x) {
    float y = b + c + x * ((255.0f - c)/255.0f);
    return (int) std::min(255.0f, y);
}

void Camera::saveAndPrintImage(Printer *printer, Printing *printing) {
    if (printing->save_images) {
		std::string filename = printing->save_folder + "/" + getDateAndTime() + "_" + std::to_string(++image_count) + ".jpg";
        IMG_SaveJPG(capture_surface, filename.c_str(), 100);
    }
	if (printing->print_images && capture_surface != nullptr) {
		int w, h;
        SDL_Surface *scaled_surface;
        DitherImage* dither_image;
        if (printing->landscape) {
            int max_height = 576;
            float scaled_width = (float) capture_surface->w * max_height / (float) capture_surface->h;
            scaled_surface = SDL_ScaleSurface(capture_surface, (int) scaled_width, max_height, SDL_SCALEMODE_LINEAR);
    	    dither_image = DitherImage_new(scaled_surface->h, scaled_surface->w);
        } else {
            int max_width = 576;
            float scaled_height = (float) capture_surface->h * max_width / (float) capture_surface->w;
            scaled_surface = SDL_ScaleSurface(capture_surface, max_width, (int) scaled_height, SDL_SCALEMODE_LINEAR);
    	    dither_image = DitherImage_new(scaled_surface->w, scaled_surface->h);
        }
        w = scaled_surface->w;
        h = scaled_surface->h;
		Uint8 r, g, b;
        std::cout << "Scale X-Y" << w << "-" << h << std::endl;
        std::cout << "Dither X-Y" << dither_image->width << "-" << dither_image->height << std::endl;
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				SDL_ReadSurfacePixel(scaled_surface, x, y, &r, &g, &b, NULL);
                r = brightnessContrast(printing->brightness, printing->contrast, r);
                g = brightnessContrast(printing->brightness, printing->contrast, g);
                b = brightnessContrast(printing->brightness, printing->contrast, b);
                if (printing->landscape) {
                    if (x <= 2 && y <= 2) std::cout <<  x << "|" << y << " --To-- " << h-y << "|" << x << std::endl;
                    DitherImage_set_pixel(dither_image, h-y, x, r, g, b, true);
                } else {
                    DitherImage_set_pixel(dither_image, x, y, r, g, b, true);
                }
			}
		}
        uint8_t *out_image = (uint8_t*)calloc(w * h, sizeof(uint8_t));
        ErrorDiffusionMatrix *em = get_shiaufan3_matrix();
		// error_diffusion_dither(dither_image, em, false, 0.0, out_image);
        dbs_dither(dither_image, 3, out_image);
		printer->printDitheredImage(out_image, dither_image->width, dither_image->height);
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				Uint8 f = out_image[x + y * w] == 0xff ? 255 : 0;
				SDL_WriteSurfacePixel(scaled_surface, x, y, f, f, f, 255);
			}
		}
		std::string filename = printing->save_folder + "/DITH_" + getDateAndTime() + "_" + std::to_string(++image_count) + ".jpg";
        IMG_SaveJPG(scaled_surface, filename.c_str(), 100);
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
	float ms_since_zero = (float) (SDL_GetTicks() - settings->countdown.start_time - ((settings->countdown.len) * settings->countdown.pace));
	float inverse_lerp = 1.0f - (ms_since_zero / (float) settings->countdown.pace);
	new_frame.zoom = std::max(inverse_lerp * 0.5 + 0.5, 0.7); // zoom out until at 70%
	new_frame.pos_y = -1.0f + inverse_lerp * 2;
	return renderTexture(renderer, capture_texture, &new_frame, false);
}

bool Camera::renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, Framing *framing, bool renderBorder) {

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

	bool error = !SDL_RenderTextureRotated(renderer, texture, NULL, &d, (double) framing->rotation, NULL, framing->mirror ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    if (renderBorder) {
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
