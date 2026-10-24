# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.7.0 hardens FeiQ 2013 / IPMSG TCP file interop, 64-bit sizes/offsets, and a configurable UDP/TCP listen port + NIC. 2.4.0 UI and 2.3 protocol features stay. Remaining items stay deferred.

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
- [x] FeiQ/IPMSG TCP file command accepted as decimal or hex; packet-id dual lookup; 8s connect; cap regular-file reads
- [x] 64-bit file size/offset on the wire, resume.json strings, and progress display (GB/TB)
- [x] Configurable UDP/TCP listen port and bind address in Settings → Network (default 2425 / any IPv4, soft-fail)

## Later / out of scope for 2.7.0

1. **Custom emoji / face packs**
2. **FeiQ plugin system**
3. **File-share / public folder server**
4. **Group chat / 群 chat rooms**
5. **Voice**
6. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY` / `ANSPUBKEY`)
7. **Dark theme / extra skins**
   - 2.4.0 shipped the light messenger theme; dark mode and user skins remain later
8. **Modern packaging**
   - NSIS installer for Windows
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux

## How to try 2.7.0

1. Start QIpMsg. The main window is a light messenger list (我的好友 / department / 自己 / 黑名单) with a rounded avatar, signature field and online count. Last-seen peers from `~/.qipmsg/peers.json` appear grey until the next refresh.
2. Preferences → Detail → File: resume interrupted transfers, auto-accept incoming files, auto-rename, default receive folder (`~/.qipmsg/recv` by default). Preferences → Detail → Network: UDP/TCP listen port (default 2425) and bind NIC (empty = any IPv4). Restart after changing.
3. Double-click a user for the chat window. Toolbar: **Shake**, **Screenshot** (full / region; `Ctrl+Shift+S` / `Ctrl+Shift+A`), **Attach**, **Find** (`Ctrl+F`). Drag files/folders onto the chat (Windows uses the same `file://` drop path as the main send window).
4. Tray or chat **Find in history** searches all `~/.qipmsg/history/*.jsonl` rows. Per-chat scrollback is unchanged.
5. Preferences nickname box: personal signature. It is appended after `name\0group\0` on entry packets (classic IPMSG ignores the extra field). Peer signatures show in the user-list tooltip and chat caption.
6. Incoming `#@#shake#@#` / `{shake}` / `SHAKE` / `窗口抖动` shakes the chat or a visible main window. Outgoing Shake sends `#@#shake#@#`.

## Want to help?

Pick a deferred item, open an issue to discuss the design, then submit a focused PR.
