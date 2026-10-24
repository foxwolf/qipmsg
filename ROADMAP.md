# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.19.3 adds department-group hide, per-group message mute, and user sticker packs (PNG/GIF/WebP plus named groups) on top of 2.15.2. 2.15.2 tray/settings items, 2.11.1 protocol gaps, 2.7.0 file interop, 2.6.0 images, and 2.5.x groups / offline queue / memo stay. Light discussion groups are **not** FeiQ multicast chatroom. Per-group hide is best-effort IPMSG, not a FeiQ private extension.

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
- [x] Inline image preview in chat bubbles (send/receive/screenshot/clipboard)
- [x] Basic Unicode emoji pack in the chat toolbar (wire text; FeiQ tokens rendered locally)
- [x] User sticker folder: import PNG/GIF/WebP/JPEG under `~/.qipmsg/stickers`, send as an image file attachment
- [x] Named sticker groups in the emoji picker (`stickers.json`); built-in Unicode stays one Classic group
- [x] FeiQ/IPMSG TCP file command accepted as decimal or hex; packet-id dual lookup; 8s connect; cap regular-file reads
- [x] 64-bit file size/offset on the wire, resume.json strings, and progress display (GB/TB)
- [x] Configurable UDP/TCP listen port and bind address in Settings → Network (default 2425 / any IPv4, soft-fail)
- [x] Cross-subnet IPv4 range / CIDR bulk-add into the broadcast discovery list (unicast probe, no server, 1024-host cap)
- [x] Broadcast to all online peers with `IPMSG_BROADCASTOPT` (main window + tray)
- [x] Sealed envelope on the chat bubble (`SECRETOPT` / existing `READCHECKOPT`), opened with `READMSG`
- [x] Announce `CAPUTF8OPT` + `CLIPBOARDOPT`; UTF-8 encode/decode when the peer advertises it; GBK default for FeiQ 2013
- [x] Opt-in start with login (Windows HKCU Run, XDG autostart, macOS LaunchAgent). Default off
- [x] Do-not-disturb tray + preference: no balloon and no auto-raise; history and unread stay. Not `BR_ABSENCE`
- [x] Optional online/offline sounds (`info.wav` / `warning.wav`) with startup quiet time and per-peer / global rate limit
- [x] Outgoing bubble shows Read when `READMSG` arrives; opening a seal still sends `READMSG` as before
- [x] Hide from selected department groups (no `ANSENTRY`, unicast `BR_EXIT`; subnet broadcast pauses while any group is hidden)
- [x] Mute a department group (history and unread stay; no raise, sound, or tray balloon)

## Later / out of scope for 2.19.3

1. **FeiQ plugin system**
2. **File-share / public folder server**
3. **FeiQ multicast group / 群 chat rooms** (shared folders, announcements, full room sync). 2.5.2 only has local multi-send 讨论组.
4. **Voice**
5. **Message encryption / signatures**
   - IPMSG public key exchange (`GETPUBKEY` / `ANSPUBKEY`)
6. **Dark theme / extra skins**
   - 2.4.0 shipped the light messenger theme; dark mode and user skins remain later
7. **Modern packaging**
   - NSIS installer for Windows
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux

## How to try 2.19.3

1. Right-click a department group (or a member) and check **Hide from group**. The same flags are under Preferences → Send/Receive → **Group permissions**. They persist in `~/.qipmsg/group_policy.json` (no Qt Sql). Peers in that group are still listed locally. Their next `BR_ENTRY` gets no `ANSENTRY`, plus a unicast `BR_EXIT`. While any group stays hidden, periodic presence is unicast to other known peers and is not a subnet broadcast. Clearing the last hidden group restores the subnet broadcast on the next refresh.
2. Check **Mute messages** on another group. Incoming text is still written to history and the unread count. The chat does not raise, the notice sound does not play, and the tray balloon does not show. A tray click can still open the chat. This is independent of do-not-disturb and of hide.
3. In a chat, open **Emoji**. **Classic** inserts the built-in Unicode set. **Import** copies PNG, GIF, WebP, or JPEG into `~/.qipmsg/stickers/` (index `stickers.json`, 8 MB each, up to 400 stickers / 40 groups). Clicking a sticker sends it as a normal image attachment. Offline peers cannot take that attachment (same as other files).
4. **New group** / **Rename** / **Delete group** organize custom stickers only. Right-click a sticker to delete it or move it. Deleting the last group is refused while it still has stickers.

## How to try 2.15.2

1. Preferences → Detail: **Start with the system** is unchecked. Enable it, then the next login launches this binary (Windows current-user Run value `QIpMsg`, Linux `~/.config/autostart/qipmsg-autostart.desktop`, macOS `~/Library/LaunchAgents/org.qipmsg.login.plist`). Unchecking removes only that entry. FeiQ's startup entry is not touched. The current session does not spawn a second copy.
2. Tray **Do not disturb (don't pop message windows)** or the same checkbox under Send/Receive. New text does not balloon and does not raise a chat. The unread tooltip and history still update. Click the tray to open chats yourself. Away / auto-reply is a different menu (**Leave**).
3. Send/Receive: **Sound when peers come online or go offline** is off. Turn it on. The first 8 seconds after launch stay quiet. One peer will not beep again for 20 seconds. A burst closer than 1.5 seconds keeps a single beep. Away ↔ online does not beep; explicit exit and a later entry do (`warning.wav` / `info.wav`).
4. Check **Encap** and send. When the peer opens the seal, their `READMSG` (original packet number in the body, same as before) flips that outgoing bubble to **Read**. If that chat is not visible and **Envelope open check** is still on, the old acknowledge window also appears. Do-not-disturb suppresses that window; the bubble/history mark remains.

## How to try 2.11.1

1. Start QIpMsg. The main window is a light messenger list (我的好友 / 讨论组 / department / 自己 / 黑名单) with a rounded avatar, signature field and online count. Last-seen peers from `~/.qipmsg/peers.json` appear grey until the next refresh.
2. Right-click a peer → **Set memo**. The tree caption and chat title use the memo; 对方资料 **Nickname** stays the real IPMSG nickname. Search accepts `zs` / `zhang` for 张三.
3. Multi-select two or more contacts → right-click **创建讨论组**. Groups persist in `~/.qipmsg/discussions.json`. Double-click a discussion to send the same text to each member. Replies stay 1:1. @name highlight is local only.
4. Preferences → Send/Receive, or tray **Leave → Stealth / Hide mode**: no `ANSENTRY` to `BR_ENTRY`, no own `BR_ENTRY`/`BR_ABSENCE`; enabling stealth sends `BR_EXIT` first.
5. Send text to a grey/offline peer: it appears in the chat **Offline queue**. When that peer broadcasts `BR_ENTRY` / `ANSENTRY`, queued texts flush in order. Attachments and shake are not queued.
6. Preferences → Detail → File: resume interrupted transfers, auto-accept incoming files, auto-rename, default receive folder (`~/.qipmsg/recv` by default). Preferences → Detail → Network: UDP/TCP listen port (default 2425) and bind NIC (empty = any IPv4). Restart after changing.
7. Double-click a user for the chat window. Toolbar: **Emoji**, **Shake**, **Screenshot** (full / region; `Ctrl+Shift+S` / `Ctrl+Shift+A`), **Attach**, **Find** (`Ctrl+F`). Drag files/folders onto the chat, or paste an image with `Ctrl+V`. Image attachments show a thumbnail in the bubble; click to open full size.
8. Tray or chat **Find in history** searches all `~/.qipmsg/history/*.jsonl` rows. Per-chat scrollback is unchanged. History rows may include an `img` array of local image paths.
9. Preferences nickname box: personal signature. It is appended after `name\0group\0` on entry packets (classic IPMSG ignores the extra field). Peer signatures show in the user-list tooltip and chat caption.
10. Incoming `#@#shake#@#` / `{shake}` / `SHAKE` / `窗口抖动` shakes the chat or a visible main window. Outgoing Shake sends `#@#shake#@#`.
11. Preferences → broadcast list: add `192.168.2.1`–`192.168.2.20` or `192.168.2.0/24`. The next presence refresh unicasts `BR_ENTRY` to each address (at most 1024). A single broadcast IP such as `192.168.2.255` stays one target.
12. Type in the main window and choose **Broadcast**, or use the tray item. Each reachable peer gets one `SENDMSG` with `BROADCASTOPT`. Offline peers are skipped. Receivers show a `[Broadcast]` prefix.
13. Check **Encap** in a chat and send. The other side shows a sealed bubble and does not write the body to history until it is opened (`READMSG` + `READCHECKOPT`). Attachments still use the classic receive window.
14. Default transfer codec stays GBK. A peer whose entry sets `CAPUTF8OPT` is sent UTF-8 with `UTF8OPT` on later directed messages. Our own entry/answer sets `CAPUTF8OPT` and `CLIPBOARDOPT` without switching the presence body off GBK.

## Want to help?

Pick a deferred item, open an issue to discuss the design, then submit a focused PR.
