# QIpMsg 变更日志

## 2.0.3 — 2026-10-24

### xx
- 迁移构建系统到 CMake，优先 Qt6、兼容 Qt5
- 移除 ALSA 与 Linux-only 的 xdg-open、/etc/hostname 依赖，抽象为跨平台 Qt API

### yy
- 使用 Qt Multimedia / QSoundEffect 替代 ALSA 播放提示音
- 使用 QDesktopServices 打开日志、接收文件/目录，支持 Windows / macOS / Linux
- 使用 QSysInfo::machineHostName() 获取主机名
- 文件传输时间戳统一使用跨平台的 QDateTime 转换
- 添加 CMake 构建与多平台编译文档

### zz
- 修复 Qt 5/6 下 QMessageBox、QHeaderView、QSortFilterProxyModel 等弃用/移除接口
- 替换 QRegExp、QDirModel、qSort、QString::SkipEmptyParts 等 Qt4 专有 API
- 修复登录名环境变量匹配失败，并补充 Windows 的 USERNAME
- 修复 Windows 下 UDP 绑定到 IPv6 Any 导致广播失败并弹出阻塞错误框的问题

#### 编译
- 使用 QLockFile 替代 POSIX flock/fcntl，修复 Windows MinGW 下 S_IRUSR、struct flock、F_SETLK 无法编译
- 使用 QThread::msleep 替代 usleep，去掉对 unistd.h 的依赖
- 文件发送套接字改为 Winsock/POSIX 条件编译，避免 sys/socket.h 在 Windows 下失败
- incomingConnection 与发送线程套接字描述符改为 qintptr，避免 Win64 句柄截断
- 接收文件时间戳在 Windows 下改用 sys/utime.h
- helper.cpp 的 endl 改为 Qt::endl

---

## 1.0.1
- License unify.

## 1.0.0
- Message default not sealed.
- Fix send file bug.

## 0.9.9
- Ability to add specified broadcast address.

## 0.9.7
- Bug fix with Qt 4.6.
