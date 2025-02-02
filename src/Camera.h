#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "Kbooth.h"
namespace Kbooth {
    class Camera {
    private:
        int cameraCount;
        SDL_CameraID *cameraIDs;
        SDL_Camera *camera;
        SDL_Texture *texture;
    public:
        Camera();
        ~Camera();
        void getDevices();
        bool open(int device, int format_index);
        void renderFrame(SDL_Renderer *renderer, SDL_Window *window, KB_framing* framing, bool *window_should_close);
		const char ** getAvailCameraNames(int *size);
		const char ** getAvailFormatNames(int camera_index, int *formats_count);
		int getOpendedCameraID();
    };
}
#endif // CAMERA_H
