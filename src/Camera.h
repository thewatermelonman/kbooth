#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Kbooth.h"
#include "Printer.h"
namespace Kbooth {

    struct CountdownState {
		bool active;
        bool update; // signals change in position
		int position; // n
		Uint64 start_time; // t0
        float progression; // [0.0, 1.0)  for progression between tn and t(n-1)
    };
        
    class Camera {
    private:
        SDL_Camera *camera;
        SDL_CameraID *cameras;
        int cameras_size;
        SDL_Texture *texture;
        SDL_Texture *capture_texture;
		SDL_Surface *capture_surface;

        TTF_Font *countdown_font;
        TTF_Font *countdown_border_font;
        SDL_Texture *countdown_texture;

        // frame and black bars that crop to image capture 1:1, 4:3, 16:9, etc.
        SDL_Rect frame;
        SDL_FRect framing_bar_start;
        SDL_FRect framing_bar_end;

		int image_count;
        CountdownState countdown;
        SDL_Color countdown_color;
        

		void cleanup(); // closes all resources
		bool renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, Framing *framing, bool renderBorder);
    public:
        Camera();
        ~Camera();
        void setFont(const char *font_file);
        void setFontColor(float *color);

        bool open(int device, int format_index);
        void setAspectRatio(SDL_Renderer *renderer, int aspect_x, int aspect_y);

		void saveAndPrintImage(Printer *printer, PrintSettings *printing);

        bool renderFrame(SDL_Renderer *renderer, Settings *settings);
        void renderCountdown(SDL_Renderer *renderer);
		bool renderImageCapture(SDL_Renderer *renderer, Settings *settings);
        bool renderCameraFeed(SDL_Renderer *renderer, Framing *framing, bool renderBorder);

		const char ** getAvailCameraNames(int *size);
		const char ** getAvailFormatNames(int camera_index, int *formats_count);
		int getOpendedCameraID();
    
        void startCountdown(CountdownSettings *cd_set);
        bool updateCountdown(CountdownSettings *cd_set);
        void createCountdownTexture(SDL_Renderer *renderer);
    };
}
#endif // CAMERA_H
