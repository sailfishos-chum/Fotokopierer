cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.0)

set(FOTOKOPIERER_VERSION 0.1.0)
set(OPENCV_VERSION 3.4.0)

project(fotokopierer VERSION 0.1.0 LANGUAGES CXX)

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
  src/ColorizeFilter.cxx
  src/Convert.cxx
  src/CutFilter.cxx
  src/Document.cxx
  src/DocumentList.cxx
  src/Filter.cxx
  src/FilterImage.cxx
  src/Fotokopierer.cxx
  src/Page.cxx
  src/RotateFilter.cxx
  src/ScanImage.cxx
  src/ZoomImage.cxx
  src/init.cxx)

set_property(SOURCE src/Fotokopierer.cxx APPEND PROPERTY COMPILE_DEFINITIONS
  FOTOKOPIERER_AUTHOR="Frank Fischer"
  FOTOKOPIERER_VERSION="${FOTOKOPIERER_VERSION}"
  OPENCV_VERSION="${OPENCV_VERSION}"
  )

if (SAILFISH_FOUND)
  set(OpenCV_SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION})

  include(ExternalProject)

  # configure OpenCV
  set(OpenCV_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/opencv)

  ExternalProject_Add(opencv
    SOURCE_DIR ${OpenCV_SOURCE_DIR}
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/3rdparty
    URL "https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip"
    URL_HASH SHA256=ae8a9f468b7ee3822a62d20d356c69f2b32330a231e738ac2914f332766611b7
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
    add_custom_target(opencv-3rdparty ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS opencv)
  else()
    add_custom_target(opencv-3rdparty)
  endif ()
else()
  set(TARGET fotokopierer)

  find_package(OpenCV REQUIRED core imgproc)
endif()

find_package(Qt5 COMPONENTS Core Concurrent Gui Quick)

# We need to set these directories here ... luckily we know exactly where it goes.

qt5_add_resources(RESOURCES common.qrc translations.qrc)
if (SAILFISH_FOUND)
  qt5_add_resources(RESOURCES sfos.qrc)
  add_executable(${TARGET} src/main-sfos.cxx ${SOURCES} ${RESOURCES})
  add_dependencies(${TARGET} opencv-3rdparty)
else()
  qt5_add_resources(RESOURCES desktop.qrc)
  add_executable(${TARGET} src/main-desktop.cxx ${SOURCES} ${RESOURCES})
endif()

set_property(TARGET ${TARGET} PROPERTY
  INCLUDE_DIRECTORIES
  "${SAILFISH_INCLUDE_DIRS}"
  "${OpenCV_INCLUDE_DIRS}")
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
target_link_libraries(${TARGET} ${SAILFISH_LDFLAGS} ${OpenCV_LIBS})

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
# install(DIRECTORY qml DESTINATION share/${TARGET})
# install(DIRECTORY translations DESTINATION share/${TARGET} FILES_MATCHING PATTERN "*.qm")
install(FILES ${TARGET}.desktop DESTINATION share/applications)
install(FILES ${TARGET}.png DESTINATION share/icons/hicolor/86x86/apps)
