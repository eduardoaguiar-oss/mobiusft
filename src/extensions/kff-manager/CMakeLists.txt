cmake_minimum_required(VERSION 3.10)
project(kff_manager LANGUAGES NONE)  # No compiled language needed

# Gather source files
set(SOURCE_FILES
    main.py
    metadata.py
    icon.png
)

# Custom command to zip files into kff-manager.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kff-manager.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/kff-manager.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into kff-manager.mobius"
)

# Custom target to trigger the zip
add_custom_target(kff_manager ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kff-manager.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kff-manager.mobius
        DESTINATION share/mobiusft/extensions)
