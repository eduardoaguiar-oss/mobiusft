cmake_minimum_required(VERSION 3.10)
project(hive LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    filelist_view.py
    registry_treeview.py
    report_view.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into hive.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hive.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/hive.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into hive.mobius"
)

# Custom target to trigger the zip
add_custom_target(hive ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hive.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hive.mobius
        DESTINATION share/mobiusft/extensions)
