cmake_minimum_required(VERSION 3.10)
project(vfs_item_attributes LANGUAGES NONE)  # No compiled language needed

# Gather source files
set(SOURCE_FILES
    main.py
    metadata.py
    icon.png)

# Custom command to zip files into vfs-item-attributes.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vfs-item-attributes.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/vfs-item-attributes.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into vfs-item-attributes.mobius"
)

# Custom target to trigger the zip
add_custom_target(vfs_item_attributes ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vfs-item-attributes.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vfs-item-attributes.mobius
        DESTINATION share/mobiusft/extensions)
