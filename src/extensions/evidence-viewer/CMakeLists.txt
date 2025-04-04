cmake_minimum_required(VERSION 3.10)
project(evidence_viewer LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    common.py
    generic_view.py
    hashes_view.py
    icon_view.py
    list_view.py
    metadata_view.py
    model.py
    navigation_view.py
    processing_view.py
    icon.png
    ${RESOURCE_FILES}
)

# Custom command to zip files into evidence-viewer.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/evidence-viewer.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/evidence-viewer.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into evidence-viewer.mobius"
)

# Custom target to trigger the zip
add_custom_target(evidence_viewer ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/evidence-viewer.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/evidence-viewer.mobius
        DESTINATION share/mobiusft/extensions)
