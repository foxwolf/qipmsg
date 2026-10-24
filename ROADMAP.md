# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.5.2 adds light discussion groups on top of 2.5.1 (offline queue) and 2.5.0 (memo, pinyin, stealth). This is **not** FeiQ multicast chatroom. Full group rooms stay deferred.

## In this line of work (done)

- [x] CMake build with Qt6 preferred / Qt5 fallback
- [x] Remove ALSA, xdg-open, /etc/hostname and other Linux-only dependencies
- [x] Cross-platform sound via `QSoundEffect`
- [x] Cross-platform file/URL opening via `QDesktopServices`
- [x] Replace Qt4-only APIs (`QRegExp`, `QDirModel`, `qSort`, `toTime_t`, etc.)
- [x] Document Windows / Linux / macOS build steps in `BUILD.md`
- [x] Periodic presence refresh (`BR_ENTRY` / `BR_ABSENCE`) with configurable interval
- [x] Soft-timeout stale peers (grey; last-seen rows stay after restart)
- [x] User list shows nickname, group/部门, host, IP, status, and signature tooltip
- [x] Persistent per-peer chat window (reopen keeps scrollback)
- [x] Local message history under `~/.qipmsg/history/` (`%USERPROFILE%\.qipmsg\history\`)
- [x] Global Find across all history JSONL files
- [x] GBK default transfer codec with decode fallbacks (GBK / GB2312 / UTF-8)
- [x] Tray balloon on new text + unread tooltip
- [x] Away / auto-reply prefs, cooldown, tray Leave menu, and `BR_ABSENCE`
- [x] TCP file resume via `~/.qipmsg/resume.json` (plain JSON, no Qt Sql)
- [x] Directory-tree receive progress (current child + folder/file counts)
- [x] Auto-accept / default recv path / auto-rename preferences
- [x] Chat drag-and-drop attach (same `toLocalFile()` path as the send window)
- [x] Screen shake (`#@#shake#@#` and common FeiQ-tolerated tokens)
- [x] Qt screenshot (full screen / region) sent as a PNG file attachment
- [x] Personal signature announced as a third NUL field after nickname/group
- [x] Full Simplified Chinese UI (`qipmsg_zh_CN`), loaded by default
- [x] FeiQ-like main contact tree (signature, online count, search, groups, blacklist)
- [x] FeiQ-like chat header, Chinese toolbar, 对方资料, disabled stubs
- [x] Modern light stylesheet (WeChat / Slack-lite / Fluent-ish), rounded avatars, chat bubbles
- [x] Local memo / remark nicknames persisted in `peers.json` (`memo`; no Qt Sql)
- [x] Contact search: substring + Chinese pinyin initials (GB2312 ranges) and common full pinyin
- [x] Stealth / HideMode: skip `ANSENTRY` and presence broadcasts; tray + settings toggle
- [x] Offline text queue (`offline_queue.json`; flush on ANSENTRY / online refresh)
- [x] Light discussion groups (local JSON member set + multi-send UX; not FeiQ multicast)

## Later / out of scope for 2.5.2

1. **Custom emoji / face packs**
2. **FeiQ plugin system**
3. **File-share / public folder server**
4. **FeiQ multicast group / 群 chat rooms** (shared folders, announcements, full room sync). 2.5.2 only has local multi-send 讨论组.
5. **Voice**
6. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY` / `ANSPUBKEY`)
7. **Dark theme / extra skins**
   - 2.4.0 shipped the light messenger theme; dark mode and user skins remain later
8. **Modern packaging**
   - NSIS installer for Windows
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux
9. **Hide from specific groups**
   - Per-group stealth is out of scope for 2.5.0 (LAN discovery has no server to enforce it)

## How to try 2.5.2

1. Start QIpMsg. The main window is a light messenger list (我的好友 / 讨论组 / department / 自己 / 黑名单) with a rounded avatar, signature field and online count. Last-seen peers from `~/.qipmsg/peers.json` appear grey until the next refresh.
2. Right-click a peer → **Set memo**. The tree caption and chat title use the memo; 对方资料 **Nickname** stays the real IPMSG nickname. Search accepts `zs` / `zhang` for 张三.
3. Multi-select two or more contacts → right-click **创建讨论组**. Groups persist in `~/.qipmsg/discussions.json`. Double-click a discussion to send the same text to each member. Replies stay 1:1. @name highlight is local only.
4. Preferences → Send/Receive, or tray **Leave → Stealth / Hide mode**: no `ANSENTRY` to `BR_ENTRY`, no own `BR_ENTRY`/`BR_ABSENCE`; enabling stealth sends `BR_EXIT` first.
5. Send text to a grey/offline peer: it appears in the chat **Offline queue**. When that peer broadcasts `BR_ENTRY` / `ANSENTRY`, queued texts flush in order. Attachments and shake are not queued.
6. Preferences → Detail → File: resume interrupted transfers, auto-accept incoming files, auto-rename, default receive folder (`~/.qipmsg/recv` by default).
7. Double-click a user for the chat window. Toolbar: **Shake**, **Screenshot** (full / region; `Ctrl+Shift+S` / `Ctrl+Shift+A`), **Attach**, **Find** (`Ctrl+F`).
8. Tray or chat **Find in history** searches all `~/.qipmsg/history/*.jsonl` rows.
9. Incoming `#@#shake#@#` / `{shake}` / `SHAKE` / `窗口抖动` shakes the chat or a visible main window. Outgoing Shake sends `#@#shake#@#`.

## Want to help?

Pick a deferred item, open an issue to discuss the design, then submit a focused PR.
