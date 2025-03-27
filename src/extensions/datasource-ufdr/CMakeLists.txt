cmake_minimum_required(VERSION 3.10)
project(datasource_ufdr LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    caseinfo_view.py
    extractions_view.py
    fileinfo_view.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into datasource-ufdr.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into datasource-ufdr.mobius"
)

# Custom target to trigger the zip
add_custom_target(datasource_ufdr ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/datasource-ufdr.mobius
        DESTINATION share/mobiusft/extensions)
