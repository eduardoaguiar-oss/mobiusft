cmake_minimum_required(VERSION 3.10)
project(file_explorer LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png resource/*.svg)

set(SOURCE_FILES
    main.py
    metadata.py
    compact_view.py
    file_finder_view.py
    file_view.py
    icon_view.py
    navigation_bar.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into file-explorer.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/file-explorer.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/file-explorer.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into file-explorer.mobius"
)

# Custom target to trigger the zip
add_custom_target(file_explorer ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/file-explorer.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/file-explorer.mobius
        DESTINATION share/mobiusft/extensions)
