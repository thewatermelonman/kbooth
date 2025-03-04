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
		int aspect_x;
		int aspect_y;
		bool mirror;
        float rotation;
	};

	// counts down from Countdown.len --> 0 at Countdown.pace
	struct CountdownSettings {
		int len;
		int pace;	
	};

    struct PrintSettings {
		std::string save_folder;
		bool save_images;
		bool print_images;
		int usb_port;
		float brightness;
		float contrast;
        bool landscape;
    };

    struct Settings
    {
		Framing framing;
		CountdownSettings countdown;
        PrintSettings print_settings;
		Uint32 capture_button; // Button that triggers image Capture
    };

    struct UsbDevice {
        uint16_t vendor_id;
        uint16_t product_id;
        std::string  description;
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

static std::string getDateAndTime() {
	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);
	std::stringstream ss;	
	ss << std::put_time(&tm, "%d_%m_%Y_%H_%M");
	return ss.str();
}

#endif // KBOOTH_H
