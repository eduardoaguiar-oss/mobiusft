cmake_minimum_required(VERSION 3.10)
project(vfs_block_view_partition_system LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into vfs-block-view-partition-system.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-partition-system.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-partition-system.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into vfs-block-view-partition-system.mobius"
)

# Custom target to trigger the zip
add_custom_target(vfs_block_view_partition_system ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-partition-system.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vfs-block-view-partition-system.mobius
        DESTINATION share/mobiusft/extensions)
