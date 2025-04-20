cmake_minimum_required(VERSION 3.10)
project(mobius_python)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(crypt)
add_subdirectory(database)
add_subdirectory(datasource)
add_subdirectory(datetime)
add_subdirectory(decoder)
add_subdirectory(encoder)
add_subdirectory(framework)
add_subdirectory(kff)
add_subdirectory(io)
add_subdirectory(model)
add_subdirectory(os)
add_subdirectory(pod)
add_subdirectory(system)
add_subdirectory(turing)
add_subdirectory(ui)
add_subdirectory(vfs)

# Create the mobius Python module
add_library(mobius MODULE
    module.cc
    pycallback.cc
    pyfunction.cc
    pymodule.cc
    pyobject.cc
    pymobius.cc
    pytypeobject.cc
    api_dataholder.cc
    api_metadata.cc
)

# Add header files
file(GLOB HEADER_FILES "*.h")
target_sources(mobius PRIVATE ${HEADER_FILES})

# Set include directories
target_include_directories(mobius PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compiler flags
target_compile_options(mobius PRIVATE -fno-strict-aliasing)

# Link against Python libraries
target_link_libraries(mobius PRIVATE
    ${Python_LIBRARIES}
    mobius
    mobius_framework
)

# Set output properties
set_target_properties(mobius PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Link against subdirectory libraries
target_link_libraries(mobius PRIVATE
    pymobius-core
    pymobius-crypt
    pymobius-database
    pymobius-datasource
    pymobius-datetime
    pymobius-decoder
    pymobius-encoder
    pymobius-framework
    pymobius-kff
    pymobius-io
    pymobius-model
    pymobius-os
    pymobius-pod
    pymobius-system
    pymobius-turing
    pymobius-ui
    pymobius-vfs
) 