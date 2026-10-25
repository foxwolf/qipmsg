TEMPLATE = app

CONFIG += qt warn_on release c++17

QT += network widgets multimedia

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat
}

RESOURCES = icons.qrc

HEADERS += \
	about_dialog.h \
	autostart.h \
	avatarStore.h \
	avatarFetch.h \
	avatarWire.h \
	doodleCanvas.h \
	groupLog.h \
	fileHashIndex.h \
	shareBrowser.h \
	shareFolder.h \
	themeName.h \
	userSkin.h \
	nicBind.h \
	dataBackup.h \
	msg_server.h \
	constants.h \
	version.h \
	dir_dialog.h \
	recv_file_finish_dialog.h \
	global.h \
	groupPolicy.h \
	groupPolicyDialog.h \
	helper.h \
	ipmsg.h \
	lockfile.h \
	main_window.h \
        chat_window.h \
        contactDelegate.h \
        discussion_store.h \
        discussion_window.h \
        emoji_pack.h \
        mention_highlighter.h \
	message_store.h \
	msgCrypto.h \
        history_search_window.h \
        recentStore.h \
        resume_store.h \
        richText.h \
        screen_capture.h \
        search_user_dialog.h \
        sendThrottle.h \
        shotEditor.h \
        msg.h \
	msg_base.h \
	send_msg.h \
	recv_msg.h \
	msg_thread.h \
	msg_readed_window.h \
	msg_window.h \
	offline_queue.h \
	preferences.h \
	protocolutil.h \
	qipmsg.h \
        recv_file.h \
        recv_file_handle.h \
	recv_file_model.h \
	recv_file_map.h \
	recv_file_transfer.h \
	recv_file_thread.h \
	recvfilesortfilterproxymodel.h \
	retry_recv_file_dialog.h \
	recv_file_window.h \
	send_file_model.h \
	send_file_map.h \
	send_file.h \
	send_file_handle.h \
	send_file_thread.h \
	send_file_window.h \
	send_file_manager.h \
	serve_socket.h \
	setup_window.h \
	sizecolumndelegate.h \
	systray.h \
	file_server.h \
	transfer_codec.h \
	translator.h \
	owner.h \
	pinyin.h \
	sound_thread.h \
	stickerPicker.h \
	stickerStore.h \
	user_manager.h \
	transfer_file_window.h \
	transfer_file_model.h \
	window_manager.h \
	windowSnap.h

SOURCES += \
	about_dialog.cpp \
	autostart.cpp \
	avatarStore.cpp \
	avatarFetch.cpp \
	avatarWire.cpp \
	doodleCanvas.cpp \
	groupLog.cpp \
	fileHashIndex.cpp \
	shareBrowser.cpp \
	shareFolder.cpp \
	userSkin.cpp \
	nicBind.cpp \
	puffInflate.cpp \
	dataBackup.cpp \
	dataBackupUi.cpp \
	msg_server.cpp \
	dir_dialog.cpp \
	recv_file_finish_dialog.cpp \
	global.cpp \
	groupPolicy.cpp \
	groupPolicyDialog.cpp \
	helper.cpp \
	lockfile.cpp \
	main.cpp \
	main_window.cpp \
        chat_window.cpp \
        contactDelegate.cpp \
        discussion_store.cpp \
        discussion_window.cpp \
        emoji_pack.cpp \
        mention_highlighter.cpp \
        message_store.cpp \
        msgCrypto.cpp \
        msgCryptoSend.cpp \
        history_search_window.cpp \
        recentStore.cpp \
        resume_store.cpp \
        richText.cpp \
        screen_capture.cpp \
        search_user_dialog.cpp \
        sendThrottle.cpp \
        shotEditor.cpp \
        msg.cpp \
	msg_base.cpp \
	send_msg.cpp \
	recv_msg.cpp \
	msg_thread.cpp \
	msg_readed_window.cpp \
	msg_window.cpp \
	offline_queue.cpp \
	preferences.cpp \
	protocolutil.cpp \
	qipmsg.cpp \
        recv_file.cpp \
        recv_file_handle.cpp \
	recv_file_model.cpp \
	recv_file_map.cpp \
	recv_file_transfer.cpp \
	recv_file_thread.cpp \
	recvfilesortfilterproxymodel.cpp \
	retry_recv_file_dialog.cpp \
	recv_file_window.cpp \
	send_file_model.cpp \
	send_file_map.cpp \
	send_file.cpp \
	send_file_handle.cpp \
	send_file_thread.cpp \
	send_file_window.cpp \
	send_file_manager.cpp \
	serve_socket.cpp \
	setup_window.cpp \
	sizecolumndelegate.cpp \
	stickerPicker.cpp \
	stickerStore.cpp \
	systray.cpp \
	file_server.cpp \
	transfer_codec.cpp \
	translator.cpp \
	owner.cpp \
	pinyin.cpp \
	sound_thread.cpp \
	user_manager.cpp \
	transfer_file_window.cpp \
	transfer_file_model.cpp \
	window_manager.cpp \
	windowSnap.cpp

unix:!macx {
    LIBS += -ldl
}

macx {
    LIBS += -framework CoreGraphics
}

TRANSLATIONS = translations/qipmsg_zh_CN.ts

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj

  DEFINES += DATA_PATH=$(DATA_PATH)
  DEFINES += SOUND_PATH=$(SOUND_PATH)
  DEFINES += TRANSLATION_PATH=$(TRANSLATION_PATH)
  DEFINES += ICON_PATH=$(ICON_PATH)
  #DEFINES += NO_DEBUG_ON_CONSOLE
}

win32 {
  LIBS += -lws2_32
}
