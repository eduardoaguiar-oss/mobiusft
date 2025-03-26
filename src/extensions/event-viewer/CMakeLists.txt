cmake_minimum_required(VERSION 3.10)
project(event_viewer LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB PYTHON_FILES *.py)
set(SOURCE_FILES ${PYTHON_FILES} icon.png)

# Custom command to zip files into event-viewer.mobius
add_custom_command(
    OUTPUT event-viewer.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "event-viewer.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into event-viewer.mobius"
)

# Custom target to trigger the zip
add_custom_target(event_viewer ALL DEPENDS event-viewer.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/event-viewer.mobius
        DESTINATION share/mobiusft/extensions)
