cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0)

set(FOTOKOPIERER_VERSION 0.4.5)
set(PODOFO_VERSION 0.9.7)
set(OPENCV_VERSION 3.4.16)
set(FREETYPE_VERSION 2.11.1)

project(fotokopierer VERSION ${FOTOKOPIERER_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp)

find_package(OpenSSL 1.1)

if (SAILFISH_FOUND)
  set(TARGET harbour-fotokopierer)
else()
  set(TARGET fotokopierer)
endif()

set(SOURCES
  src/DocumentList.cxx
  src/Document.cxx
  src/Page.cxx
  src/Clipboard.cxx

  src/Scanner.cxx
  src/ScanImage.cxx
  src/ScanImageView.cxx
  src/CutView.cxx
  src/ColorizeView.cxx
  src/ColorizeChooser.cxx
  src/ZoomImage.cxx

  src/Fotokopierer.cxx
  src/Convert.cxx
  src/EdgeDetection.cxx

  src/init.cxx)

set_property(SOURCE src/Fotokopierer.cxx APPEND PROPERTY COMPILE_DEFINITIONS
  FOTOKOPIERER_AUTHOR="Frank Fischer"
  FOTOKOPIERER_VERSION="${FOTOKOPIERER_VERSION}"
  PODOFO_VERSION="${PODOFO_VERSION}"
  OPENCV_VERSION="${OPENCV_VERSION}"
  FREETYPE_VERSION="${FREETYPE_VERSION}"
  )


find_package(OpenCV REQUIRED core imgproc)

find_path(PODOFO_INCLUDE_DIRS NAMES podofo/podofo.h)
if (NOT PODOFO_INCLUDE_DIRS)
  message(FATAL_ERROR "Could not find the PoDoFo include directory")
endif ()

find_library(PODOFO_LIBRARIES NAMES podofo)
if (NOT PODOFO_LIBRARIES)
  message(FATAL_ERROR "Could not find the PoDoFo library")
endif ()
message(STATUS "Found PoDoFo: ${PODOFO_LIBRARIES}")

if (SAILFISH_FOUND)
  find_package(PNG REQUIRED)
  find_package(OpenSSL REQUIRED)
  pkg_search_module(FONTCONFIG REQUIRED fontconfig)
  set(PODOFO_LIBRARIES ${PODOFO_LIBRARIES} ${FONTCONFIG_LIBRARIES} OpenSSL::Crypto libtiff libjpeg PNG::PNG freetype)
endif()

find_package(Qt5 COMPONENTS Core Concurrent Gui Multimedia Quick)

# We need to set these directories here ... luckily we know exactly where it goes.

qt5_add_resources(RESOURCES common.qrc translations.qrc)
if (SAILFISH_FOUND)
  qt5_add_resources(RESOURCES sfos.qrc)
  add_executable(${TARGET} src/main-sfos.cxx ${SOURCES} ${RESOURCES})
  add_dependencies(${TARGET} ${OpenCV_LIBS})
  target_link_libraries(${TARGET} ${PODOFO_LIBRARIES})
else()
  qt5_add_resources(RESOURCES desktop.qrc)
  add_executable(${TARGET} src/main-desktop.cxx ${SOURCES} ${RESOURCES})

  add_executable(edgedetect src/main-edgedetect.cxx src/Convert.cxx src/EdgeDetection.cxx)
  qt5_use_modules(edgedetect Core Gui Multimedia Quick Widgets)
  target_link_libraries(edgedetect ${SAILFISH_LDFLAGS} ${OpenCV_LIBS} ${PODOFO_LIBRARIES})
endif()

set_property(TARGET ${TARGET} PROPERTY
  INCLUDE_DIRECTORIES
  "${SAILFISH_INCLUDE_DIRS}"
  "${OpenCV_INCLUDE_DIRS}"
  "${PODOFO_INCLUDE_DIRS}")
set_property(TARGET ${TARGET} APPEND PROPERTY COMPILE_DEFINITIONS
  QT_DEPRECATED_WARNINGS
  QT_NO_URL_CAST_FROM_STRING
  QT_NO_CAST_TO_ASCII
  QT_NO_CAST_FROM_ASCII
  QT_NO_CAST_FROM_BYTEARRAY
  QT_STRICT_ITERATORS
  QT_USE_FAST_CONCATENATION
  QT_USE_FAST_OPERATOR_PLUS
  )

qt5_use_modules(${TARGET} Core Gui Multimedia Quick)
target_link_libraries(${TARGET} ${SAILFISH_LDFLAGS} ${OpenCV_LIBS} ${PODOFO_LIBRARIES})

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)

# install(DIRECTORY qml DESTINATION share/${TARGET})
# install(DIRECTORY translations DESTINATION share/${TARGET} FILES_MATCHING PATTERN "*.qm")

install(FILES ${TARGET}.desktop DESTINATION share/applications)
install(FILES icons/86x86/${TARGET}.png DESTINATION share/icons/hicolor/86x86/apps)
install(FILES icons/108x108/${TARGET}.png DESTINATION share/icons/hicolor/108x108/apps)
install(FILES icons/128x128/${TARGET}.png DESTINATION share/icons/hicolor/128x128/apps)
install(FILES icons/172x172/${TARGET}.png DESTINATION share/icons/hicolor/172x172/apps)
