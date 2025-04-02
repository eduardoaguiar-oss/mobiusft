cmake_minimum_required(VERSION 3.10)
project(p2p_viewer LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    application_view.py
    common.py
    local_files_view.py
    remote_files_view.py
    icon.png
    ${RESOURCE_FILES}
)

# Custom command to zip files into p2p-viewer.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/p2p-viewer.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/p2p-viewer.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into p2p-viewer.mobius"
)

# Custom target to trigger the zip
add_custom_target(p2p_viewer ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/p2p-viewer.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/p2p-viewer.mobius
        DESTINATION share/mobiusft/extensions)
