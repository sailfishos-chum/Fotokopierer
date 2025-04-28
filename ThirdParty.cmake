# --------------------------------------------------

function (build_external_project target dir_name maketargets)

    set(CMAKELIST_CONTENT "
        cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

        project(build_external_project)

        include(ExternalProject)
        ExternalProject_add(${target}
            SOURCE_DIR \"${THIRD_PARTY_DIR}/${dir_name}\"
            BUILD_COMMAND cmake --build . -- ${maketargets}
            CMAKE_GENERATOR \"${CMAKE_GENERATOR}\"
            CMAKE_GENERATOR_PLATFORM \"${CMAKE_GENERATOR_PLATFORM}\"
            CMAKE_GENERATOR_TOOLSET \"${CMAKE_GENERATOR_TOOLSET}\"
            CMAKE_GENERATOR_INSTANCE \"${CMAKE_GENERATOR_INSTANCE}\"
            LIST_SEPARATOR "|"
            CMAKE_ARGS ${ARGN})

        add_custom_target(build_external_project)
        add_dependencies(build_external_project ${target})
    ")

    set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/${target}")

    file(WRITE "${TARGET_DIR}/CMakeLists.txt" "${CMAKELIST_CONTENT}")

    file(MAKE_DIRECTORY "${TARGET_DIR}" "${TARGET_DIR}/build")

    execute_process(COMMAND ${CMAKE_COMMAND}
        -G "${CMAKE_GENERATOR}"
        -A "${CMAKE_GENERATOR_PLATFORM}"
        -T "${CMAKE_GENERATOR_TOOLSET}"
        ..
        WORKING_DIRECTORY "${TARGET_DIR}/build")

    execute_process(COMMAND ${CMAKE_COMMAND}
        --build .
        --config ${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY "${TARGET_DIR}/build")

endfunction()

#------------------------------------------------------------------------------------------

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_LIST_DIR}/3rdparty")
set(THIRD_PARTY_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdparty")

#------------------------------------------------------------------------------------------

# OpenCV.

set(OPENCV_OPTIONS "")

list(APPEND OPENCV_OPTIONS -D CMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}/OpenCV)
list(APPEND OPENCV_OPTIONS -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
list(APPEND OPENCV_OPTIONS -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

list(APPEND OPENCV_OPTIONS -D OPENCV_EXTRA_CXX_FLAGS="-U__SSE2__")
list(APPEND OPENCV_OPTIONS -D BUILD_DOCS=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_EXAMPLES=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_FAT_JAVA_LIB=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_ITT=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_JAVA=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_PACKAGE=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_PERF_TESTS=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_SHARED_LIBS=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_TESTS=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_PROTOBUF=Off)
list(APPEND OPENCV_OPTIONS -D BUILD_TIFF=Off)
list(APPEND OPENCV_OPTIONS -D BUILD_JPEG=Off)
list(APPEND OPENCV_OPTIONS -D BUILD_JPEG_TURBO_DISABLE=ON)
list(APPEND OPENCV_OPTIONS -D BUILD_PNG=Off)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_apps=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_calib3d=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_dnn=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_features2d=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_flann=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_highgui=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_imgcodecs=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_ml=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_objdetect=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_photo=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_python2=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_python_bindings_generator=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_shape=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_stitching=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_superres=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_ts=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_video=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_videoio=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_videostab=OFF)
list(APPEND OPENCV_OPTIONS -D BUILD_opencv_world=OFF)
list(APPEND OPENCV_OPTIONS -D ENABLE_PRECOMPILED_HEADERS=OFF)
list(APPEND OPENCV_OPTIONS -D WITH_JASPER=OFF)
list(APPEND OPENCV_OPTIONS -D WITH_OPENEXR=OFF)
list(APPEND OPENCV_OPTIONS -D WITH_QUIRC=OFF)
list(APPEND OPENCV_OPTIONS -D WITH_WEBP=OFF)

build_external_project(OpenCV "opencv-${OPENCV_VERSION}" "all libjpeg libtiff" ${OPENCV_OPTIONS})

set(OpenCV_STATIC ON)

#------------------------------------------------------------------------------------------

# FreeType

set(FREETYPE_OPTIONS "")

list(APPEND FREETYPE_OPTIONS -D CMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}/FreeType)
list(APPEND FREETYPE_OPTIONS -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
list(APPEND FREETYPE_OPTIONS -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
list(APPEND FREETYPE_OPTIONS -D BUILD_SHARED_LIBS:BOOL=false)
list(APPEND FREETYPE_OPTIONS -D CMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)

build_external_project(FreeType "freetype-${FREETYPE_VERSION}" all ${FREETYPE_OPTIONS})

#------------------------------------------------------------------------------------------

# PoDoFo

set(PODOFO_OPTIONS "")
list(APPEND PODOFO_OPTIONS -D CMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY_INSTALL_DIR}/PoDoFo)
list(APPEND PODOFO_OPTIONS -D CMAKE_PREFIX_PATH:PATH=${THIRD_PARTY_INSTALL_DIR}/FreeType)
list(APPEND PODOFO_OPTIONS -D CMAKE_BUILD_TYPE=Release)
list(APPEND PODOFO_OPTIONS -D CMAKE_POSITION_INDEPENDENT_CODE:BOOL=true)
list(APPEND PODOFO_OPTIONS -D PODOFO_BUILD_LIB_ONLY:BOOL=true)
list(APPEND PODOFO_OPTIONS -D "CMAKE_INCLUDE_PATH:PATH=${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libjpeg|${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libtiff|${THIRD_PARTY_INSTALL_DIR}/include/freetype2/freetype")
list(APPEND PODOFO_OPTIONS -D "CMAKE_LIBRARY_PATH:PATH=${THIRD_PARTY_INSTALL_DIR}/FreeType/lib|${CMAKE_BINARY_DIR}/ExternalProjects/OpenCV/build/OpenCV-prefix/src/OpenCV-build/3rdparty/lib")
list(APPEND PODOFO_OPTIONS -D "CMAKE_CXX_FLAGS=-isystem\\ ${CMAKE_BINARY_DIR}/ExternalProjects/OpenCV/build/OpenCV-prefix/src/OpenCV-build/3rdparty/libtiff")
list(APPEND PODOFO_OPTIONS -D LIBJPEG_LIBRARY_NAMES="jpeg,libjpeg")

build_external_project(PoDoFo "podofo-${PODOFO_VERSION}" all ${PODOFO_OPTIONS})

#------------------------------------------------------------------------------------------

# Add additional paths for 3rdparty libs (by OpenCV)

list(APPEND CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libjpeg)
list(APPEND CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/3rdparty/opencv-${OPENCV_VERSION}/3rdparty/libtiff)
list(APPEND CMAKE_LIBRARY_PATH ${THIRD_PARTY_INSTALL_DIR}/OpenCV/share/OpenCV/3rdparty/lib)
