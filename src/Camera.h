#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include "Kbooth.h"
#include <ctime>
namespace Kbooth {

	// counts down from Countdown.len --> 0 at Countdown.pace
	struct Countdown {
		int len;
		int pace;	
	
		bool active;
		int position;
		long long start_time;
	};

    class Camera {
    private:
        SDL_Camera *camera;
        SDL_CameraID *cameras;
        int cameras_size;
        SDL_Texture *texture;

		Countdown countdown;

		bool renderCountdown(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing *framing);
		bool renderImageCapture(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing *framing);
        bool renderCameraFeed(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing* framing);
    public:
        Camera();
        ~Camera();

        bool open(int device, int format_index);
        bool renderFrame(SDL_Renderer *renderer, SDL_Window *window, Kbooth::Framing* framing);
		void startCountdown();

		// renders the process of capturing an image

		const char ** getAvailCameraNames(int *size);
		const char ** getAvailFormatNames(int camera_index, int *formats_count);
		int getOpendedCameraID();
    };
}
#endif // CAMERA_H
