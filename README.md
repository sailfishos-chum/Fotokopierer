# Fotokopierer

## Introduction

Fotokopierer is a document scanning application for [Sailfish OS](https://sailfishos.org) and the Desktop.

## Screenshots


<img src="https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot1.png" alt="Documents grid" title="Documents grid" width="24%">
<img src="https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot2.png" alt="Cut image" title="Cut image" width="24%">
<img src="https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot3.png" alt="Colorized image" title="Colorized image" width="24%">
<img src="https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot4.png" alt="Colorizing image" title="Colorizing image" width="24%">

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

Fotokopierer needs the [OpenCV][OpenCV] 3.4.20, [Podofo][Podofo]
0.10.4 and [FreeType][FreeType] libraries. These libraries can be
either used as shared libraries installed on your system or can be
compiled and statically linked. In order to build Fotokopierer for the
official [Sailfish OS app store][harbour], you *must* statically link
against these libraries.

In all cases you need [CMake][cmake] to build Fotokopierer.

### Get static 3rparty libraries

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
	
### Building the package	

The easiest way is to open the project in the SailfishOS SDK IDE
(QtCreator). Note that the build process is quite complicated due to
the required 3rd-party libraries. When you open the project in IDE
**for the first time** the following will happen:

1. The IDE configures the project using cmake. This will also compile
   certain 3rd-party libraries (OpenCV, FreeType). **Note that this
   happens during the configuration step** and might take a while
   (OpenCV is big).
2. Compile the project in the usual way. This will compiler other
   3rd-party libraries (PoDoFo) so it may also take some time.
   
Note that the build engine has only limited memory. This memory might
get exhausted during a parallel build. There it may necessary to limit
the number of parallel build steps (by configuring the "CMake build
step" and adding "-j2" or even "-j1" to the "Tool arguments" field),
otherwise the build will fail.

## Download sources    

Latest development version: [Fotokopierer-main.tar.gz][TRUNK]

Latest release version: [Fotokopierer-v1.0.1.tar.gz][STABLE]

## Help with translations

Translate to another language at [POEditor](https://poeditor.com/join/project/EOPT1z2FZK)

## Credits

This project uses

- Conversion between Qt and OpenCV images by 
  [Andy Maloney](https://github.com/asmaloney/asmOpenCV)
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
[STABLE]: https://codeberg.org/fifr/Fotokopierer/archive/v1.0.1.tar.gz 
