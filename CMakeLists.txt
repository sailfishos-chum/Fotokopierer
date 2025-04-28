cmake_policy(SET CMP0048 NEW)

project (fotokopierer VERSION 0.1 LANGUAGES CXX)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp)

if (SAILFISH_FOUND)
  set(TARGET harbour-fotokopierer)
else()
  set(TARGET fotokopierer)
endif()

set(SOURCES
  src/AsyncImage.cxx
  src/BaseImage.cxx
  src/ColorizeImage.cxx
  src/Convert.cxx
  src/CutImage.cxx
  src/PlainImage.cxx
  src/RotImage.cxx
  src/TransformWorker.cxx
  src/Util.cxx
  src/ZoomImage.cxx
  src/init.cxx)

# We need to set this directory here ... luckily we know exactly where it goes.
link_directories(${CMAKE_CURRENT_BINARY_DIR}/podofo/lib)
add_executable(${TARGET} ${SOURCES})

if (SAILFISH_FOUND)
  set(OpenCV_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/opencv-3.4.0)
  set(PODOFO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/podofo-0.9.6)

  include(ExternalProject)

  # configure OpenCV
  set(OpenCV_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/opencv)

  ExternalProject_Add(opencv
    SOURCE_DIR ${OpenCV_SOURCE_DIR}
    # URL "https://github.com/opencv/opencv/archive/3.4.0.zip"
    PREFIX ${OpenCV_PREFIX}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DBUILD_DOCS=OFF
    -DBUILD_EXAMPLES=OFF
    -DBUILD_FAT_JAVA_LIB=OFF
    -DBUILD_ITT=OFF
    -DBUILD_JAVA=OFF
    -DBUILD_PACKAGE=OFF
    -DBUILD_PERF_TESTS=OFF
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_TESTS=OFF
    -DBUILD_opencv_apps=OFF
    -DBUILD_opencv_calib3d=OFF
    -DBUILD_opencv_dnn=OFF
    -DBUILD_opencv_features2d=OFF
    -DBUILD_opencv_flann=OFF
    -DBUILD_opencv_highgui=OFF
    -DBUILD_opencv_imgcodecs=OFF
    -DBUILD_opencv_ml=OFF
    -DBUILD_opencv_objdetect=OFF
    -DBUILD_opencv_photo=OFF
    -DBUILD_opencv_python2=OFF
    -DBUILD_opencv_python_bindings_generator=OFF
    -DBUILD_opencv_shape=OFF
    -DBUILD_opencv_stitching=OFF
    -DBUILD_opencv_superres=OFF
    -DBUILD_opencv_ts=OFF
    -DBUILD_opencv_video=OFF
    -DBUILD_opencv_videoio=OFF
    -DBUILD_opencv_videostab=OFF
    -DBUILD_opencv_world=OFF
    -DWITH_JASPER=OFF
    -DWITH_OPENEXR=OFF
    -DWITH_WEBP=OFF)

  list(INSERT CMAKE_PREFIX_PATH 0 ${OpenCV_PREFIX})
  find_package(OpenCV QUIET COMPONENTS core imgproc)

  if (NOT OpenCV_FOUND)
    add_custom_target(Rescan ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS opencv)
  else()
    add_custom_target(Rescan)
  endif ()

  # We must use our own freetype library
  set(FREETYPE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/freetype-2.9.1)
  set(FREETYPE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/freetype)
  ExternalProject_Add(freetype-3rdparty
    SOURCE_DIR ${FREETYPE_SOURCE_DIR}
    PREFIX ${FREETYPE_PREFIX}
    LIST_SEPARATOR ,
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DBUILD_SHARED_LIBS:BOOL=false
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)

  # configure PoDoFo
  find_package(OpenSSL REQUIRED)
  pkg_search_module(FONTCONFIG REQUIRED fontconfig)

  set(PODOFO_3RDPARTY_INCLUDE_PATH ${OpenCV_SOURCE_DIR}/3rdparty/libjpeg,${OpenCV_SOURCE_DIR}/3rdparty/libtiff,${FREETYPE_PREFIX}/include)
  set(PODOFO_3RDPARTY_CMAKE_CXX_FLAGS "-isystem ${OpenCV_PREFIX}/src/opencv-build/3rdparty/libtiff")

  set(PODOFO_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/podofo)
  ExternalProject_Add(podofo-3rdparty
    SOURCE_DIR ${PODOFO_SOURCE_DIR}
    PREFIX ${PODOFO_PREFIX}
    DEPENDS freetype-3rdparty opencv
    LIST_SEPARATOR ,
    CMAKE_ARGS
    -DPODOFO_BUILD_LIB_ONLY:BOOL=true
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
    -DCMAKE_CXX_FLAGS=${PODOFO_3RDPARTY_CMAKE_CXX_FLAGS}
    -DCMAKE_INCLUDE_PATH=${PODOFO_3RDPARTY_INCLUDE_PATH}
    -DCMAKE_LIBRARY_PATH=${OpenCV_PREFIX}/src/opencv-build/3rdparty/lib)

  set(PODOFO_INCLUDE_DIRS ${PODOFO_PREFIX}/include)
  set(PODOFO_LIBRARIES podofo ${FONTCONFIG_LIBRARIES} libjpeg OpenSSL::Crypto freetype)

  add_dependencies(${TARGET} Rescan podofo-3rdparty)
else()
  set(TARGET fotokopierer)

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
endif()

find_package(Qt5 COMPONENTS Core Gui Quick)

qt5_add_resources(RESOURCES common.qrc translations.qrc)

if (SAILFISH_FOUND)
  qt5_add_resources(RESOURCES sfos.qrc)
  set_property(TARGET ${TARGET} APPEND PROPERTY SOURCES src/main-sfos.cxx ${RESOURCES})
else()
  qt5_add_resources(RESOURCES desktop.qrc)
  set_property(TARGET ${TARGET} APPEND PROPERTY SOURCES src/main-desktop.cxx ${RESOURCES})
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

qt5_use_modules(${TARGET} Core Gui Quick)
target_link_libraries(${TARGET} ${SAILFISH_LDFLAGS} ${OpenCV_LIBS} ${PODOFO_LIBRARIES})

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
# install(DIRECTORY qml DESTINATION share/${TARGET})
# install(DIRECTORY translations DESTINATION share/${TARGET} FILES_MATCHING PATTERN "*.qm")
install(FILES ${TARGET}.desktop DESTINATION share/applications)
install(FILES ${TARGET}.png DESTINATION share/icons/hicolor/86x86/apps)
