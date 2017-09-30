project(screensaver.pyro)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
if(NOT WIN32)
  find_package(OpenGL REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
endif()

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways (becomes done in future)
                    )

set(PYRO_SOURCES src/Pyro.cpp)

set(PYRO_HEADERS src/Pyro.h)


build_addon(screensaver.pyro PYRO DEPLIBS)

include(CPack)
