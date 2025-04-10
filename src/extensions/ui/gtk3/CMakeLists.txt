# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define the project and language (C++)
project(ui_gtk3 LANGUAGES CXX)

# When every extension is build by cmake, move this variables to src/extension/CMakeLists.txt
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set project variables
set(TOP_SRC_DIR ${CMAKE_SOURCE_DIR}/../../../..)
set(TOP_BUILD_DIR ${CMAKE_BINARY_DIR}/../../../../..)

# Add include directories
include_directories(${TOP_SRC_DIR}/src)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Find dependencies
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_library(MOBIUS_LIB mobius HINTS ${TOP_BUILD_DIR}/src/mobius/.libs NO_DEFAULT_PATH)
find_library(MOBIUS_FRAMEWORK_LIB mobius_framework HINTS ${TOP_BUILD_DIR}/src/mobius/framework/.libs NO_DEFAULT_PATH)

if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_BUILD_DIR}/src/mobius/.libs")
endif()
if(NOT MOBIUS_FRAMEWORK_LIB)
    message(FATAL_ERROR "libmobius_framework not found in ${TOP_BUILD_DIR}/src/mobius/framework/.libs")
endif()

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# EXTENSION
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Define source files
set(SOURCES
    main.cpp
    box_impl.cpp
    button_impl.cpp
    icon_impl.cpp
    label_impl.cpp
    message_dialog_impl.cpp
    ui_impl.cpp
    widget_impl.cpp
    window_impl.cpp
)

add_library(ui_gtk3 MODULE ${SOURCES})
set_target_properties(ui_gtk3 PROPERTIES OUTPUT_NAME "ui-gtk3" PREFIX "")
target_include_directories(ui_gtk3 PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(ui_gtk3 PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB} ${GTK3_LIBRARIES})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Install
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS ui_gtk3 LIBRARY DESTINATION share/mobiusft/extensions)
