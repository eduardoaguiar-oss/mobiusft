cmake_minimum_required(VERSION 3.10)
project(evidence_password_hashes LANGUAGES NONE)  # No compiled language needed

# Gather source files
file(GLOB RESOURCE_FILES resource/*.png)

set(SOURCE_FILES
    main.py
    metadata.py
    icon.png
    ${RESOURCE_FILES}
)

# Custom command to zip files into evidence-password-hashes.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/evidence-password-hashes.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/evidence-password-hashes.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into evidence-password-hashes.mobius"
)

# Custom target to trigger the zip
add_custom_target(evidence_password_hashes ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/evidence-password-hashes.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/evidence-password-hashes.mobius
        DESTINATION share/mobiusft/extensions)
