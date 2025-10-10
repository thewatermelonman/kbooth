#ifndef KB_PRINTER_H
#define KB_PRINTER_H
#include <vector>
#include "libusb.h"
#include "stb_image.h"
#include <SDL3/SDL.h>
#include "Kbooth.h"

namespace Kbooth {

    class Printer {
    private:
	    libusb_context *ctx;
        bool initialized = false;
        std::vector<UsbDevice> usb_devices;
		libusb_device_handle *handle;

		int send_command(std::vector<unsigned char> command);
		int cut();
    public:
        bool init();
        std::vector<UsbDevice>* getAvailUsbDevices();
        bool open(UsbDevice& dev);

        bool initAndOpen(UsbDevice *default_dev);
        void cleanup();

        void printSdlSurface(SDL_Surface *capture_surface, PrintSettings *print_set);
		void printDitheredImage(uint8_t *image, int width, int height);

		~Printer();	
    };
}



#endif // KB_PRINTER_H
