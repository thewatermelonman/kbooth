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

	// counts down from Countdown.len --> 0 at Countdown.pace
	struct Countdown {
		int len;
		int pace;	
	
		bool active;
		int position;
		Uint64 start_time;
	};

    struct Settings
    {
		Framing Framing;
		Countdown countdown;
		Uint32 Capture_Button; // Button that triggers image Capture
		int Capture_Duration; //in frames
    };
}
#endif // KBOOTH_H
