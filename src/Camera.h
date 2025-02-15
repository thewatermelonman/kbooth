#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "Kbooth.h"
#include <ctime>
namespace Kbooth {

    class Camera {
    private:
        SDL_Camera *camera;
        SDL_CameraID *cameras;
        int cameras_size;
        SDL_Texture *texture;
        SDL_Texture *capture_texture;
		SDL_Surface *capture_surface;

		void cleanup(); // closes all resources
		void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, Framing *framing);
    public:
        Camera();
        ~Camera();

        bool open(int device, int format_index);

		void saveImage();

		// renders the process of capturing an image
		bool renderImageCapture(SDL_Renderer *renderer, Framing *framing, Countdown *countdown);
        bool renderCameraFeed(SDL_Renderer *renderer, Framing* framing);

		const char ** getAvailCameraNames(int *size);
		const char ** getAvailFormatNames(int camera_index, int *formats_count);
		int getOpendedCameraID();
    };
}
#endif // CAMERA_H
