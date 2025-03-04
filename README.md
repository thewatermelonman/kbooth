# kbooth
Photobooth Application for printing to ESC/POS Printers.

Tested with 

# External Libraries
- [SDL3](https://wiki.libsdl.org/SDL3) Sam Lantinga
- [SDLimages](https://wiki.libsdl.org/SDL3_image) Sam Lantinga 
- [SDLttf](https://wiki.libsdl.org/SDL3_ttf) Sam Lantinga
- [Dear ImGui](ttps://github.com/ocornut/imgui) Omar Cornut
- [libdither](ttps://github.com/robertkist/libdither) Robert Kist
- [libusb](ttps://libusb.info/)
- [simpleini](ttps://github.com/brofield/simpleini) Brodie Thiesfield

# Build

```bash
git clone --recurse-submodules --shallow-submodules https://github.com/thewatermelonman/kbooth.git
cd kbooth
```

```bash
 mkdir build && cd build
 cmake ..
 make 
```

## Requirements:
- cmake
- [freetype](https://freetype.org) 
``` -DFREETYPE_INCLUDE_DIRS=<your-freetype-dir> -DFREETYPE_LIBRARY=<your-freetype-lib>```
- ESC/POS Printer
    - libusb compatible driver (Windows: use [Zadig](https://zadig.akeo.ie) to install driver )
