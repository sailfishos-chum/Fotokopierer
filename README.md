# Fotokopierer

## Introduction

Fotokopierer is a document scanning application for [Sailfish OS](https://sailfishos.org) and the Desktop.

## Author

Frank Fischer <frank-fischer@shadow-soft.de>

## Contact

Talk with the developers in [#fotokopierer][IRC] on [Freenode](http://freenode.net)

## License

Licensed under GNU GPLv3

## Build

Fotokopierer needs the [OpenCV][OpenCV] 3.4.0 library. These
libraries can be either used as shared libraries installed on your
system or can be compiled and statically linked. In order to build
Fotokopierer for the official [Sailfish OS app store][harbour], you
*must* statically link against these libraries.

In all cases you need [CMake][cmake] to build Fotokopierer.

### Build with shared libraries in the Sailfish OS build engine

	cd path/to/fotokopierer
	mb2 -t SailfishOS-2.1.3.7-i486 build

Note that the OS version and target might differ for you.

### Build with static libraries in the Sailfish OS build engine

You need to download the sources of OpenCV (the CMake file will automatically download them):

- [https://github.com/opencv/opencv/archive/3.4.0.zip](https://github.com/opencv/opencv/archive/3.4.0.zip)

Extract both archives to the `3rdparty` subdirectory.

    cd path/to/fotokopierer
	mkdir 3rdparty
	cd 3rdparty
	unzip path/to/opencv-3.4.0.zip

Finally, build the project as with shared libraries. The CMake build
script will automatically compile OpenCV.

	cd path/to/fotokopierer
	mb2 -t SailfishOS-2.1.3.7-i486 build
    
## Download sources    

Latest development version: [harbour-fotokopierer.tar.gz][TRUNK]

## Help with translations

Translate to another language at [POEditor](https://poeditor.com/join/project/EOPT1z2FZK)

## Credits

This project uses

- Conversion between Qt and OpenCV images by [Andy Maloney](https://github.com/asmaloney/asmOpenCV)
- [OpenCV][OpenCV] for image processing


[CMake]: https://cmake.org
[OpenCV]: https://opencv.org
[SFOS]: https://sailfishos.org
[harbour]: https://harbour.jolla.com/
[IRC]: https://kiwiirc.com/nextclient/irc.freenode.net/#fotokopierer

[TRUNK]: http://chiselapp.com/user/fifr/repository/fotokopierer/tarball/harbour-fotokopierer-trunk.tar.gz?name=harbour-fotokopierer
