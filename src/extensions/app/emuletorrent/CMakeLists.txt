# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define the project and language (C++)
project(app_emuletorrent LANGUAGES CXX)

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
# EXTENSION
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Define source files
set(SOURCES
    main.cpp
    evidence_loader_impl.cpp
    file_bt_fastresume.cpp
    file_ed2k_fastresume.cpp
)

# Build the library as a dynamically loadable module
add_library(app_emuletorrent MODULE ${SOURCES})
set_target_properties(app_emuletorrent PROPERTIES OUTPUT_NAME "app-emuletorrent" PREFIX "")

# Find Autotools-built libraries in their .libs/ directories
find_library(MOBIUS_LIB mobius HINTS ${TOP_BUILD_DIR}/src/mobius/.libs NO_DEFAULT_PATH)
find_library(MOBIUS_FRAMEWORK_LIB mobius_framework HINTS ${TOP_BUILD_DIR}/src/mobius/framework/.libs NO_DEFAULT_PATH)

if(NOT MOBIUS_LIB)
    message(FATAL_ERROR "libmobius not found in ${TOP_BUILD_DIR}/src/mobius/.libs")
endif()
if(NOT MOBIUS_FRAMEWORK_LIB)
    message(FATAL_ERROR "libmobius_framework not found in ${TOP_BUILD_DIR}/src/mobius/framework/.libs")
endif()

# Link the libraries
target_link_libraries(app_emuletorrent PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# Install to the same location as the original
install(TARGETS app_emuletorrent LIBRARY DESTINATION share/mobiusft/extensions)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# TOOLS
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
