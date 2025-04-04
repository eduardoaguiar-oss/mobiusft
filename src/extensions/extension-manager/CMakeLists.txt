cmake_minimum_required(VERSION 3.10)
project(extension_manager LANGUAGES NONE)  # No compiled language needed

# Gather source files
set(SOURCE_FILES main.py metadata.py icon.png)

# Custom command to zip files into extension-manager.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/extension-manager.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/extension-manager.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into extension-manager.mobius"
)

# Custom target to trigger the zip
add_custom_target(extension_manager ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/extension-manager.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/extension-manager.mobius
        DESTINATION share/mobiusft/extensions)
