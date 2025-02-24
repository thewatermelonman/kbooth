#ifndef KB_PRINTER_H
#define KB_PRINTER_H
#include <vector>
#include "libusb.h"
#include "stb_image.h"
#include <SDL3/SDL.h>
#include "Kbooth.h"

namespace Kbooth {

    struct UsbDevice {
        uint16_t vendor_id;
        uint16_t product_id;
        std::string  description;
    };

    class Printer {
    private:
		
        std::vector<UsbDevice> usb_devices;
		libusb_device **device_list;
		libusb_device_handle *handle;
		int send_command(std::vector<unsigned char> command);
		int cut();
    public:
		bool init(int port);
        void printSdlSurface(SDL_Surface *capture_surface, PrintSettings *print_set);
		void printDitheredImage(uint8_t *image, int width, int height);
		~Printer();	
    };
}



#endif // KB_PRINTER_H
