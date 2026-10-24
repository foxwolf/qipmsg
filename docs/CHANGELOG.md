# QIpMsg 变更日志

## 2.6.0 — 2026-10-24

在已合并的 2.5.2（#31 轻量讨论组）、2.5.1（#30 离线队列）与 2.5.0（#29 备注 / 拼音 / 隐身）之上加入会话图片预览与基础表情。版本保持 **2.6.0**。

### yy
- 会话气泡内联图片预览：发送/接收/截图/剪贴板粘贴的常见图片显示缩略图，点击用系统程序打开原图
- 聊天工具栏基础 Unicode 表情包，写入普通 IPMSG 文本；本端将飞秋 `/微笑` 等口令渲染为表情
- 历史 JSONL 增加可选 `img` 字段（本地图片路径数组），与现有 `t`/`d`/`n`/`ip`/`x` 并存

### zz
- 更新 README、ROADMAP、BUILD 与 zh_CN，说明图片预览与表情互通假设
- 版本取 2.6.0，避免与并行 PR 碰撞（#29=2.5.0，#30=2.5.1，#31=2.5.2，#27=2.7.0）

---

## 2.5.2 — 2026-10-24

在已合并的 2.5.0（#29）与 2.5.1（#30）之上加入轻量讨论组。版本保持 **2.5.2**。

### yy
- 轻量讨论组：多选联系人创建命名成员集合，本地 JSON 持久化到 `~/.qipmsg/discussions.json`（不用 Qt Sql）
- 主窗口「讨论组」独立于飞秋部门分组；双击打开多对端会话，同一文本走现有群发路径（`IPMSG_MULTICASTOPT` + 按 IP 发送）
- 讨论组窗口本地记录 `history/disc-<id>.jsonl`；可选 @成员显示名高亮（仅界面，线路仍为纯文本）

### zz
- 明确本能力是本地多发 UX，不是飞秋组播群 / 聊天室协议；群共享、群公告、完整聊天室同步仍不在范围
- 版本取 2.5.2，避免与并行 PR 碰撞（#29=2.5.0，#30=2.5.1，#28=2.6.0，#27=2.7.0）
- 更新 README、ROADMAP、BUILD 与 zh_CN 翻译

---

## 2.5.1 — 2026-10-24

在已合并的 2.5.0（#29 备注 / 拼音 / 隐身）之上加入离线队列。版本保持 **2.5.1**。

### yy
- 飞秋式离线队列：对离线/过期对端发文本时入队（默认自动，可改为询问），会话内显示待发送列表并可取消
- 队列写入 `~/.qipmsg/offline_queue.json`（无 Qt Sql）；对端 `ANSENTRY`/刷新变为在线或离开后按序发出并写入聊天历史
- 附件不入队（无中转服务器）；窗口抖动不入队

### zz
- 更新 README、ROADMAP、BUILD 与 zh_CN，说明与飞秋「在线自动发送」的差异

---

## 2.5.0 — 2026-10-24

本版从 master（含 #25，约 2574035）分出，**不**叠在 #27 / #28 上。#28 在独立分支上声明 2.6.0，#27 声明 2.7.0；2.5.0 留给本条 P1（备注 + 拼音 + 隐身）。

### yy
- 本地备注昵称：联系人树与会话标题显示备注，对方资料保留真实昵称；写入 `~/.qipmsg/peers.json` 的 `memo` 字段（无 Qt Sql）
- 联系人搜索支持中文拼音首字母与常见字全拼，同时匹配昵称 / 主机 / 登录名 / 备注 / IP
- 隐身模式：偏好设置与托盘「离开」菜单可切换；开启后不回复 `BR_ENTRY` 的 `ANSENTRY`，也不再广播入网

### zz
- 更新 README、ROADMAP、BUILD 与 zh_CN，说明备注、拼音表许可与飞秋隐身互通假设

---

## 2.4.0 — 2026-10-24

### yy
- 用现代浅色会话界面替换飞秋绿顶栏：共享 QSS、主窗口头像/搜索/圆角联系人、聊天气泡与对方资料卡片

### zz
- 更新 README、ROADMAP 与 zh_CN 翻译，说明 2.4.0 浅色界面

---

## 2.3.0 — 2026-10-24

### xx
- 迁移构建系统到 CMake，优先 Qt6、兼容 Qt5
- 移除 ALSA 与 Linux-only 的 xdg-open、/etc/hostname 依赖，抽象为跨平台 Qt API

### yy
- 使用 Qt Multimedia / QSoundEffect 替代 ALSA 播放提示音
- 使用 QDesktopServices 打开日志、接收文件/目录，支持 Windows / macOS / Linux
- 使用 QSysInfo::machineHostName() 获取主机名
- 文件传输时间戳统一使用跨平台的 QDateTime 转换
- 添加 CMake 构建与多平台编译文档
- 周期性广播 BR_ENTRY / BR_ABSENCE，按间隔刷新在线列表
- 未应答用户先标为过期（灰色），再按错过次数移除
- 按对端复用聊天窗口，关闭后再开会载入本地历史
- 消息历史写入 `~/.qipmsg/history/`（Windows 为 `%USERPROFILE%\.qipmsg\history\`）
- 传输编码默认 GBK，解码依次尝试已配置编码 / GBK / GB2312 / UTF-8
- 新文本消息弹出托盘提示，工具提示显示未读数
- 离开/自动回复：偏好设置与托盘菜单，按对端冷却回复，并广播 BR_ABSENCE
- 普通文件 TCP 断点续传，偏移写入 `~/.qipmsg/resume.json`（不用 Qt Sql）
- 目录接收进度显示当前子文件与文件夹/文件计数
- 文件偏好：自动接收、默认保存目录、重名自动改名；聊天窗口支持拖放附件
- 启动时从 `~/.qipmsg/peers.json` 恢复上次用户并标为离线灰色
- 全局查找本地 `history/*.jsonl` 聊天记录
- 窗口抖动：发送 `#@#shake#@#`，并接受飞秋常见抖动标记
- 聊天工具栏截图（全屏/框选），以 PNG 走现有文件传输
- 个人签名写入入网附加字段第三段 NUL，列表提示与会话标题可显示对端签名

### zz
- 修复 Qt 5/6 下 QMessageBox、QHeaderView、QSortFilterProxyModel 等弃用/移除接口
- 替换 QRegExp、QDirModel、qSort、QString::SkipEmptyParts 等 Qt4 专有 API
- 修复登录名环境变量匹配失败，并补充 Windows 的 USERNAME
- 修复 Windows 下 UDP 绑定到 IPv6 Any 导致广播失败并弹出阻塞错误框的问题
- 更新 ROADMAP、README，说明如何试用在线刷新、持久会话、编码与自动回复
- 更新 ROADMAP、README、BUILD，说明续传、历史查找、抖动、截图与签名
- 补全简体中文界面并默认加载 zh_CN 翻译目录
- 主窗口改为飞秋式分组树：个性签名、在线数、搜索好友、我的好友/部门/自己/黑名单
- 会话窗口增加飞秋式标题栏、中文工具栏、对方资料与未实现功能占位

#### 编译
- 使用 QLockFile 替代 POSIX flock/fcntl，修复 Windows MinGW 下 S_IRUSR、struct flock、F_SETLK 无法编译
- 使用 QThread::msleep 替代 usleep，去掉对 unistd.h 的依赖
- 文件发送套接字改为 Winsock/POSIX 条件编译，避免 sys/socket.h 在 Windows 下失败
- incomingConnection 与发送线程套接字描述符改为 qintptr，避免 Win64 句柄截断
- 接收文件时间戳在 Windows 下改用 sys/utime.h
- helper.cpp 的 endl 改为 Qt::endl
- 修复 Qt6 下 `class QStringList` 前向声明与 `QByteArray::append(QString)` 无法编译
- 文件创建时间改为 `QFileInfo::birthTime()`，避免 Qt6 移除 `created()`

#### 注释
- 补充飞秋向能力的 ROADMAP 完成项与后续范围说明
- 将 2.2.0 本轮能力标为完成，表情/插件/文件共享/群聊/语音/加密/换肤/NSIS 仍延后
- 说明界面默认简体中文，zh_CN 目录随程序资源加载
- 说明飞秋式主窗口分组树与会话对方资料

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
