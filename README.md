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

## Contact

Talk with the developers in [#fotokopierer][IRC] on [libera.chat](https://libera.chat)

## License

Licensed under GNU GPLv3

## Build

Fotokopierer needs the [OpenCV][OpenCV] 3.4.16, [Podofo][Podofo] 0.9.8
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

You need to download the sources of OpenCV, Podofo and FreeType (the CMake file will automatically download them):

- [https://github.com/opencv/opencv/archive/opencv-3.4.16.zip](https://github.com/opencv/opencv/archive/3.4.16.zip)
- [http://sourceforge.net/projects/podofo/files/podofo/0.9.8/podofo-0.9.8.tar.gz/download](http://sourceforge.net/projects/podofo/files/podofo/0.9.8/podofo-0.9.8.tar.gz/download)
- [https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz](https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz)

Put all archives to the `rpm/` directory.

    cd path/to/fotokopierer/rpm
	cp path/to/opencv-3.4.16.zip .
	cp path/to/podofo-0.9.8.tar.gz .
	cp path/to/freetype-2.13.2.tar.gz .

Finally, build the project using the build engine:

	cd path/to/fotokopierer
	path/to/SailfishOS/bin/sfdk prepare
	path/to/SailfishOS/bin/sfdk build
    
## Download sources    

Latest development version: [harbour-fotokopierer.tar.gz][TRUNK]

Latest release version: [harbour-fotokopierer-0.4.7.tar.gz][STABLE]

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

[TRUNK]: http://chiselapp.com/user/fifr/repository/fotokopierer/tarball/harbour-fotokopierer-trunk.tar.gz?name=harbour-fotokopierer
[STABLE]: http://chiselapp.com/user/fifr/repository/fotokopierer/tarball/harbour-fotokopierer-0.4.7.tar.gz?uuid=v0.4.7
