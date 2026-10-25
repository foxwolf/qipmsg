cmake_minimum_required(VERSION 3.16)
project(qipmsg VERSION 2.39.11 LANGUAGES CXX)

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
    src/autostart.cpp
    src/avatarStore.cpp
    src/chat_window.cpp
    src/contactDelegate.cpp
    src/dataBackup.cpp
    src/dataBackupUi.cpp
    src/dir_dialog.cpp
    src/discussion_store.cpp
    src/discussion_window.cpp
    src/emoji_pack.cpp
    src/fileHashIndex.cpp
    src/file_server.cpp
    src/global.cpp
    src/groupPolicy.cpp
    src/groupPolicyDialog.cpp
    src/helper.cpp
    src/history_search_window.cpp
    src/lockfile.cpp
    src/main.cpp
    src/main_window.cpp
    src/mention_highlighter.cpp
    src/message_store.cpp
    src/msgCrypto.cpp
    src/msgCryptoSend.cpp
    src/nicBind.cpp
    src/msg.cpp
    src/msg_base.cpp
    src/msg_readed_window.cpp
    src/msg_server.cpp
    src/msg_thread.cpp
    src/msg_window.cpp
    src/offline_queue.cpp
    src/owner.cpp
    src/pinyin.cpp
    src/puffInflate.cpp
    src/preferences.cpp
    src/protocolutil.cpp
    src/qipmsg.cpp
    src/recentStore.cpp
    src/recv_file.cpp
    src/recv_file_finish_dialog.cpp
    src/recv_file_handle.cpp
    src/recv_file_map.cpp
    src/recv_file_model.cpp
    src/recv_file_thread.cpp
    src/recv_file_transfer.cpp
    src/recv_file_window.cpp
    src/resume_store.cpp
    src/richText.cpp
    src/recvfilesortfilterproxymodel.cpp
    src/recv_msg.cpp
    src/retry_recv_file_dialog.cpp
    src/screen_capture.cpp
    src/search_user_dialog.cpp
    src/sendThrottle.cpp
    src/shareBrowser.cpp
    src/shareFolder.cpp
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
    src/shotEditor.cpp
    src/sizecolumndelegate.cpp
    src/sound_thread.cpp
    src/stickerPicker.cpp
    src/stickerStore.cpp
    src/systray.cpp
    src/transfer_codec.cpp
    src/transfer_file_model.cpp
    src/transfer_file_window.cpp
    src/translator.cpp
    src/userSkin.cpp
    src/user_manager.cpp
    src/window_manager.cpp
    src/windowSnap.cpp
)

set(HEADERS
    src/about_dialog.h
    src/autostart.h
    src/avatarStore.h
    src/chat_window.h
    src/contactDelegate.h
    src/constants.h
    src/dataBackup.h
    src/dir_dialog.h
    src/discussion_store.h
    src/discussion_window.h
    src/emoji_pack.h
    src/fileHashIndex.h
    src/file_server.h
    src/global.h
    src/groupPolicy.h
    src/groupPolicyDialog.h
    src/helper.h
    src/history_search_window.h
    src/ipmsg.h
    src/lockfile.h
    src/main_window.h
    src/mention_highlighter.h
    src/message_store.h
    src/msgCrypto.h
    src/nicBind.h
    src/msg.h
    src/msg_base.h
    src/msg_readed_window.h
    src/msg_server.h
    src/msg_thread.h
    src/msg_window.h
    src/offline_queue.h
    src/owner.h
    src/pinyin.h
    src/preferences.h
    src/protocolutil.h
    src/qipmsg.h
    src/recentStore.h
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
    src/richText.h
    src/screen_capture.h
    src/recv_msg.h
    src/retry_recv_file_dialog.h
    src/search_user_dialog.h
    src/sendThrottle.h
    src/shareBrowser.h
    src/shareFolder.h
    src/themeName.h
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
    src/shotEditor.h
    src/sizecolumndelegate.h
    src/sound_thread.h
    src/stickerPicker.h
    src/stickerStore.h
    src/systray.h
    src/transfer_codec.h
    src/transfer_file_model.h
    src/transfer_file_window.h
    src/translator.h
    src/userSkin.h
    src/user_manager.h
    src/version.h
    src/window_manager.h
    src/windowSnap.h
)

set(RESOURCES src/icons.qrc)
set(TS_FILES src/translations/qipmsg_zh_CN.ts)

if (Qt6_FOUND)
    qt6_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(qipmsg ${SOURCES} ${HEADERS} ${RESOURCES} ${QM_FILES})

add_executable(qipmsgP1Check test/p1check/main.cpp src/protocolutil.cpp src/autostart.cpp)
target_include_directories(qipmsgP1Check PRIVATE src)

add_executable(qipmsgBackupCheck
    test/backupcheck/main.cpp
    src/dataBackup.cpp
    src/sendThrottle.cpp
)
target_include_directories(qipmsgBackupCheck PRIVATE src)

add_executable(qipmsgShareCheck
    test/sharecheck/main.cpp
    src/fileHashIndex.cpp
    src/shareFolder.cpp
)
target_include_directories(qipmsgShareCheck PRIVATE src)

add_executable(qipmsgCryptoCheck
    test/cryptocheck/main.cpp
    src/msgCrypto.cpp
)
target_include_directories(qipmsgCryptoCheck PRIVATE src)

add_executable(qipmsgQueueCheck
    test/queuecheck/main.cpp
    test/queuecheck/stubs.cpp
    src/offline_queue.cpp
)
target_include_directories(qipmsgQueueCheck PRIVATE src)

add_executable(qipmsgP2Check
    test/p2check/main.cpp
    src/richText.cpp
    src/windowSnap.cpp
    src/discussion_store.cpp
    src/userSkin.cpp
    src/nicBind.cpp
    src/puffInflate.cpp
)
target_include_directories(qipmsgP2Check PRIVATE src)

target_include_directories(qipmsg PRIVATE src)

if (Qt6_FOUND)
    target_link_libraries(qipmsg PRIVATE
        Qt6::Core
        Qt6::Core5Compat
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
    )
    target_link_libraries(qipmsgP1Check PRIVATE Qt6::Core Qt6::Network)
    target_link_libraries(qipmsgBackupCheck PRIVATE Qt6::Core)
    target_link_libraries(qipmsgShareCheck PRIVATE Qt6::Core)
    target_link_libraries(qipmsgCryptoCheck PRIVATE Qt6::Core)
    target_link_libraries(qipmsgQueueCheck PRIVATE Qt6::Core Qt6::Widgets)
    target_link_libraries(qipmsgP2Check PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
else()
    target_link_libraries(qipmsg PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        Qt5::Multimedia
    )
    target_link_libraries(qipmsgP1Check PRIVATE Qt5::Core Qt5::Network)
    target_link_libraries(qipmsgBackupCheck PRIVATE Qt5::Core)
    target_link_libraries(qipmsgShareCheck PRIVATE Qt5::Core)
    target_link_libraries(qipmsgCryptoCheck PRIVATE Qt5::Core)
    target_link_libraries(qipmsgQueueCheck PRIVATE Qt5::Core Qt5::Widgets)
    target_link_libraries(qipmsgP2Check PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
endif()

if (WIN32)
    target_link_libraries(qipmsg PRIVATE ws2_32)
endif()

if (APPLE)
    target_link_libraries(qipmsg PRIVATE "-framework CoreGraphics")
    target_link_libraries(qipmsgP2Check PRIVATE "-framework CoreGraphics")
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(qipmsg PRIVATE ${CMAKE_DL_LIBS})
    target_link_libraries(qipmsgP2Check PRIVATE ${CMAKE_DL_LIBS})
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS qipmsg RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg/translations)

add_custom_command(TARGET qipmsg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:qipmsg>/translations"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${QM_FILES}
            "$<TARGET_FILE_DIR:qipmsg>/translations"
    COMMENT "Copy zh_CN catalog next to the executable")
install(DIRECTORY src/icons   DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg)
install(DIRECTORY src/sounds  DESTINATION ${CMAKE_INSTALL_DATADIR}/qipmsg)

if (UNIX AND NOT APPLE)
    install(FILES qipmsg.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES icons/qipmsg.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
endif()
