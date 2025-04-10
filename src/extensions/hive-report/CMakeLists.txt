cmake_minimum_required(VERSION 3.10)
project(hive_report LANGUAGES NONE)  # No compiled language needed

# Gather source files
set(SOURCE_FILES
    main.py
    metadata.py
    common.py
    report/autorun.py
    report/cached_credentials.py
    report/common.py
    report/computer_info.py
    report/devices.py
    report/email_accounts.py
    report/encrypted_volumes.py
    report/enum_devices.py
    report/gigatribe.py
    report/__init__.py
    report/installed_programs.py
    report/internet_explorer.py
    report/lsa_secrets.py
    report/metadata.py
    report/mounted_devices.py
    report/mru.py
    report/my_network_places.py
    report/my_recent_documents.py
    report/network_list.py
    report/os_folders.py
    report/os_info.py
    report/os_shared_folders.py
    report/printer_ports.py
    report/product_keys.py
    report/search_assistant.py
    report/services.py
    report/tcpip_interfaces.py
    report/user_accounts.py
    report/user_assist.py
    report/user_profiles.py
    report/word_wheel_query.py
    icon.png
)

# Custom command to zip files into hive-report.mobius
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hive-report.mobius
    COMMAND ${CMAKE_COMMAND} -E tar c "${CMAKE_CURRENT_BINARY_DIR}/hive-report.mobius" --format=zip ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping files into hive-report.mobius"
)

# Custom target to trigger the zip
add_custom_target(hive_report ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hive-report.mobius)

# Install the zipped file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hive-report.mobius
        DESTINATION share/mobiusft/extensions)
