# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.2.0 lands the next FeiQ / 飞鸽-compatible essentials (file resume, last-seen peers, history Find, window shake, screenshot, personal signature). Remaining items stay deferred.

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

## Later / out of scope for 2.2.0

1. **Custom emoji / face packs**
2. **FeiQ plugin system**
3. **File-share / public folder server**
4. **Group chat / 群 chat rooms**
5. **Voice**
6. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY` / `ANSPUBKEY`)
7. **Full skin rewrite**
   - Modern stylesheet, dark/light theme, high-DPI polish
8. **Modern packaging**
   - NSIS installer for Windows
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux

## How to try 2.2.0

1. Start QIpMsg. Last-seen peers from `~/.qipmsg/peers.json` appear grey (`Stale`) until the next `BR_ENTRY` / `ANSENTRY` refresh.
2. Preferences → Detail → File: resume interrupted transfers, auto-accept incoming files, auto-rename, default receive folder (`~/.qipmsg/recv` by default).
3. Double-click a user for the chat window. Toolbar: **Shake**, **Screenshot** (full / region; `Ctrl+Shift+S` / `Ctrl+Shift+A`), **Attach**, **Find** (`Ctrl+F`). Drag files/folders onto the chat (Windows uses the same `file://` drop path as the main send window).
4. Tray or chat **Find in history** searches all `~/.qipmsg/history/*.jsonl` rows. Per-chat scrollback is unchanged.
5. Preferences nickname box: personal signature. It is appended after `name\0group\0` on entry packets (classic IPMSG ignores the extra field). Peer signatures show in the user-list tooltip and chat caption.
6. Incoming `#@#shake#@#` / `{shake}` / `SHAKE` / `窗口抖动` shakes the chat or a visible main window. Outgoing Shake sends `#@#shake#@#`.

## Want to help?

Pick a deferred item, open an issue to discuss the design, then submit a focused PR.
