# Fotokopierer

## Introduction

Fotokopierer is a document scanning application for [Sailfish OS](https://sailfishos.org) and the Desktop.

## Authors

Frank Fischer <frank-fischer@shadow-soft.de>

planetos (Icons)

G. Yavorov, Standjata (Bulgarian translation)

Godfried Cobben (Dutch translation)

pherjung (French translation)

holask (Slovak translation)

Åke Engelbrektson (Swedish translation)

## License

Licensed under GNU GPLv3

## Build

Fotokopierer needs the [OpenCV][OpenCV] 3.4.20, [Podofo][Podofo] 0.10.4
and [FreeType][FreeType] libraries. These libraries can be either used
as shared libraries installed on your system or can be compiled and
statically linked. In order to build Fotokopierer for the official
[Sailfish OS app store][harbour], you *must* statically link against
these libraries.

In all cases you need [CMake][cmake] to build Fotokopierer.

### Build with shared libraries in the Sailfish OS build engine

	cd path/to/fotokopierer
	path/to/SailfishOS/bin/sfdk -c target=SailfishOS-4.0.1.48-i486 build

In order to create an rpm package execute

	path/to/SailfishOS/bin/sfdk -c target=SailfishOS-4.0.1.48-i486 package

Note that the OS version and target might differ for you.

### Build with static libraries in the Sailfish OS build engine

You need to download the sources of OpenCV, Podofo and FreeType:

- [https://github.com/opencv/opencv/archive/refs/tags/3.4.20.tar.gz](https://github.com/opencv/opencv/archive/refs/tags/3.4.20.tar.gz)
- [http://sourceforge.net/projects/podofo/files/podofo/0.10.4/podofo-0.10.4.tar.gz/download](http://sourceforge.net/projects/podofo/files/podofo/0.10.4/podofo-0.10.4.tar.gz/download)
- [https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz](https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz)

Extract all archives to the `3rdparty/` directory.

    cd path/to/fotokopierer
	mkdir -p 3rdparty
	cd 3rdparty
	tar -xzf path/to/opencv-3.4.20.tar.gz
	tar -xzf path/to/podofo-0.10.4.tar.gz
	tar -xzf path/to/freetype-2.13.3.tar.gz

Finally, build the project using the build engine:

	cd path/to/fotokopierer
	path/to/SailfishOS/bin/sfdk prepare
	path/to/SailfishOS/bin/sfdk build
    
## Download sources    

Latest development version: [Fotokopierer-main.tar.gz][TRUNK]

Latest release version: [Fotokopierer-v0.4.8.tar.gz][STABLE]

## Help with translations

Translate to another language at [POEditor](https://poeditor.com/join/project/EOPT1z2FZK)

## Credits

This project uses

- Conversion between Qt and OpenCV images by [Andy Maloney](https://github.com/asmaloney/asmOpenCV)
- [OpenCV][OpenCV] for image processing
- [Podofo][Podofo] for PDF-handling
- [FreeType][FreeType] for font rendering in PDF files


[CMake]: https://cmake.org
[OpenCV]: https://opencv.org
[Podofo]: http://podofo.sourceforge.net
[FreeType]: https://www.freetype.org
[SFOS]: https://sailfishos.org
[harbour]: https://harbour.jolla.com/
[IRC]: https://web.libera.chat/#fotokopierer

[TRUNK]: https://codeberg.org/fifr/Fotokopierer/archive/main.tar.gz
[STABLE]: https://codeberg.org/fifr/Fotokopierer/archive/v0.4.8.tar.gz 
