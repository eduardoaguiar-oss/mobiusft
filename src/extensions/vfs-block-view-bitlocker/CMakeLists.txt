cmake_minimum_required(VERSION 3.10)
project(vfs_block_view_bitlocker LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    fvek_view.py
    protectors_view.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into vfs-block-view-bitlocker.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-bitlocker.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-bitlocker.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into vfs-block-view-bitlocker.mobius"
)

# Custom target to trigger the zip
add_custom_target(vfs_block_view_bitlocker ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-bitlocker.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-bitlocker.mobius
        DESTINATION share/mobiusft/extensions)
