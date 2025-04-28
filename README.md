# Fotokopierer

## Introduction

Fotokopierer is a document scanning application for [Sailfish OS](https://sailfishos.org) and the Desktop.

## Author

Frank Fischer <frank-fischer@shadow-soft.de>

## License

Licensed under GNU GPLv3

## Build

Fotokopierer needs the [OpenCV][OpenCV] 3.4.0 and [Podofo][Podofo] 0.9.6 libraries. These
libraries can be either used as shared libraries installed on your
system or can be compiled and statically linked. In order to build
Fotokopierer for the official [Sailfish OS app store][harbour], you
*must* statically link against these libraries.

In all cases you need [CMake][cmake] to build Fotokopierer.

### Build with shared libraries on a desktop

	cd path/to/fotokopierer
	mkdir build
	cd build
	cmake ..
	make

### Build with shared libraries in the Sailfish OS build engine

	cd path/to/fotokopierer
	mb2 -t SailfishOS-2.1.3.7-i486 build

Note that the OS version and target might differ for you.

### Build with static libraries in the Sailfish OS build engine

You need to download the sources of OpenCV and Podofo:

- [https://github.com/opencv/opencv/archive/3.4.0.zip](https://github.com/opencv/opencv/archive/3.4.0.zip)
- [http://sourceforge.net/projects/podofo/files/podofo/0.9.6/podofo-0.9.6.tar.gz/download](http://sourceforge.net/projects/podofo/files/podofo/0.9.6/podofo-0.9.6.tar.gz/download)

Extract both archives to the `3rdparty` subdirectory.

    cd path/to/fotokopierer
	mkdir 3rdparty
	cd 3rdparty
	unzip path/to/opencv-3.4.0.zip
	tar -xzf path/to/podofo-0.9.6.tar.gz

Finally, build the project as with shared libraries. The CMake build
script will automatically compile OpenCV and Podofo.

	cd path/to/fotokopierer
	mb2 -t SailfishOS-2.1.3.7-i486 build

## Credits

This project uses

- Conversion between Qt and OpenCV images by [Andy Maloney](https://github.com/asmaloney/asmOpenCV)
- [OpenCV][OpenCV] for image processing
- [Podofo][Podofo] for PDF-handling


[CMake]: https://cmake.org
[OpenCV]: https://opencv.org
[Podofo]: http://podofo.sourceforge.net
[SFOS]: https://sailfishos.org
[harbour]: https://harbour.jolla.com/
