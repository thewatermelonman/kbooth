# kbooth
Photobooth Application for printing to ESC/POS Printers.

Tested with 

# External Libraries
- SDL3 -  [https://wiki.libsdl.org/SDL3] Sam Lantinga
- SDLimages - [https://wiki.libsdl.org/SDL3_ttf] Sam Lantinga 
- SDLttf -  [https://wiki.libsdl.org/SDL3_image] Sam Lantinga
- Dear ImGui - [https://github.com/ocornut/imgui] Omar Cornut
- libdither - [https://github.com/robertkist/libdither] Robert Kist
- libusb - [https://libusb.info/]  

# Build

```bash
git clone --recurse-submodules --shallow-submodules https://github.com/thewatermelonman/kbooth.git
```

```bash
 mkdir build && cd build
 cmake ..
 make 
```

## Requirements:
libusb - [https://libusb.info/]
- on Windows Zadig can be used to install WinUSB/libusb Drivers
