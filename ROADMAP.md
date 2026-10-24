# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

This PR lands the build/cross-platform foundation. Remaining features are ordered by value and implementation risk.

## In this PR (done)

- [x] CMake build with Qt6 preferred / Qt5 fallback
- [x] Remove ALSA, xdg-open, /etc/hostname and other Linux-only dependencies
- [x] Cross-platform sound via `QSoundEffect`
- [x] Cross-platform file/URL opening via `QDesktopServices`
- [x] Replace Qt4-only APIs (`QRegExp`, `QDirModel`, `qSort`, `toTime_t`, etc.)
- [x] Document Windows / Linux / macOS build steps in `BUILD.md`

## Next: Feiqiu-like essentials (high value, low/medium risk)

1. **Stable user list / presence**
   - Heartbeat/refresh to keep peer list alive across subnets
   - Preserve user list across restarts

2. **Improved chat UX**
   - Persistent chat window per peer (currently opens a new send window each time)
   - Multi-recipient send hardening (already supported, test with real peers)
   - Basic message history view / searchable log viewer

3. **File & folder transfer hardening**
   - Resume interrupted transfers
   - Better progress reporting for directory trees
   - Drag-and-drop already exists; verify on Windows/macOS

4. **Cross-platform tray + notifications**
   - Native OS notifications where available
   - Unread message badge / tray tooltip

5. **Encoding robustness**
   - Auto-detect GBK vs UTF-8 from peers
   - Default to GBK/GB2312 for Feiqiu compatibility

## Later: nice-to-have features

6. **Auto-reply / away status**
   - IPMSG `BR_ABSENCE` / `GETABSENCEINFO` support

7. **Group chat / chat rooms**
   - Broadcast group messages via multicast

8. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY`/`ANSPUBKEY`)

9. **Modern packaging**
   - NSIS installer for Windows
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux

10. **UI refresh**
    - Modern stylesheet, dark/light theme, high-DPI support

## Want to help?

Pick an item, open an issue to discuss the design, then submit a focused PR.
