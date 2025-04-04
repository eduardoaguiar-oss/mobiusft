# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define the project and language (C++)
project(app_shareaza LANGUAGES CXX)

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
    CBTInfo.cpp
    CDownload.cpp
    CDownloadSource.cpp
    CLibrary.cpp
    CLibraryFile.cpp
    CLibraryFolder.cpp
    CManagedSearch.cpp
    CMatchFile.cpp
    CMatchList.cpp
    CQueryHit.cpp
    CQuerySearch.cpp
    CShareaza.cpp
    CSharedSource.cpp
    CThumbCache.cpp
    CWndSearch.cpp
    CXMLElement.cpp
    evidence_loader_impl.cpp
    file_searches_dat.cpp
)

# Build the library as a dynamically loadable module
add_library(app_shareaza MODULE ${SOURCES})
set_target_properties(app_shareaza PROPERTIES OUTPUT_NAME "app-shareaza" PREFIX "")

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
target_link_libraries(app_shareaza PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# Install to the same location as the original
install(TARGETS app_shareaza LIBRARY DESTINATION share/mobiusft/extensions)

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_library_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(SHAREAZA_LIBRARY_DAT_SOURCES
    tools/shareaza_library_dat.cpp
    CLibrary.cpp
    CLibraryFolder.cpp
    CLibraryFile.cpp
    CSharedSource.cpp
    CXMLElement.cpp
)

add_executable(shareaza_library_dat ${SHAREAZA_LIBRARY_DAT_SOURCES})
target_include_directories(shareaza_library_dat PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(shareaza_library_dat PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_sd
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(SHAREAZA_SD_SOURCES
    tools/shareaza_sd.cpp
    CBTInfo.cpp
    CDownload.cpp
    CDownloadSource.cpp
    CShareaza.cpp
    CXMLElement.cpp
)

add_executable(shareaza_sd ${SHAREAZA_SD_SOURCES})
target_include_directories(shareaza_sd PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(shareaza_sd PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: shareaza_searches_dat
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
set(SHAREAZA_SEARCHES_DAT_SOURCES
        tools/shareaza_searches_dat.cpp
        CManagedSearch.cpp
        CMatchFile.cpp
        CMatchList.cpp
        CQueryHit.cpp
        CQuerySearch.cpp
        CShareaza.cpp
        CWndSearch.cpp
        CXMLElement.cpp
)

add_executable(shareaza_searches_dat ${SHAREAZA_SEARCHES_DAT_SOURCES})
target_include_directories(shareaza_searches_dat PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(shareaza_searches_dat PRIVATE ${MOBIUS_LIB} ${MOBIUS_FRAMEWORK_LIB})

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tools installation
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
install(TARGETS
	shareaza_library_dat
	shareaza_sd
	shareaza_searches_dat
        RUNTIME DESTINATION share/mobiusft/tools
)
