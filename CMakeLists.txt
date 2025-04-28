cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0)

set(FOTOKOPIERER_VERSION 0.3.1)
set(PODOFO_VERSION 0.9.7)
set(OPENCV_VERSION 3.4.14)
set(FREETYPE_VERSION 2.10.4)

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

if (SAILFISH_FOUND)
  set(TARGET harbour-fotokopierer)
else()
  set(TARGET fotokopierer)
endif()

set(SOURCES
  src/DocumentList.cxx
  src/Document.cxx
  src/Page.cxx

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

if (SAILFISH_FOUND)
  set(OpenCV_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}-${CMAKE_SYSTEM_PROCESSOR})
  set(PODOFO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/podofo-${PODOFO_VERSION}-${CMAKE_SYSTEM_PROCESSOR})

  include(ExternalProject)

  # configure OpenCV
  set(OpenCV_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/opencv)

  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL i486)
    # we disable this flag because otherwise OpenCV will complain about some undefined
    # _mm_pause function (a compiler intrinsic?)
    set(OPENCV_EXTRA_CXX_FLAGS "-U__SSE2__")
  endif()

  ExternalProject_Add(opencv
    SOURCE_DIR ${OpenCV_SOURCE_DIR}
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rdparty
    URL "https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip"
    URL_HASH SHA256=302d3fe23b09d608d14b10212ed25649d9b6c7a2f817ccb1c8005172a479dedb
    PREFIX ${OpenCV_PREFIX}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=Release
    -DOPENCV_EXTRA_CXX_FLAGS=${OPENCV_EXTRA_CXX_FLAGS}
    -DBUILD_DOCS=OFF
    -DBUILD_EXAMPLES=OFF
    -DBUILD_FAT_JAVA_LIB=OFF
    -DBUILD_ITT=OFF
    -DBUILD_JAVA=OFF
    -DBUILD_PACKAGE=OFF
    -DBUILD_PERF_TESTS=OFF
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_TESTS=OFF
    # we must disable libjpeg-turbo because of a version clash
    -DBUILD_JPEG_TURBO_DISABLE=ON
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
    -DENABLE_PRECOMPILED_HEADERS=OFF
    -DWITH_JASPER=OFF
    -DWITH_OPENEXR=OFF
    -DWITH_WEBP=OFF)

  list(INSERT CMAKE_PREFIX_PATH 0 ${OpenCV_PREFIX})
  find_package(OpenCV QUIET COMPONENTS core imgproc)

  if (NOT OpenCV_FOUND)
    add_custom_target(opencv-3rdparty ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS opencv)
  else()
    add_custom_target(opencv-3rdparty)
  endif ()

  link_directories(${OpenCV_PREFIX}/src/opencv-build/3rdparty/lib)

  # We must use our own freetype library
  set(FREETYPE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/freetype-${FREETYPE_VERSION}-${CMAKE_SYSTEM_PROCESSOR})
  set(FREETYPE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/freetype)
  ExternalProject_Add(freetype-3rdparty
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rdparty
    URL "https://download.savannah.gnu.org/releases/freetype/freetype-${FREETYPE_VERSION}.tar.gz"
    URL_HASH SHA256=5eab795ebb23ac77001cfb68b7d4d50b5d6c7469247b0b01b2c953269f658dac
    SOURCE_DIR ${FREETYPE_SOURCE_DIR}
    PREFIX ${FREETYPE_PREFIX}
    LIST_SEPARATOR ,
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DBUILD_SHARED_LIBS:BOOL=false
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)

  link_directories(${FREETYPE_PREFIX}/lib64 ${FREETYPE_PREFIX}/lib)

  # configure PoDoFo
  find_package(PNG REQUIRED)
  find_package(OpenSSL REQUIRED)
  pkg_search_module(FONTCONFIG REQUIRED fontconfig)

  set(PODOFO_3RDPARTY_INCLUDE_PATH ${OpenCV_SOURCE_DIR}/3rdparty/libjpeg,${OpenCV_SOURCE_DIR}/3rdparty/libtiff,${FREETYPE_PREFIX}/include/freetype2)
  set(PODOFO_3RDPARTY_CMAKE_CXX_FLAGS "-isystem ${OpenCV_PREFIX}/src/opencv-build/3rdparty/libtiff")

  set(PODOFO_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/podofo)
  ExternalProject_Add(podofo-3rdparty
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rdparty
    URL "https://downloads.sourceforge.net/project/podofo/podofo/${PODOFO_VERSION}/podofo-${PODOFO_VERSION}.tar.gz"
    URL_HASH SHA256=7cf2e716daaef89647c54ffcd08940492fd40c385ef040ce7529396bfadc1eb8
    SOURCE_DIR ${PODOFO_SOURCE_DIR}
    PREFIX ${PODOFO_PREFIX}
    DEPENDS freetype-3rdparty opencv
    LIST_SEPARATOR ,
    CMAKE_ARGS
    -DPODOFO_BUILD_LIB_ONLY:BOOL=true
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
    -DCMAKE_CXX_FLAGS=${PODOFO_3RDPARTY_CMAKE_CXX_FLAGS}
    -DCMAKE_INCLUDE_PATH=${PODOFO_3RDPARTY_INCLUDE_PATH}
    -DCMAKE_LIBRARY_PATH=${FREETYPE_PREFIX}/lib,${OpenCV_PREFIX}/src/opencv-build/3rdparty/lib
    # the following should be libjpeg-turbo in case that would work ...
    -DLIBJPEG_LIBRARY_NAMES=jpeg)

  link_directories(${PODOFO_PREFIX}/lib)

  set(PODOFO_INCLUDE_DIRS ${PODOFO_PREFIX}/include)
  # use libjpeg-turbo in case that would work ...
  set(PODOFO_LIBRARIES podofo ${FONTCONFIG_LIBRARIES} OpenSSL::Crypto libtiff libjpeg PNG::PNG freetype)
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

find_package(Qt5 COMPONENTS Core Concurrent Gui Multimedia Quick)

# We need to set these directories here ... luckily we know exactly where it goes.

qt5_add_resources(RESOURCES common.qrc translations.qrc)
if (SAILFISH_FOUND)
  qt5_add_resources(RESOURCES sfos.qrc)
  add_executable(${TARGET} src/main-sfos.cxx ${SOURCES} ${RESOURCES})
  add_dependencies(${TARGET} opencv-3rdparty podofo-3rdparty)
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
