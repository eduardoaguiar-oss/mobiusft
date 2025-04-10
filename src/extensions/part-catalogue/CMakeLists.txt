cmake_minimum_required(VERSION 3.10)
project(part_catalogue LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.svg)

set(SOURCE_FILES
    main.py
    metadata.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into part-catalogue.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/part-catalogue.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/part-catalogue.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into part-catalogue.mobius"
)

# Custom target to trigger the zip
add_custom_target(part_catalogue ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/part-catalogue.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/part-catalogue.mobius
        DESTINATION share/mobiusft/extensions)
