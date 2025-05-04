cmake_minimum_required(VERSION 3.5)

set(FOTOKOPIERER_VERSION 1.0.1)

project(fotokopierer VERSION ${FOTOKOPIERER_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package (Qt5 COMPONENTS Core Concurrent Gui Multimedia Quick LinguistTools REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(WITH_PODOFO On CACHE BOOL "Build and use PoDoFo pdf library")

set(OPENCV_VERSION "3.4.20")

if (WITH_PODOFO)
    set(PODOFO_VERSION "0.10.4")
    set(FREETYPE_VERSION "2.13.3")
endif ()

add_executable(harbour-fotokopierer
	src/main-sfos.cxx
    src/Clipboard.cxx
    src/ColorizeChooser.cxx
    src/ColorizeView.cxx
    src/Convert.cxx
    src/CutView.cxx
    src/Document.cxx
    src/DocumentList.cxx
    src/EdgeDetection.cxx
    src/Fotokopierer.cxx
    src/init.cxx
    src/Page.cxx
    src/ScanImage.cxx
    src/ScanImageView.cxx
    src/Scanner.cxx
	src/ZoomImage.cxx)

target_compile_definitions(harbour-fotokopierer PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(harbour-fotokopierer PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}
>)

include(./ThirdParty.cmake)

find_package(OpenCV REQUIRED core imgproc PATHS "${THIRD_PARTY_INSTALL_DIR}/OpenCV" NO_DEFAULT_PATH)

##############################

# PoDoFo and depedencies

if (WITH_PODOFO)
    set(CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libjpeg;${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libtiff;${CMAKE_BINARY_DIR}/ExternalProjects/OpenCV/build/OpenCV-prefix/src/OpenCV-build/3rdparty/libtiff)
    set(CMAKE_LIBRARY_PATH ${THIRD_PARTY_INSTALL_DIR}/OpenCV/share/OpenCV/3rdparty/lib)

    find_package(PNG REQUIRED)
    find_package(JPEG REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(Fontconfig REQUIRED)
    find_package(LibXml2 REQUIRED)
    find_package(FreeType REQUIRED PATHS "${THIRD_PARTY_INSTALL_DIR}/FreeType" NO_DEFAULT_PATH)

    include_directories(SYSTEM ${CMAKE_BINARY_DIR}/ExternalProjects/OpenCV/build/OpenCV-prefix/src/OpenCV-build/3rdparty/libtiff)
    set(PODOFO_BUILD_LIB_ONLY TRUE CACHE BOOL "" FORCE)
    set(PODOFO_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
    add_subdirectory(3rdparty/podofo-${PODOFO_VERSION} EXCLUDE_FROM_ALL)
    set_property(TARGET podofo_static PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_property(TARGET podofo_private PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set(PODOFO_LIBRARIES podofo::podofo)

    set_property(SOURCE src/Document.cxx APPEND PROPERTY COMPILE_DEFINITIONS USE_PODOFO=1)
endif ()
##############################

# Add dependencies to app

add_dependencies(harbour-fotokopierer ${OpenCV_LIBS})
target_link_libraries(harbour-fotokopierer
    Qt5::Quick
    ${SAILFISH_LDFLAGS}
    ${PODOFO_LIBRARIES}
    ${OpenCV_LIBS}
)

set_property(SOURCE src/Fotokopierer.cxx APPEND PROPERTY COMPILE_DEFINITIONS
  FOTOKOPIERER_AUTHOR="Frank Fischer"
  FOTOKOPIERER_VERSION="${FOTOKOPIERER_VERSION}"
  PODOFO_VERSION="${PODOFO_VERSION}"
  OPENCV_VERSION="${OpenCV_VERSION}"
  FREETYPE_VERSION="${FREETYPE_VERSION}"
  )

set_property(TARGET harbour-fotokopierer PROPERTY
  INCLUDE_DIRECTORIES
  "${SAILFISH_INCLUDE_DIRS}"
  "${OpenCV_INCLUDE_DIRS}"
  "${PODOFO_INCLUDE_DIRS}")

set_property(TARGET harbour-fotokopierer APPEND PROPERTY COMPILE_DEFINITIONS
  QT_DEPRECATED_WARNINGS
  QT_NO_URL_CAST_FROM_STRING
  QT_NO_CAST_TO_ASCII
  QT_NO_CAST_FROM_ASCII
  QT_NO_CAST_FROM_BYTEARRAY
  QT_STRICT_ITERATORS
  QT_USE_FAST_CONCATENATION
  QT_USE_FAST_OPERATOR_PLUS
)

qt5_use_modules(harbour-fotokopierer Core Gui Multimedia Quick)

FILE(GLOB TsFiles "translations/*.ts")
qt5_add_translation(QmFiles ${TsFiles})
add_custom_target(translations ALL DEPENDS ${QmFiles})
add_dependencies(harbour-fotokopierer translations)

###############################
# Installation

install(TARGETS harbour-fotokopierer
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml/common
	DESTINATION share/harbour-fotokopierer/qml
)
install(DIRECTORY qml/sfos
	DESTINATION share/harbour-fotokopierer/qml
)
install(FILES ${QmFiles}
	DESTINATION share/harbour-fotokopierer/translations
)
install(FILES harbour-fotokopierer.desktop
    DESTINATION share/applications
)
install(DIRECTORY icons
	DESTINATION share/harbour-fotokopierer
	FILES_MATCHING PATTERN "*.svg"
)
install(FILES icons/86x86/harbour-fotokopierer.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-fotokopierer.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-fotokopierer.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-fotokopierer.png
    DESTINATION share/icons/hicolor/172x172/apps
)

# Get the other files reachable from the project tree in Qt Creator
add_custom_target(distfiles
    SOURCES
		harbour-fotokopierer.desktop
        qml/common/ColorizeChooserItem.qml
        qml/common/CornerMarker.qml
        qml/common/CutImageView.qml
        qml/common/DragDelegate.qml
        qml/sfos/CoverPage.qml
        qml/sfos/DeletableItem.qml
        qml/sfos/DocumentDelegate.qml
        qml/sfos/ErrorOverlay.qml
        qml/sfos/main.qml
        qml/sfos/PageDelegate.qml
        qml/sfos/pages/AboutPage.qml
        qml/sfos/pages/ColorizePage.qml
        qml/sfos/pages/CutPage.qml
        qml/sfos/pages/DocumentPage.qml
        qml/sfos/pages/DocumentsPage.qml
        qml/sfos/pages/NewImagePage.qml
        qml/sfos/pages/PagePage.qml
        qml/sfos/pages/RenamePage.qml
        qml/sfos/ValueSlider.qml
		rpm/harbour-fotokopierer.changes
		rpm/harbour-fotokopierer.spec
        ${TsFiles})

# Tell Qt Creator where the application executable(s) would be located on the
# device.
#
# It is not necessary to list other deployables than executables (runtime
# targets) here. The deployment process of Sailfish OS projects is opaque to
# Qt Creator and the information contained in QtCreatorDeployment.txt is only
# used to locate the executable associated with the active run configuration
# on the device in order to run it.
#
# Search the Qt Creator Manual to learn about the QtCreatorDeployment.txt file
# format.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/fotokopierer:bin
")

