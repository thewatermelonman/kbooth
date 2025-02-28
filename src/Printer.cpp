#include "Printer.h"
#include <libusb.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "EscPosCommands.h"
#include "SDL3/SDL.h"
#include "libdither.h"
#include <string>

using namespace Kbooth;


int brightnessContrast(float b, float c, float x) {
    float y = b + c + x * ((255.0f - c)/255.0f);
    return (int) std::min(255.0f, y);
}

bool Printer::init(int port) {
	libusb_context *ctx = NULL;
	handle = nullptr;
	int err = libusb_init(&ctx);
    std::vector<std::string> usb_descriptors;
	struct libusb_device_descriptor desc;
    if (err) {
		std::cerr << "libusb initialization failed\n" << std::endl;
        return false;
    }
	std::cout << "Initialized libusb" << std::endl;
	ssize_t count = libusb_get_device_list(NULL, &device_list);	
	libusb_device *printer = nullptr;
	for (int i = 0; i < count; i++) {
		int dev_port = (int) libusb_get_port_number(device_list[i]);
		if (handle == nullptr) libusb_open(device_list[i], &handle);

		if (handle != nullptr) {
			int ret = libusb_get_device_descriptor(device_list[i], &desc);
			if (ret < 0) {
				std::cerr << "failed to get device descriptor" << std::endl;
				return false;
			}
            unsigned char data[256];
            ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, data, 256);
            std::cout << "On Port " << dev_port << "  found: " << data << std::endl;
            if (ret > 0) {
                std::string data_sting(reinterpret_cast<char*>(data));
                UsbDevice device = {
                    .vendor_id = desc.idVendor,
                    .product_id = desc.idProduct,
                    .description = data_sting}; 
                usb_devices.push_back(device);
            }
			libusb_close(handle);
			handle = nullptr;
			if (dev_port == port && printer == nullptr) {
				printer = device_list[i];
				// break;
			}
		}
	}
	if (printer == nullptr) {
		std::cerr << "ERROR: No USB device at port: " << port << std::endl;
		return false;
	}
	err = libusb_open(printer, &handle);
    int ret = libusb_get_device_descriptor(printer, &desc);
    if (ret < 0) {
        fprintf(stderr, "failed to get device descriptor");
        return false;
    }
    std::cout << "Opened Device" << std::endl;
	if (err) {
		std::cerr << "ERROR: could not open usb device: " << (int) err << std::endl;
		libusb_free_device_list(device_list, 0);
		return false;
	}
	std::cout << "OPENED PRINTER" << std::endl;


	if (libusb_kernel_driver_active(handle, 0)) {
		libusb_detach_kernel_driver(handle, 0);
	}

	err = libusb_claim_interface(handle, 0);
	if (err) {
		std::cerr << "ERROR: could not claim interface: " << err << std::endl;
		libusb_close(handle);
		libusb_free_device_list(device_list, 0);
		return false;
	}
	return true;
}

Printer::~Printer() {
	libusb_close(handle);
	libusb_free_device_list(device_list, 0);
	std::cout << "Closing Printer resources" << std::endl;
}

int Printer::send_command(std::vector<unsigned char> command) {
	int actual_len;
	unsigned char ENDPOINT = 0x01;
	int err = libusb_bulk_transfer(handle, 
								ENDPOINT, 
								const_cast<unsigned char*>(command.data()), 
								command.size(),
								&actual_len, 
								50000);
	if (err) {
		std::cerr << "ERROR: could not write to open usb device: " << (int) err << std::endl;
		return 1;
	}
	return err;
}

int Printer::cut() {
    unsigned char n = 25;  // Set 'n' to the desired feed length (e.g., 5 vertical motion units)
    std::vector<unsigned char> cutCommand2 = {0x1D, 0x56, 0x42, n};

	int err = send_command(cutCommand2);
	if (err) {
		std::cerr << "Error while sending data: " << err << std::endl; 
	}
	return err;
}

void Printer::printDitheredImage(uint8_t *image, int width, int height) {
	std::cout << "WidthxHeight apparently " << width << "x" << height << std::endl;
	send_command(ESC_Init);
	send_command(ESC_Three);
	int width_char = (int) std::ceil((float) width / 8.0f);
	unsigned char yL = (unsigned char) (height % 256);
	unsigned char yH = (unsigned char) (height / 256);
    unsigned char xL = (unsigned char) (width_char % 256);
    unsigned char xH = (unsigned char) (width_char / 256);
	int x_num = 0;
	int y_num = 0;
    std::string total = {'\x1d', '\x76', '\x30', '\x00'};
	total += xL;
	total += xH;
	total += yL;
	total += yH;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width;) {
			char d_k = 0;
			for (int c = 0; c < 8 && x < width; c++, x++) {
				if (image[y * width + x] != 0xff) {
					d_k |= (1 << (7 - c));
				}
			}
			total += d_k;
			if (y == 0) x_num++;
		}
		y_num++;
	}
	std::vector<unsigned char> data(total.begin(), total.end());
    send_command(data);
    send_command(ESC_Two);
	send_command(ESC_LF);
	send_command(ESC_J);
	cut();
	std::cout << "AFTER DATA TRANS: " << total.size() << "WxH: " << x_num << "x"  << y_num << std::endl; 
	std::cout << "xL "<< (int) xL << ", xH "<< (int) xH << ", yL "<< (int) yL << ", yH "<< (int) yH << std::endl; 
}

void Printer::printSdlSurface(SDL_Surface *capture_surface, PrintSettings *print_set) {
    int w, h;
    SDL_Surface *scaled_surface;
    DitherImage* dither_image;
    if (print_set->landscape) {
        int max_height = 576;
        float scaled_width = (float) capture_surface->w * max_height / (float) capture_surface->h;
        scaled_surface = SDL_ScaleSurface(capture_surface, (int) scaled_width, max_height, SDL_SCALEMODE_LINEAR);
        dither_image = DitherImage_new(scaled_surface->h, scaled_surface->w);
    } else {
        int max_width = 576;
        float scaled_height = (float) capture_surface->h * max_width / (float) capture_surface->w;
        scaled_surface = SDL_ScaleSurface(capture_surface, max_width, (int) scaled_height, SDL_SCALEMODE_LINEAR);
        dither_image = DitherImage_new(scaled_surface->w, scaled_surface->h);
    }
    w = scaled_surface->w;
    h = scaled_surface->h;
    Uint8 r, g, b;
    std::cout << "Scale X-Y" << w << "-" << h << std::endl;
    std::cout << "Dither X-Y" << dither_image->width << "-" << dither_image->height << std::endl;
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            SDL_ReadSurfacePixel(scaled_surface, x, y, &r, &g, &b, NULL);
            r = brightnessContrast(print_set->brightness, print_set->contrast, r);
            g = brightnessContrast(print_set->brightness, print_set->contrast, g);
            b = brightnessContrast(print_set->brightness, print_set->contrast, b);
            if (print_set->landscape) {
                if (x <= 2 && y <= 2) std::cout <<  x << "|" << y << " --To-- " << h-y << "|" << x << std::endl;
                DitherImage_set_pixel(dither_image, h-y, x, r, g, b, true);
            } else {
                DitherImage_set_pixel(dither_image, x, y, r, g, b, true);
            }
        }
    }
    uint8_t *out_image = (uint8_t*)calloc(w * h, sizeof(uint8_t));
    ErrorDiffusionMatrix *em = get_shiaufan3_matrix();
    // error_diffusion_dither(dither_image, em, false, 0.0, out_image);
    dbs_dither(dither_image, 3, out_image);
    printDitheredImage(out_image, dither_image->width, dither_image->height);
    ErrorDiffusionMatrix_free(em);
    free(out_image);
    SDL_DestroySurface(scaled_surface);
}




//unused
/*
void Printer::printBitmap(std::vector< std::vector<bool> > &bitmap) {
	// Quickly check the integrity of the "bitmap"
    int height = bitmap.size();
    int width = bitmap[0].size();
    std::cout << "WIDTHxHEIGHT = " <<  height <<" x" << (width / 8) << " = " << height * (width / 8) << std::endl;
    for(int i = 1; i < height; i++){
        if(width != bitmap[i].size()){
            std::cout << "Error: the bitmap is not squared" << std::endl;
            throw -1;
        }
    }
	int x_num = 0;
	int y_num = 0;
	int width_char = (int) std::ceil((float) width / 8.0f);
    // First of all, set line height:
	send_command(ESC_Three);
	unsigned char yL = (unsigned char) (height % 256);
	unsigned char yH = (unsigned char) (height / 256);
    unsigned char xL = (unsigned char) (width_char % 256);
    unsigned char xH = (unsigned char) (width_char / 256);
    std::string total = {'\x1d', '\x76', '\x30', '\x00'};
	total += xL;
	total += xH;
	total += yL;
	total += yH;
	std::cout << std::dec;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width;) {
			char d_k = 0;
			for (int c = 0; c < 8 && x < width; c++, x++) {
				if (bitmap[y][x]) {
					d_k |= (1 << (7 - c));
				}
			}
			total += d_k;
			if (y == 0) x_num++;
		}
		y_num++;
	}
	std::vector<unsigned char> data(total.begin(), total.end());
    send_command(data);
	std::cout << "AFTER DATA TRANS: " << total.size() << std::endl; 
    send_command(ESC_Two);
}

static std::vector<std::vector<bool>> loadImage(std::string filename){
    int width, height, channels;
    stbi_uc *result = stbi_load(filename.c_str(), &width, &height, &channels, 3);

    std::vector< std::vector<bool> > image(height, std::vector<bool>(width, 0));
    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            // For each pixel, we will use the Luminosity method:
            // -> (0.3 * R) + (0.59 * G) + (0.11 * B)
            float grayscale = result[3*i + width*j*3] * 0.3;
            grayscale += result[3*i +1 + width*j*3] * 0.59;
            grayscale += result[3*i +2 + width*j*3] * 0.11;
            image[j][i] = grayscale < 150 ;
            //image[j][i] = result[3*i + width*j*3] > (unsigned char) 150;
        }
    }
    stbi_image_free(result);

    return image;
}
*/
