#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "Kbooth.h"
namespace Kbooth {
    class Camera {
    private:
        SDL_Camera *camera;
        SDL_CameraID *cameras;
        int cameras_size;
        SDL_Texture *texture;
    public:
        Camera();
        ~Camera();

        bool open(int device, int format_index);
        bool renderFrame(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing* framing);

		const char ** getAvailCameraNames(int *size);
		const char ** getAvailFormatNames(int camera_index, int *formats_count);
		int getOpendedCameraID();
    };
}
#endif // CAMERA_H
