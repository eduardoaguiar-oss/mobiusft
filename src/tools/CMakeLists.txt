cmake_minimum_required(VERSION 3.10)
project(mobiusft_tools LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Installation directory for tools
set(TOOLS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/tools")

# Find pre-built libmobius and libmobius_framework from Autotools build
find_library(MOBIUS_LIB mobius HINTS "${CMAKE_SOURCE_DIR}/mobius/.libs")
find_library(MOBIUS_FW_LIB mobius_framework HINTS "${CMAKE_SOURCE_DIR}/mobius/framework/.libs")
if(NOT MOBIUS_LIB OR NOT MOBIUS_FW_LIB)
    message(FATAL_ERROR "Could not find libmobius or libmobius_framework")
endif()

# Find libtsk (SleuthKit) - mandatory dependency
find_library(LIBTSK_LDFLAGS NAMES tsk sleuthkit REQUIRED)
if(NOT LIBTSK_LDFLAGS)
    message(FATAL_ERROR "libtsk (SleuthKit) is required but not found")
endif()


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: casetree - Builds a case tree structure
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(casetree casetree.cpp)
target_include_directories(casetree PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(casetree PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(casetree PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS casetree DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirfs - Directory filesystem operations
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(dirfs dirfs.cpp)
target_include_directories(dirfs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(dirfs PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(dirfs PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS dirfs DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: dirtree - Displays directory tree
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(dirtree dirtree.cpp)
target_include_directories(dirtree PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(dirtree PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(dirtree PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS dirtree DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: filesystem_scan - Scans filesystem metadata
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(filesystem_scan filesystem_scan.cpp)
target_include_directories(filesystem_scan PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(filesystem_scan PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(filesystem_scan PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS filesystem_scan DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hashfs - Hashes each file from disk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hashfs hashfs.cpp)
target_include_directories(hashfs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hashfs PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hashfs PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS hashfs DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_info - Displays Windows Registry hive info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hive_info hive_info.cpp)
target_include_directories(hive_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hive_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hive_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS hive_info DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: hive_scan - Scans Windows Registry hives
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(hive_scan hive_scan.cpp)
target_include_directories(hive_scan PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hive_scan PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(hive_scan PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS hive_scan DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_cat - Print image file to the standard output
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_cat imagefile_cat.cpp)
target_include_directories(imagefile_cat PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_cat PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_cat PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS imagefile_cat DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_convert - Converts image file formats
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_convert imagefile_convert.cpp)
target_include_directories(imagefile_convert PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_convert PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_convert PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS imagefile_convert DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: imagefile_info - Displays image file info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(imagefile_info imagefile_info.cpp)
target_include_directories(imagefile_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(imagefile_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(imagefile_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS imagefile_info DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: list_resources - Lists resources
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(list_resources list_resources.cpp)
target_include_directories(list_resources PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(list_resources PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB})
set_target_properties(list_resources PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS list_resources DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Tool: vfs_info - Displays virtual filesystem info
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
add_executable(vfs_info vfs_info.cpp)
target_include_directories(vfs_info PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(vfs_info PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBTSK_LDFLAGS})
set_target_properties(vfs_info PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
install(TARGETS vfs_info DESTINATION ${TOOLS_INSTALL_DIR})


# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Conditional tools requiring libudev
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBUDEV libudev)
    if(LIBUDEV_FOUND)
        # --------------------------------------------------------------------
        # Tool: device_list - Lists devices using libudev
        add_executable(device_list device_list.cpp)
        target_include_directories(device_list PRIVATE ${CMAKE_SOURCE_DIR} ${LIBUDEV_INCLUDE_DIRS})
        target_compile_options(device_list PRIVATE ${LIBUDEV_CFLAGS})
        target_link_libraries(device_list PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBUDEV_LIBRARIES})
        set_target_properties(device_list PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
        install(TARGETS device_list DESTINATION ${TOOLS_INSTALL_DIR})

        # --------------------------------------------------------------------
        # Tool: disk_list - Lists disks using libudev
        add_executable(disk_list disk_list.cpp)
        target_include_directories(disk_list PRIVATE ${CMAKE_SOURCE_DIR} ${LIBUDEV_INCLUDE_DIRS})
        target_compile_options(disk_list PRIVATE ${LIBUDEV_CFLAGS})
        target_link_libraries(disk_list PRIVATE ${MOBIUS_LIB} ${MOBIUS_FW_LIB} ${LIBUDEV_LIBRARIES})
        set_target_properties(disk_list PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
        install(TARGETS disk_list DESTINATION ${TOOLS_INSTALL_DIR})
    endif()
endif()
