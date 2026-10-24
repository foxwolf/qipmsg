cmake_minimum_required(VERSION 3.16)
project(qipmsg VERSION 2.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Prefer Qt 6, fall back to Qt 5.
find_package(Qt6 QUIET COMPONENTS Core Core5Compat Widgets Network Multimedia LinguistTools)
if (Qt6_FOUND)
    message(STATUS "Using Qt 6")
else()
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets Network Multimedia LinguistTools)
    message(STATUS "Using Qt 5")
endif()

set(SOURCES
    src/about_dialog.cpp
    src/chat_window.cpp
    src/dir_dialog.cpp
    src/file_server.cpp
    src/global.cpp
    src/helper.cpp
    src/history_search_window.cpp
    src/lockfile.cpp
    src/main.cpp
    src/main_window.cpp
    src/message_store.cpp
    src/msg.cpp
    src/msg_base.cpp
    src/msg_readed_window.cpp
    src/msg_server.cpp
    src/msg_thread.cpp
    src/msg_window.cpp
    src/owner.cpp
    src/preferences.cpp
    src/qipmsg.cpp
    src/recv_file.cpp
    src/recv_file_finish_dialog.cpp
    src/recv_file_handle.cpp
    src/recv_file_map.cpp
    src/recv_file_model.cpp
    src/recv_file_thread.cpp
    src/recv_file_transfer.cpp
    src/recv_file_window.cpp
    src/resume_store.cpp
    src/recvfilesortfilterproxymodel.cpp
    src/recv_msg.cpp
    src/retry_recv_file_dialog.cpp
    src/screen_capture.cpp
    src/search_user_dialog.cpp
    src/send_file.cpp
    src/send_file_handle.cpp
    src/send_file_manager.cpp
    src/send_file_map.cpp
    src/send_file_model.cpp
    src/send_file_thread.cpp
    src/send_file_window.cpp
    src/send_msg.cpp
    src/serve_socket.cpp
    src/setup_window.cpp
    src/sizecolumndelegate.cpp
    src/sound_thread.cpp
    src/systray.cpp
    src/transfer_codec.cpp
    src/transfer_file_model.cpp
    src/transfer_file_window.cpp
    src/translator.cpp
    src/user_manager.cpp
    src/window_manager.cpp
)

set(HEADERS
    src/about_dialog.h
    src/chat_window.h
    src/constants.h
    src/dir_dialog.h
    src/file_server.h
    src/global.h
    src/helper.h
    src/history_search_window.h
    src/ipmsg.h
    src/lockfile.h
    src/main_window.h
    src/message_store.h
    src/msg.h
    src/msg_base.h
    src/msg_readed_window.h
    src/msg_server.h
    src/msg_thread.h
    src/msg_window.h
    src/owner.h
    src/preferences.h
    src/qipmsg.h
    src/recv_file.h
    src/recv_file_finish_dialog.h
    src/recv_file_handle.h
    src/recv_file_map.h
    src/recv_file_model.h
    src/recv_file_thread.h
    src/recv_file_transfer.h
    src/recv_file_window.h
    src/recvfilesortfilterproxymodel.h
    src/resume_store.h
    src/screen_capture.h
    src/recv_msg.h
    src/retry_recv_file_dialog.h
    src/search_user_dialog.h
    src/send_file.h
    src/send_file_handle.h
    src/send_file_manager.h
    src/send_file_map.h
    src/send_file_model.h
    src/send_file_thread.h
    src/send_file_window.h
    src/send_msg.h
    src/serve_socket.h
    src/setup_window.h
    src/sizecolumndelegate.h
    src/sound_thread.h
    src/systray.h
    src/transfer_codec.h
    src/transfer_file_model.h
    src/transfer_file_window.h
    src/translator.h
    src/user_manager.h
    src/version.h
    src/window_manager.h
)

set(RESOURCES src/icons.qrc)
set(TS_FILES src/translations/qipmsg_zh_CN.ts)

if (Qt6_FOUND)
    qt6_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(qipmsg ${SOURCES} ${HEADERS} ${RESOURCES} ${QM_FILES})

target_include_directories(qipmsg PRIVATE src)

if (Qt6_FOUND)
    target_link_libraries(qipmsg PRIVATE
        Qt6::Core
        Qt6::Core5Compat
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
    )
else()
    target_link_libraries(qipmsg PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        Qt5::Multimedia
    )
endif()

if (WIN32)
    target_link_libraries(qipmsg PRIVATE ws2_32)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS qipmsg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg/translations)
install(DIRECTORY src/icons   DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg)
install(DIRECTORY src/sounds  DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg)

if (UNIX AND NOT APPLE)
    install(FILES qipmsg.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES icons/qipmsg.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
endif()
