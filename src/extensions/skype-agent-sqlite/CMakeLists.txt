cmake_minimum_required(VERSION 3.10)
project(skype_agent_sqlite LANGUAGES NONE)  # No compiled language needed

# Gather source files
set(SOURCE_FILES
    main.py
    metadata.py
    icon.png)

# Custom command to zip files into skype-agent-sqlite.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/skype-agent-sqlite.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/skype-agent-sqlite.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into skype-agent-sqlite.mobius"
)

# Custom target to trigger the zip
add_custom_target(skype_agent_sqlite ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/skype-agent-sqlite.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/skype-agent-sqlite.mobius
        DESTINATION share/mobiusft/extensions)
