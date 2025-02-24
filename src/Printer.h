#ifndef KB_PRINTER_H
#define KB_PRINTER_H
#include <vector>
#include "libusb.h"
#include "stb_image.h"
namespace Kbooth {

    class Printer {
    private:
		
		libusb_device **device_list;
		libusb_device_handle *handle;
		int send_command(std::vector<unsigned char> command);
		int cut();
    public:
		bool init(int port);
		void printBitmap(std::vector<std::vector<bool>> &bitmap);
		void printDitheredImage(uint8_t *image, int width, int height);
		~Printer();	
    };
}



#endif // KB_PRINTER_H
