#include "Camera.h"
Camera::Camera() { }
Camera::~Camera() { }
void Camera::set_available_cameras() {
    int devcount = 0;
    SDL_CameraID *devices = SDL_GetCameras(&devcount);
}