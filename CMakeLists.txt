cmake_minimum_required(VERSION 3.20)
project(frostbyte LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

option(FROSTBYTE_BUILD_DESKTOP "Build frostbyte-desktop (and frostbytecore_desktop)" ON)
option(FROSTBYTE_BUILD_SERVER  "Build frostbyte-server (and frostbytecore_headless)" ON)

if(NOT UNIX OR APPLE)
  message(WARNING "This project is only set up for Linux right now.")
endif()

add_subdirectory(frostbyte-core)

if(FROSTBYTE_BUILD_DESKTOP)
  add_subdirectory(frostbyte-desktop)
endif()
if(FROSTBYTE_BUILD_SERVER)
  add_subdirectory(frostbyte-server)
endif()
