cmake_minimum_required(VERSION 3.10)
project(iped_frontend LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png resource/*.zip)

set(SOURCE_FILES
    main.py
    metadata.py
    preferences_view.py
    processing_view.py
    report_view.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into iped-frontend.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iped-frontend.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/iped-frontend.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into iped-frontend.mobius"
)

# Custom target to trigger the zip
add_custom_target(iped_frontend ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/iped-frontend.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iped-frontend.mobius
        DESTINATION share/mobiusft/extensions)
