cmake_minimum_required(VERSION 3.10)
project(ice LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png resource/*.txt resource/*.svg)

set(SOURCE_FILES
    main.py
    metadata.py
    about_dialog.py
    add_item_dialog.py
    case_treeview.py
    case_view.py
    new_case_dialog.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into ice.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ice.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/ice.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into ice.mobius"
)

# Custom target to trigger the zip
add_custom_target(ice ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ice.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ice.mobius
        DESTINATION share/mobiusft/extensions)
