cmake_minimum_required(VERSION 3.10)
project(gtk_ui LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    attributelist.py
    common.py
    dnd_toolitem.py
    gtk_image_buffer.py
    hex_view.py
    report_dialog.py
    tableview.py
    view_selector.py
    widetableview.py
    icon.png
    ${RESOURCE_FILES})

# Custom command to zip files into gtk-ui.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gtk-ui.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/gtk-ui.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into gtk-ui.mobius"
)

# Custom target to trigger the zip
add_custom_target(gtk_ui ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gtk-ui.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gtk-ui.mobius
        DESTINATION share/mobiusft/extensions)
