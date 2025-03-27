cmake_minimum_required(VERSION 3.10)
project(datasource_vfs LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    add_imagefile_dialog.py
    blocktable_view.py
    copy_dialog.py
    disktable_view.py
    imagefile_chooser_dialog.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into datasource-vfs.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/datasource-vfs.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/datasource-vfs.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into datasource-vfs.mobius"
)

# Custom target to trigger the zip
add_custom_target(datasource_vfs ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/datasource-vfs.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/datasource-vfs.mobius
        DESTINATION share/mobiusft/extensions)
