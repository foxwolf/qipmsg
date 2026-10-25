# QIpMsg Roadmap — toward a Feiqiu-like LAN messenger

2.52.16 groups the chat toolbar into Transfer, Chat, and More so the session window no longer stretches to fit a single row of text buttons. Send file, send folder, screenshot, emoji, shake, and find stay one click. Share browse, group folder, doodle, favorites, zone, and log sit under More. Remote assist, remote control, and voice stay disabled there. The header Profile button hides the peer card. Width, height, maximized state, and that switch are stored in `qipmsg.ini` under `ChatWindow`. 2.50.15 opens the contact window at launch unless start-in-tray is on and the tray icon is actually visible, and it explains a listen-port conflict (in use or access denied, including a Windows excluded range when `netsh` can be read) before saving a free `udpPort`. Port 2425 stays preferred. 2.48.14 opens `\\<ipv4>\` on Windows from the contact menu or the chat profile, accepts `qipmsg --msg <ipv4-or-login@host> --text <message>` on a per-user local socket while the app is already running, and stores markdown notes under `~/.qipmsg/zone/`. The share action does not collect SMB passwords. The command is not a plugin API and does not listen on TCP. The notes are not FeiQ's `zone/*.db` blog. Remote assist, remote control, and voice stay disabled. 2.45.13 stores local reminders in `schedule.json` and favorites in `favorites.json` (no Qt Sql, no cloud, no daemon after quit). A due reminder shows a tray balloon while the app is running and can send ordinary `SENDMSG` text when the user asks to notify a peer. There is no FeiQ clock packet. Peers that set `IPMSG_DIALUPOPT` are labeled in the contact tree; presence, absence, and exit are also unicast to them. The preference "I am a dial-up host" is off by default. Ordinary LAN peers are unchanged. FeiQ's own dial-up icon was not checked packet by packet. 2.42.12 exchanges a small photo between qipmsg peers (presence tag `,av:<12 hex>` and TCP command 126 verb `AVATAR`), keeps an append-only discussion log in `grouplog/<id>.jsonl`, and sends a doodle PNG through the existing image attachment. FeiQ still has no verified photo opcode: the version string stays `1`, and the photo is not a file attachment. The log note is plain text, not a FeiQ room. 2.39.11 adds a user skin pack (folder or zip under `~/.qipmsg/skins/`), a NIC choice stored by MAC when the adapter has one, and a per-discussion local folder that online qipmsg members can list. Light / dark / system stay. A bad QSS falls back to the built-in sheet. Binding is still an IPv4 listen; Linux may also set `SO_BINDTODEVICE`, macOS `IP_BOUND_IF`, and Windows `IP_UNICAST_IF`. No stack here binds a raw MAC. Any-IPv4 discovery still walks every NIC. The discussion folder is `sharePath` in `discussions.json` plus the existing file-port command 126 (`GLIST` / `GGET`), not a FeiQ multicast room. 2.36.10 adds a practical chat rich-text subset, window snap while selecting a screenshot region, and a local discussion-group notice. The SENDMSG body stays plain UTF-8/GBK text so FeiQ stays readable. A qipmsg peer may also get private command 126 `RICH:<packet>:<base64 HTML>` when the message is neither encrypted nor sealed. Snap uses top-level window bounds where the platform allows it; hold Alt (Option on macOS) to disable. The notice is a string in `discussions.json` plus the existing multi-send, not a FeiQ multicast room. 2.30.6 adds a local file-hash index with qipmsg-only instant skip, a qipmsg shared folder (browse and download on the existing file port), and light / dark / system appearance. FeiQ still receives a full file transfer and cannot browse this share: neither hash handshake nor FeiQ share packets were verified. 2.27.5 outbound TCP send cap, zip backup (no Qt Sql), screenshot annotation, and local-only avatars stay. 2.23.4 department-group multicast, recent contacts, FeiQ typing commands 121/122 (version stays `1`), and transfer cancel plus ETA stay. Light discussion groups are **not** FeiQ multicast chatroom. Per-group hide is best-effort IPMSG, not a FeiQ private extension. Typing is the captured FeiQ 121/122 mode, not a new private opcode. Avatars are not a FeiQ photo protocol.

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
- [x] Chat toolbar grouped into Transfer / Chat / More (icon + text). Secondary actions and disabled remote/voice items are under More. Peer profile collapses. Last size is remembered in `qipmsg.ini` `[ChatWindow]`
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
- [x] Startup shows the contact window unless "Start hidden in the notification area" is on. The tray menu opens that window. No tray, or an icon that did not become visible, still shows the window
- [x] If UDP or TCP cannot bind, explain in-use vs access denied (Windows excluded ranges via netsh when readable) and offer a free port saved to `udpPort`. 2425 stays preferred. Closing the dialog keeps the contact window open
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
- [x] Send to a department group: online and away only, existing `IPMSG_MULTICASTOPT` path, offline not queued
- [x] Recent contacts panel from history timestamps (`recent.json`, no Qt Sql); click opens chat
- [x] Typing status via FeiQ commands 121 / 122 (no option bits, version stays `1`); preference defaults on
- [x] Cancel an in-progress TCP file send; receiver cancel also sends `IPMSG_RELEASEFILES`
- [x] Transfer window ETA from observed throughput (folders: speed only)
- [x] Outbound TCP file send cap in KB/s (0 = unlimited), stored in QSettings
- [x] Zip backup / restore of private data under `~/.qipmsg` (stored zip, no Qt Sql)
- [x] Basic screenshot annotation (pen, rectangle, arrow, text, undo) before the existing PNG attach
- [x] Local PNG/JPEG avatar for self and per-IP override under `avatars/` (not sent on the wire)
- [x] Local hash index of sent/received regular files (`filehash.json`, size + SHA-256, cap 2000, no Qt Sql). qipmsg peers skip TCP when the content is already local; FeiQ still full transfer
- [x] qipmsg shared folder (path + optional password) and **Browse peer share** on the file port. Paths stay under the share root. FeiQ share protocol is not implemented
- [x] Light / dark / follow-system appearance in QSettings, with a dark QSS next to the 2.4 light theme
- [x] Optional RSA text encryption (default off). `GETPUBKEY` / `ANSPUBKEY`, zero IV, PKCS#1 plus AES-256 or Blowfish-128. Attachments and peers without a parsed key stay classic plaintext. FeiQ and official IPMsg packing were not verified
- [x] Offline queue persists across restart, retries on startup and when a peer is reachable, shows a count, and can hold a shake token. Files stay out
- [x] Windows NSIS user-level installer from the `windeployqt` folder (`packaging/qipmsg.nsi`)
- [x] Chat composer bold / italic / underline / size / color. Wire body is plain text. Optional HTML sidecar is qipmsg-only (command 126). No OLE/RTF blob as the message body
- [x] Screenshot region snap to nearby top-level windows; hold Alt (Option on macOS) to turn snap off. Pen / rect / arrow / text annotation stays after capture
- [x] Discussion group notice in `discussions.json` (no Qt Sql). Banner when non-empty. Change is a plain SENDMSG to online/away members (`MULTICASTOPT` when more than one). Not a FeiQ room
- [x] User skin folder or zip (`skin.json`, optional `theme.qss`, optional background). Broken QSS keeps the built-in light or dark sheet
- [x] Network preference can stick to one NIC by MAC (or interface name). Any IPv4 still discovers on every NIC
- [x] Per-discussion local folder in `discussions.json`. Online qipmsg members list it with command 126. Not a FeiQ room
- [x] qipmsg peer photo: `,av:` on the existing presence token and TCP command 126 `AVATAR`. Manual per-IP photo still overrides. FeiQ stays a color block
- [x] Discussion log in `grouplog/<id>.jsonl` (view, export, optional plain-text note). Not a FeiQ room. No Qt Sql
- [x] Doodle canvas (pen, four colors, clear) sent as the existing PNG attachment
- [x] Local reminders (`schedule.json`). Tray or main-window list. Fires only while the app is running; a missed time rings once on the next launch. Optional plain SENDMSG to a chosen peer. No FeiQ clock protocol
- [x] Chat favorites (`favorites.json`): selected text, a file path, or a peer note. Search, open, delete. No sync
- [x] Dial-up peers (`IPMSG_DIALUPOPT`) marked in the contact tree. Member recognition is also unicast to them. Preference to announce this host as dial-up. Normal LAN peers stay as they are
- [x] Windows network neighborhood: open `\\<ipv4>\` with `ShellExecute`. Other systems keep the action disabled. No SMB password UI
- [x] Local `--msg` command into the running process (per-user socket / named pipe). Plain `SENDMSG` by IPv4 or `login@host`. Not a plugin marketplace
- [x] Chat **Zone** and **Log** open local notes in `~/.qipmsg/zone/` (create, edit, delete). Not FeiQ `zone/*.db`

## How to try 2.52.16

1. Open a 1:1 chat. The toolbar shows Transfer (send file, send folder), Chat (screenshot, emoji, shake, find), and More. The window should not need a wide monitor just to show those buttons. File, folder, emoji, shake, and find run on one click. Screenshot opens full screen or region.
2. More groups share browse and the discussion folder, then doodle, favorites, zone, and log, then the disabled remote assist, remote control, and voice items. Those three still say they are not implemented. Nothing new is sent on the wire.
3. Profile in the header is on by default and shows the peer card. Click it again to hide the card and narrow the window. Close the chat and open it again: width, height, and the card switch match the last time (`[ChatWindow]` in `qipmsg.ini`). Dragging the splitter remembers the card width.

## How to try 2.50.15

1. Start the app. The contact window should appear immediately, and the taskbar should show qipmsg. Closing that window leaves the process in the notification area. The tray menu's first item, Open main window, shows it again. Preferences → Detail → "Start hidden in the notification area" (off by default) hides the window on the next start only when the tray icon is actually visible, and explains how to open the window once. With no tray, the window still opens.
2. Occupy port 2425, or on Windows put 2425 inside an excluded port range. Startup should explain in-use versus access denied (the excluded range is checked with `netsh interface ipv4 show excludedportrange` when that output is readable). Choosing the offered free port writes `udpPort` and uses it for discovery and file transfer without a restart. Closing the dialog or continuing anyway keeps the contact window open.
3. Free 2425 and start again. If the previous port was chosen in that dialog, qipmsg asks once whether to switch back. Switching back restores the FeiQ / IPMSG default. Dismissing the question keeps the current port and does not ask every launch. Preferences → Detail → Network can still change it.

## How to try 2.48.14

1. On Windows, right-click one peer and choose **Open network neighborhood**, or use **Network neighborhood** on the chat profile. Explorer should open `\\<that IPv4>\`. If the share needs a password, Windows asks. qipmsg does not. On Linux and macOS the same item is disabled and the tooltip says it is Windows-only. A hostname or a path is rejected.
2. Start qipmsg, then in another terminal run `qipmsg --msg 192.168.1.20 --text "hello"` or `qipmsg --msg login@host --text "hello"`. Success prints `OK sent <ip>`. A known offline peer prints `OK queued <ip>` only when the offline queue is on. If qipmsg is not running the exit code is 3 and no second window appears. Encryption, when enabled, uses the existing send path. This is not a plugin API.
3. In a chat, **More → Zone** and **Log** open the same local note window. Create a note, save, close, and open it again. The file is under `~/.qipmsg/zone/` with a `.md`, `.txt`, or `.markdown` suffix. Delete removes that file only. Nothing is sent. This is not a FeiQ zone database. Remote assist, remote control, and voice stay disabled.

## How to try 2.45.13

1. Main window **Schedule**, or the tray item. Add a reminder a minute ahead, leave QIpMsg open, and wait for the tray balloon (and the notice sound, unless message sounds are muted). Quit before a later reminder and start again: it shows once. Nothing runs after quit. Check **Notify peer** and pick someone: they get a normal text message, `Reminder: …`, not a private clock command. If they are offline and the offline queue is on, it waits in that queue.
2. In a chat, select a sentence and open **Favorites**. Save the text, save a file path, and save the peer memo if one is set. Search, open the file with the desktop handler, and delete a row. The file is `~/.qipmsg/favorites.json`. Nothing is uploaded.
3. Preferences, broadcast section: **I am a dial-up host** is off. Turn it on and Apply. Your presence packets include `IPMSG_DIALUPOPT`. Another qipmsg shows 拨号 on your row and, on its own refresh, also unicasts entry to you. A peer without that bit is not unicast just for being on the LAN. FeiQ's dial-up checkbox and icon still need a human check; this follows the IPMSG note that dial-up members may miss broadcasts.

## How to try 2.42.12

1. Set your photo. Another qipmsg should show it after the next presence refresh (`avatars/wire/`). FeiQ and classic IPMSG stay on the color block; the profile line says there is no photo packet this client can use. A manual photo for that IP overrides the fetched one. Clearing a fetched-only photo hides it until that peer publishes a different id.
2. In a discussion, add a group-log line. It is appended to `~/.qipmsg/grouplog/<id>.jsonl` and listed in the window. **Tell online members** sends one plain SENDMSG; uncheck it to keep the line local. Export writes `.txt` or `.jsonl`. Offline members are not queued. This is not a FeiQ room log.
3. In a 1:1 chat, open **More → Doodle**, draw, and send. The PNG goes out as a normal image attachment. An empty canvas does not send.

## How to try 2.39.11

1. Preferences → Detail → Appearance, or drop a folder/zip in `~/.qipmsg/skins/`. See BUILD for `skin.json`. Light, Dark, and Follow system remain. A QSS with unbalanced braces, `url(`, or `@import` is ignored and the built-in sheet stays. Reopen a chat to restyle bubbles.
2. Preferences → Detail → Network: pick one NIC instead of Any IPv4, then restart. `bindMac` / `bindInterfaceId` are written. Discovery broadcasts for other NICs stop. Switch back to Any IPv4 and they return. Linux tries `SO_BINDTODEVICE` (permission failure is logged, the IPv4 bind remains). macOS sets `IP_BOUND_IF`. Windows sets `IP_UNICAST_IF`. None of these bind a MAC address itself.
3. In a discussion, **Set folder**, then **Tell the group**. The side list is local. Online members get a plain SENDMSG that includes `QIPSHARE:<id>`. In the 1:1 chat, **More → Group folder** lists and downloads only that directory. The owner must stay online. FeiQ shows the text and cannot browse. Personal share LIST/GET is unchanged. This is not a FeiQ multicast room.

## How to try 2.36.10

1. In a chat, use the toolbar above the input: **B**, **I**, **U**, size, and color. FeiQ receives only the plain sentence and should display it as before. A second qipmsg shows the same formatting. Pasted HTML is reduced to that subset; RTF and NUL-bearing clipboard data insert as plain text. Encrypted and sealed sends keep formatting on this computer and do not send the HTML sidecar. The offline queue stores plain text only.
2. Ctrl+Shift+A and drag. The rectangle snaps to nearby window edges (Windows enumerates top-level windows; Linux `dlopen`s `libX11.so.6` and otherwise snaps to the screen only; macOS uses the window list when CoreGraphics returns one). Hold Alt, or Option on macOS, while dragging to disable snap. Enter or double-click still opens the pen / rectangle / arrow / text editor from 2.27.5.
3. Open a discussion and choose **Group notice**. Saving text shows a banner and sends one plain SENDMSG to each online or away member (`MULTICASTOPT` when there are two or more). Clearing it hides the banner and sends a short clear line. Grey offline members are not queued. If nobody is reachable, the notice stays in `~/.qipmsg/discussions.json` only. This is not a FeiQ multicast room.

## How to try 2.33.9

1. Preferences → Send/Receive → **Encrypt messages when the peer has a public key** (off by default). The first apply writes `~/.qipmsg/rsa_private.json` and can pause on the UI thread while a 1024-bit key is generated. Presence then sets `ENCRYPTOPT`. Two qipmsg peers with the option on exchange keys and show **Encrypted** on text bubbles. A message with a file attachment is not sealed. A FeiQ peer that ignores `GETPUBKEY` still receives plaintext. Do not treat FeiQ or official IPMsg decrypt as done; a failed open shows a clear bubble and still ACKs.
2. Send text and a window shake to a grey peer, quit, and start again. The main-window header, the queue dialog, and the tray item still show the count (`offline_queue.json`). When that login and host are online on exactly one new IP, the items move to it. Two matches are left on the old IP. A file-only send says nothing was queued.
3. After `windeployqt`, `makensis -DVERSION=2.33.9 -DDEPLOYDIR=<win-deploy> packaging/qipmsg.nsi` writes a per-user installer. The Windows GitHub Actions job uploads it as `qipmsg-windows-setup`.

## Later / out of scope for 2.30.6

1. **FeiQ plugin system**
2. **FeiQ share-protocol interop** (qipmsg↔qipmsg browse is done; FeiQ's packet format was not verified)
3. **FeiQ multicast group / 群 chat rooms** (shared room membership, full room sync). Local multi-send 讨论组, the 2.36.10 notice string, the 2.39.11 per-discussion folder, and the 2.42.12 group log are not that protocol. FeiQ custom-face packets were not verified.
4. **Voice**
5. **More installers**
   - Windows NSIS from the windeployqt folder ships with 2.33.9
   - `.app` bundle + DMG for macOS
   - Flatpak / AppImage / DEB / RPM for Linux

## How to try 2.30.6

1. Send one regular file from qipmsg to qipmsg. After it lands, `~/.qipmsg/filehash.json` has its size and SHA-256. Send the same file again into a receive folder that already holds that content: both sides note an instant skip and no file bytes move. A directory is not eligible. A FeiQ peer must still get the old file-list shape (no `0x71` hash) and a full TCP transfer. Check that with a real FeiQ build. Resume of a partial FeiQ transfer is unchanged.
2. Preferences → Detail → File: enable **Share a folder with qipmsg peers**, pick a directory, optional password. Your own contact row shows 享. The peer must be qipmsg with sharing advertised (`qipmsg,share` in the fourth presence field). **More → Browse peer share** lists the folder (200 entries) and downloads into the receive directory. A path outside that root is refused. Opening the button against FeiQ shows the limit and does not open TCP.
3. Preferences → Detail → **Appearance**: Light, Dark, or Follow system. Dark keeps chat bubbles and the contact tree readable. Reopen a chat to restyle bubbles that were already drawn. The value is `[Appearance] theme` in `qipmsg.ini`.

## How to try 2.27.5

1. Preferences → Detail → File, or the transfer window: set **Send speed limit** to 256 KB/s. `0` is unlimited. 1024 KB/s is 1 MB/s. The value is `sendSpeedLimitKBps` in `qipmsg.ini`. Outbound TCP file sends share that cap. Cancel still closes the socket.
2. Tray **Backup private data...** writes an uncompressed zip of preferences, `peers.json`, history, stickers, `group_policy.json`, `recent.json`, `resume.json`, discussions, the offline queue, and `avatars/`. `recv/` and `screenshots/` are omitted. **Restore private data...** asks before overwrite and then asks to restart. Re-zip with a compressor and the restore will refuse it.
3. Ctrl+Shift+S / Ctrl+Shift+A still capture. The annotation window (pen, rectangle, arrow, text, undo) runs before the PNG is attached. Esc cancels and does not send.
4. Click the header avatar to set your photo (`avatars/self.png`). Right-click a peer for a local photo keyed by IP. FeiQ, and qipmsg peers who have not set the same file, still see a color block. Nothing is added to the IPMSG packet.

## How to try 2.23.4

1. Right-click a department group → **Send to this group**. Online and away members are selected; blacklist and grey offline rows are not. Confirm only when the composer already has text or files. The send is the existing per-peer message with `IPMSG_MULTICASTOPT` when more than one address is selected. **Select group** is the same set and does not send by itself.
2. **Recent contacts** lists up to 20 peers from `~/.qipmsg/recent.json` (seeded from `history/*.jsonl` when that file is empty). Click a row to open chat. Discussion transcripts (`disc-*`) are not contacts.
3. With **Show my typing status** on, typing emits UDP command 121 at most every 2 seconds; idle, clear, or send emits 122. The peer caption shows **Typing…** for 5 seconds after each 121. FeiQ may ignore packets whose version is not `1_lbt…`. Confirm that against a real FeiQ build before treating the hint as interoperable.
4. While a file is moving, open the transfer window and **Cancel transfer**. The sender shuts down the TCP socket. The receiver abort also sends `RELEASEFILES`. ETA uses bytes written this session; directory trees show speed without a finish time. Sender-side cancel has no extra opcode.

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
