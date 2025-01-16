#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
namespace Kbooth {
    class Camera {
    private:
        int cameraCount;
        SDL_CameraID* cameraIDs;
        SDL_Camera *camera;
        SDL_Texture *texture;
    public:
        Camera();
        ~Camera();
        void getDevices();
        bool open(int device);
        void renderFrame(SDL_Renderer *renderer, SDL_Window *window);
    };
}
#endif // CAMERA_H