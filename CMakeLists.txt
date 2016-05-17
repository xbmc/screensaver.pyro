project(screensaver.pyro)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)

include_directories(${KODI_INCLUDE_DIR})

set(PYRO_SOURCES src/Pyro.cpp)

set(DEPLIBS)

build_addon(screensaver.pyro PYRO DEPLIBS)

include(CPack)
