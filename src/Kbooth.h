#ifndef KBOOTH_H
#define KBOOTH_H
#include "SDL3/SDL_stdinc.h"
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
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
		std::string output_folder;
		bool save_images;
		bool print_images;
		int printer_usb_port;
		float image_brightness;
		float image_contrast;
    };
}


//Utils:

#include <sys/stat.h>  // For Linux/macOS
#include <sys/types.h>
#ifdef _WIN32
    #include <direct.h> // For _mkdir on Windows
#else
    #include <unistd.h> // For POSIX compatibility
#endif

// Cross-platform mkdir function
static bool createDirectory(const char* path) {
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

static std::string getDate() {
	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);
	std::stringstream ss;	
	ss << std::put_time(&tm, "%d_%m_%Y");
	return ss.str();
}

#endif // KBOOTH_H
