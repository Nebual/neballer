Neballer
========

Breakout clone; mostly just a learning experience for C and SDL

# Dependancies
* SDL2, SDL2_image, SDL2_mixer

## Linux Dependancies
* sudo apt-get install libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-mixer-2.0-0 libsdl2-mixer-dev libsdl2-image-dev

## Windows Dependancies: 
* Get the MinGW64 toolkit ( http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/4.8.2/threads-win32/seh/ works)
* Extract it to C:/MinGW64
* Add C:/MinGW64/bin to your path
* Extract to C:/MinGW64 SDL2 (SDL2-devel-2.0.3-mingw.tar.gz from http://www.libsdl.org/download-2.0.php )
* Extract to C:/MinGW64 SDL2_image (SDL2_image-devel-2.0.0-mingw.tar.gz from https://www.libsdl.org/projects/SDL_image/ )
* Extract to C:/MinGW64 SDL2_mixer (SDL2_mixer-devel-2.0.0-mingw.tar.gz from http://www.libsdl.org/projects/SDL_mixer/ )


# Building
* `make` or `make run`