cmake_minimum_required(VERSION 3.0)
project(openxr-example)

if (POLICY CMP0072)
  cmake_policy (SET CMP0072 NEW)
endif(POLICY CMP0072)

IF (WIN32)
	SET(EXTRA_INCLUDES "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/GL")
ENDIF()

find_package(OpenGL REQUIRED)

IF (WIN32)
	find_package(SDL2 REQUIRED)
ELSE()
	INCLUDE(FindPkgConfig)
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

  find_package(X11 REQUIRED)
  SET(LINUX_LIBS "Xrandr ${X11_LIBRARIES}" m)
ENDIF()

add_executable(openxr-example main.c)

# First try openxr.pc from OpenXR SDK
include(FindPkgConfig)
pkg_search_module(OPENXR openxr)
if (OPENXR_FOUND)
  MESSAGE("OpenXR found with pkg-config")
  target_link_libraries(openxr-example PRIVATE ${OPENXR_LIBRARIES})

# Second, try OpenXRConfig.cmake from OpenXR SDK
else()
  MESSAGE("OpenXR not found with pkg-config, trying cmake script")

  # current issue in upstream OpenXR cmake files requires us to find Threads on our own
  find_package(Threads REQUIRED)

  find_package(OpenXR REQUIRED)
  if (NOT OpenXR_FOUND)
     MESSAGE(FATAL_ERROR "OpenXR not found!")
  endif()

  target_include_directories(openxr-example PRIVATE ${OpenXR_INCLUDE_DIR})
  target_link_libraries(openxr-example PRIVATE OpenXR::openxr_loader)
endif()

target_link_libraries(openxr-example PRIVATE ${LINUX_LIBS} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
target_include_directories(openxr-example PRIVATE ${SDL2_INCLUDE_DIR} ${EXTRA_INCLUDES})

if(MSVC)
  target_compile_options(openxr-example PRIVATE /W4 /WX)
else(MSVC)
  target_compile_options(openxr-example PRIVATE -pedantic -Wall -Wextra -Wno-unused-parameter)
endif(MSVC)


install(TARGETS openxr-example RUNTIME DESTINATION bin)
