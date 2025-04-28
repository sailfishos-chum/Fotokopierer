cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0063 NEW)

add_library(Util STATIC
    AutoZipStream.cxx
    CmdArgs.cxx
    Csv.cxx
    IndentStream.cxx
    Logger.cxx
    Pretty.cxx
    PrettyStruct.cxx
    Scan.cxx
    String.cxx
    Timer.cxx
    ToolStream.cxx
    WordWrapStream.cxx)
add_library(Fifr::Util ALIAS Util)

set_property(TARGET Util PROPERTY OUTPUT_NAME fifrutil)
target_compile_features(Util PUBLIC cxx_std_17)

# Packages

find_package(ZLIB)
if (ZLIB_FOUND)
  set_property(TARGET Util APPEND PROPERTY SOURCES GZipStream.cxx)
  set_property(SOURCE AutoZipStream.cxx APPEND PROPERTY
               COMPILE_DEFINITIONS "HAVE_GZIP=1")
  set_property(TARGET Util APPEND PROPERTY LINK_LIBRARIES ZLIB::ZLIB)
  set_property(TARGET Util APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${ZLIB_LIBRARY})
endif()

find_package(BZip2)
if (BZIP2_FOUND)
  set_property(TARGET Util APPEND PROPERTY SOURCES BZip2Stream.cxx)
  set_property(SOURCE AutoZipStream.cxx APPEND PROPERTY
               COMPILE_DEFINITIONS "HAVE_BZIP2=1")
  set_property(TARGET Util APPEND PROPERTY LINK_LIBRARIES BZip2::BZip2)
  set_property(TARGET Util APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${BZIP2_LIBRARY})
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(Util PRIVATE -W -Wall -Wconversion -Wsign-conversion -pedantic)
endif ()

# Install stuff

target_include_directories(Util INTERFACE
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)

install(
  TARGETS Util
  EXPORT FifrUtilTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  INCLUDES DESTINATION include
  )

install(
  DIRECTORY .
  DESTINATION include/fifr/util
  COMPONENT headers
  FILES_MATCHING PATTERN "*.hxx"
  )
