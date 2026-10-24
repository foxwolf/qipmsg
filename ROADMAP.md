# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.1.0 lands the Feiqiu-like essentials that stay IPMSG-compatible with FeiQ / 飞鸽 peers. Remaining items are ordered by value and risk.

## In this line of work (done)

- [x] CMake build with Qt6 preferred / Qt5 fallback
- [x] Remove ALSA, xdg-open, /etc/hostname and other Linux-only dependencies
- [x] Cross-platform sound via `QSoundEffect`
- [x] Cross-platform file/URL opening via `QDesktopServices`
- [x] Replace Qt4-only APIs (`QRegExp`, `QDirModel`, `qSort`, `toTime_t`, etc.)
- [x] Document Windows / Linux / macOS build steps in `BUILD.md`
- [x] Periodic presence refresh (`BR_ENTRY` / `BR_ABSENCE`) with configurable interval
- [x] Soft-timeout stale peers (grey, then remove after missed refreshes)
- [x] User list shows nickname, group/部门, host, IP, and status
- [x] Persistent per-peer chat window (reopen keeps scrollback)
- [x] Local message history under `~/.qipmsg/history/` (`%USERPROFILE%\.qipmsg\history\`)
- [x] GBK default transfer codec with decode fallbacks (GBK / GB2312 / UTF-8)
- [x] Tray balloon on new text + unread count in tooltip
- [x] Away / auto-reply prefs, cooldown, tray Leave menu, and `BR_ABSENCE`

## Next: still high value

1. **File & folder transfer hardening**
   - Resume interrupted transfers (`resumefile.db`-style offset store)
   - Better progress reporting for directory trees
   - Drag-and-drop already exists; verify on Windows/macOS

2. **User list persistence across restarts**
   - Restore last-seen peers (offline/grey) before the next refresh cycle

3. **Searchable history viewer**
   - Chat scrollback is in; a global log search UI is not

## Later / out of scope for 2.1.0

4. **Screenshot tool** (FeiQ screen capture)

5. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY`/`ANSPUBKEY`)

6. **Custom emoji / face packs**

7. **FeiQ plugin system**

8. **File-share / public folder server**

9. **Group chat / chat rooms**
   - Broadcast group messages via multicast

10. **Modern packaging**
    - NSIS installer for Windows
    - `.app` bundle + DMG for macOS
    - Flatpak / AppImage / DEB / RPM for Linux

11. **UI refresh**
    - Modern stylesheet, dark/light theme, high-DPI support

## How to try 2.1.0

1. Start QIpMsg. The user list refreshes on a timer (default 30s). Peers that miss several cycles turn grey (`Stale`) and are removed after twice that.
2. Double-click a user (or right-click **Open chat**) for a reusable chat window. Close and reopen it — history comes from `~/.qipmsg/history/<ip>.jsonl`.
3. Preferences → Detail/Log: AutoRefresh interval, stale miss count, tray notify, transfer codec (GBK default, UTF-8 optional).
4. Preferences or tray **Leave → Away / Auto-reply**: toggle away, set reply text and cooldown. Incoming `SENDMSG` gets one auto-reply per peer per cooldown (`IPMSG_AUTORETOPT`). Away announces `BR_ABSENCE`.
5. New text shows a tray balloon; the tooltip includes unread count. Click the tray icon to raise hidden chats.

## Want to help?

Pick an item, open an issue to discuss the design, then submit a focused PR.
