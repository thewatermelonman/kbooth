#ifndef KBOOTH_H
#define KBOOTH_H
#include "SDL3/SDL_stdinc.h"
namespace Kbooth
{
	struct Framing
	{
		float zoom;
		float pos_x;
		float pos_y;
		bool mirror;
	};

    struct Settings
    {
		Framing Framing;
		Uint32 Capture_Button; // Button that triggers image Capture
		int Capture_Duration; //in frames
    };
}
#endif // KBOOTH_H
