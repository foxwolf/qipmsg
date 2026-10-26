# QIpMsg Build Guide / 构建指南

QIpMsg now uses **CMake** and supports **Qt 6 (preferred) / Qt 5.12+** on Windows, Linux and macOS.

[![Build](https://github.com/foxwolf/qipmsg/actions/workflows/build.yml/badge.svg)](https://github.com/foxwolf/qipmsg/actions/workflows/build.yml)

---

## Requirements

- CMake >= 3.16
- C++17 compiler
- Qt 6 with modules: `Core`, `Core5Compat`, `Widgets`, `Network`, `Multimedia`, `LinguistTools`
  - OR Qt 5.12+ with modules: `Core`, `Widgets`, `Network`, `Multimedia`, `LinguistTools`

> 注：Qt 6 已移除 `QTextCodec`，因此需要 `Qt6::Core5Compat` 来保持 GBK/GB2312 编解码能力。若你的 Qt6 未安装 Core5Compat，可使用 Qt5 构建。

---

## Linux

### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
# Qt6 path (requires core5compat)
sudo apt-get install -y qt6-base-dev qt6-5compat-dev qt6-multimedia-dev qt6-tools-dev qt6-tools-dev-tools libgl1-mesa-dev
# OR Qt5 path
sudo apt-get install -y qtbase5-dev qtmultimedia5-dev qttools5-dev qttools5-dev-tools

cd /path/to/qipmsg
mkdir build && cd build
cmake ..
cmake --build -j$(nproc)
```

Region-capture snap on Linux loads `libX11.so.6` at runtime (`dlopen`). The package is not a link dependency. Without it, the selection only snaps to the screen edges. `qipmsgP2Check` covers the rich-text subset, snap math, discussion notice JSON, user-skin loading, and NIC selection. `qipmsgShareCheck` also covers discussion-folder `GLIST` / `GGET`. `qipmsgP3Check` covers schedule and favorite JSON, the `IPMSG_DIALUPOPT` bit, the Windows share address filter, the local `--msg` plan, and zone note files.

Run locally without installing:

```bash
./src/qipmsg
```

Install:

```bash
sudo cmake --install .
```

---

## Windows

### Option A: MinGW

1. Install [MSYS2](https://www.msys2.org/).
2. Install toolchain and Qt:

```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
# Qt6 (with 5Compat)
pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-5compat mingw-w64-x86_64-qt6-multimedia mingw-w64-x86_64-qt6-tools
# OR Qt5
pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-multimedia mingw-w64-x86_64-qt5-tools
```

3. Build:

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j$(nproc)
```

### Option B: MSVC (Visual Studio)

1. Install Visual Studio 2019/2022 with C++ workload.
2. Install Qt via [official installer](https://www.qt.io/download) or vcpkg.
3. Build:

```cmd
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64 ..
cmake --build . --config Release
```

Deploy dependencies with `windeployqt` after building:

```cmd
C:\Qt\6.x.x\msvc2019_64\bin\windeployqt.exe Release\qipmsg.exe
```

User-level NSIS installer (no admin). Copy the `windeployqt` output into one directory, then:

```cmd
makensis -DVERSION=2.36.10 -DDEPLOYDIR=C:\path\to\win-deploy -DOUTFILE=qipmsg-2.36.10-setup.exe packaging\qipmsg.nsi
```

`VERSION` must match `src/version.h`. The script does not embed a version of its own. `OUTFILE` is optional; when omitted, the installer is written next to `packaging/qipmsg.nsi`. Installs to `%LOCALAPPDATA%\QIpMsg` and adds a Start Menu shortcut. GitHub Actions `windows-latest` installs NSIS with Chocolatey when `makensis` is missing, uploads `qipmsg-<version>-setup.exe` as artifact `qipmsg-windows-setup`, and still uploads the `win-deploy` folder. Every OS job checks that `src/version.h`, `CMakeLists.txt`, and `docs/README.md` carry the same version and that `packaging/qipmsg.nsi` exists.

---

## macOS

Install Xcode Command Line Tools and Qt via Homebrew:

```bash
brew install cmake qt@6
# If you prefer Qt5
brew install qt@5

mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
cmake --build . -j$(sysctl -n hw.ncpu)
```

Deploy app bundle (optional):

```bash
/opt/homebrew/opt/qt@6/bin/macdeployqt src/qipmsg.app
```

---

## CI

GitHub Actions (`.github/workflows/build.yml`) runs a CMake **Release** build on every push and pull request:

| Runner | Qt 6 install |
|--------|----------------|
| `ubuntu-latest` | apt: `qt6-base-dev qt6-5compat-dev qt6-multimedia-dev qt6-tools-dev qt6-tools-dev-tools libgl1-mesa-dev` |
| `windows-latest` | `jurplel/install-qt-action@v4` Qt 6.8.3 MSVC 2022 (`qt5compat` + `qtmultimedia`) |
| `macos-latest` | Homebrew `cmake` + `qt@6`, `CMAKE_PREFIX_PATH=$(brew --prefix qt@6)` |

CMake ≥ 3.16, C++17, Qt 6 + Core5Compat. No Qt Sql. Each job uploads a `qipmsg-<os>` artifact. The Windows job also uploads `qipmsg-windows-setup` when `makensis` finishes.

[![Build](https://github.com/foxwolf/qipmsg/actions/workflows/build.yml/badge.svg)](https://github.com/foxwolf/qipmsg/actions/workflows/build.yml)

---

## Legacy qmake build (optional)

`src/qipmsg.pro` is still maintained for quick qmake builds:

```bash
cd src
qmake
make
```

---

## Runtime notes

- On first run the app creates `~/.qipmsg/` (Linux/macOS) or `%USERPROFILE%\.qipmsg\` (Windows) for settings (`qipmsg.ini`), the lock file, internal log, **per-peer chat history** in `history/<ip>.jsonl` (discussion transcripts use `history/disc-<id>.jsonl`), last-seen peers in `peers.json`, discussion groups in `discussions.json` (named member keys `user@host/ip`; local multi-send UX, **not** FeiQ multicast group protocol; no Qt Sql), file-resume offsets in `resume.json`, **offline text queue** in `offline_queue.json` (plain JSON, no Qt Sql), department-group hide/mute flags in `group_policy.json`, custom stickers in `stickers/` with an index `stickers.json`, local avatars in `avatars/` (`self.png`, per-IP manual PNG copies, and `wire/` PNGs fetched from qipmsg peers), discussion logs in `grouplog/<id>.jsonl`, default receive files in `recv/`, screenshots in `screenshots/`, optional `rsa_private.json` / `peer_pubkeys.json` when message encryption has been turned on. Tray **Backup private data** stores an uncompressed zip of the private files above except `recv/`, `screenshots/`, the lock file, and the internal log.
- Transfer codec defaults to **GBK** (FeiQ / 飞鸽). Change it under Preferences → Detail/Log → Transfer. Decode tries the configured codec, then GBK, GB2312, and UTF-8, unless the datagram's command has `IPMSG_UTF8OPT` (`0x00800000`), in which case the whole datagram is decoded as UTF-8.
- `BR_ENTRY` / `BR_ABSENCE` / `BR_EXIT` / `ANSENTRY` announce `IPMSG_FILEATTACHOPT | IPMSG_CAPUTF8OPT | IPMSG_CLIPBOARDOPT`. `ANSENTRY` previously omitted even the file-attach bit; it now uses the same capability word. Presence text stays on the configured codec and sets `IPMSG_UTF8OPT` only when that codec is UTF-8, so FeiQ 2013 keeps reading GBK nicknames. A directed `SENDMSG` sets `IPMSG_UTF8OPT` and is encoded as UTF-8 when the peer has advertised `CAPUTF8OPT` (or when our codec is already UTF-8). Clipboard images are still ordinary file attachments; the capability bit is an announcement, not a new file type on the wire (`IPMSG_FILE_CLIPBOARD` is defined but not sent, so FeiQ file receive stays unchanged).
- Cross-subnet discovery: Preferences stores extra targets in `userSpecifiedBroadcastIp` (same `\r`-separated list). A token may be one IPv4, `start-end`, or `addr/prefix`. Each refresh expands it and unicasts `BR_ENTRY` there. One token is limited to 1024 addresses. There is no directory server.
- Broadcast to all online sends `IPMSG_SENDMSG | IPMSG_SENDCHECKOPT | IPMSG_BROADCASTOPT` once per reachable peer (online or away). It does not also write the same text to the subnet broadcast address. Stale peers are not queued.
- Presence refresh interval and auto-reply live in Preferences (Detail tab + Send/Receive). Tray **Leave → Away / Auto-reply** toggles absence (`BR_ABSENCE`). Tray **Leave → Stealth / Hide mode** (also Preferences → Send/Receive) skips `ANSENTRY` and own `BR_ENTRY`/`BR_ABSENCE`; turning it on sends `BR_EXIT` first.
- Local memo nicknames are stored in `peers.json` as `"memo"` (same sidecar file, no Qt Sql). The contact tree and chat title show the memo; peer-profile **Nickname** keeps the real name.
- Contact search matches nickname / host / login / memo / IP / group. Pinyin initials use public-domain GB2312 first-letter ranges via `QTextCodec` GBK/GB2312 (no third-party library). Full pinyin is a compact built-in table of surnames and common words in `src/pinyin.cpp` (public readings, GPL-3.0-or-later with the rest of QIpMsg).
- Offline queue (FeiQ-like 在线自动发送, best-effort): sending text or a window shake to a stale/offline/unknown peer enqueues it under `offline_queue.json` (atomic replace, no Qt Sql). The file survives restart. Default is auto-queue (Preferences → Send/Receive; uncheck to confirm each time). Flush runs at startup, every few seconds, and when `UserManager` marks the IP `PRESENCE_ONLINE` or `PRESENCE_AWAY` (typically `BR_ENTRY` / `ANSENTRY` / `BR_ABSENCE`). If the old IP is gone but exactly one reachable peer has the same non-empty login and host, the item is retargeted. History is written only on flush. A failed handoff to the send thread puts the item back. Files are not queued — the UI says so. The main-window header, queue dialog, and tray show the count.
- Optional message encryption is off by default (Preferences → Send/Receive). Turning it on creates `rsa_private.json` (owner read/write) and adds `ENCRYPTOPT` to presence only while the preference stays on. `GETPUBKEY` / `ANSPUBKEY` follow the IPMSG draft (`caps:exponent-modulus`, then `caps:rsa-pkcs1(session):pkcs7-cbc` with a zero IV). qipmsg seals text with RSA-1024 and AES-256 or Blowfish-128. Peers with no parsed key, and any packet that carries a file, stay classic plaintext so unknown commands do not replace a message FeiQ can read. Official IPMsg CryptoAPI blobs and FeiQ encryption were not verified.
- File receive options live in Preferences → Detail → File: resume (`GETFILEDATA` hex offset), auto-accept, auto-rename, default folder. Directory transfers restart from the tree root (IPMSG dir stream); only regular files persist an offset.
- UDP listen port and NIC choice live in Preferences → Detail → Network (`qipmsg.ini` `[Network]` `udpPort`, `portFallback`, `bindAddress`, `bindMac`, `bindInterfaceName`, `bindInterfaceId`). Default **2425** / any IPv4 (`0.0.0.0`). UDP and TCP file listen share that port. Changes made in Preferences need a restart. At startup, if the port cannot be opened, qipmsg explains in-use vs access denied (on Windows it also reads `netsh interface ipv4 show excludedportrange` when that list is available) and can save a free `udpPort` for this run. Port 2425 is kept whenever it is free. A dialog-chosen non-default port sets `portFallback`; the next start offers to return to 2425 once it is free, and either answer clears the flag. Closing the dialog does not quit the app or hide the contact window. If the port still cannot be opened, the failure is logged and the app continues (soft-fail); file receive needs a successful TCP listen. **Any IPv4** still discovers on every up, non-loopback NIC. Choosing a NIC stores its MAC as `bindInterfaceId` when a MAC is reported, otherwise the interface name, and listens on that NIC's current IPv4 (DHCP can change the address; the MAC is the id). Other NICs are not used for discovery while a choice is saved, and a missing NIC does not fall back to them. Extra broadcast-list targets are still probed through the chosen NIC. Linux then tries `SO_BINDTODEVICE` (it can fail without permission; the IPv4 bind remains). macOS sets `IP_BOUND_IF`. Windows sets `IP_UNICAST_IF`. No platform here binds a socket to a raw MAC address; outbound frames use that NIC's MAC because the route is pinned to the interface.
- FeiQ 2013 / IPMSG file-transfer assumptions (no new protocol):
  - TCP `GETFILEDATA` / `GETDIRFILES` command field is accepted as **decimal or hex** (FeiQ often sends hex `60`/`62`; classic IPMSG and older qipmsg send decimal `96`/`98`). Outgoing requests use hex.
  - File-list and stream sizes/offsets are hex. Qt `QString::arg` width is a minimum, so values above 4 GB are sent in full hex (not truncated to 8 digits).
  - Packet-id lookup tries the raw field, hex→decimal, and decimal string so FeiQ and qipmsg map keys match.
  - Resume offset is hex on the wire; `resume.json` stores size/offset as decimal strings (old double values still load). Directory streams have no per-file resume.
  - TCP connect uses the peer’s last UDP source port (fallback 2425), with an 8 s timeout. If that address refuses the connection, times out, or closes before any file bytes arrive, qipmsg tries up to three more targets: other contacts with the same login, host, and group, then a single IPv4 from `userSpecifiedBroadcastIp` (a range or CIDR is not expanded). Addresses in the same /24 are preferred. A socket that has already delivered file bytes is not replaced. The receive-progress timer is started on the UI thread (`MsgWindow`), so a worker no longer calls `QTimer::start`. Regular-file reads are capped at remaining bytes.

## Multi-homed FeiQ file receive (2.53.18)

FeiQ can emit the file-attach UDP datagram from one NIC (for example 192.168.14.164) while its TCP file server listens on another (for example 192.168.14.3). qipmsg still tries the UDP source first. On failure it tries the other known addresses described above, at most four in total. Put the address that actually listens into the broadcast list as one IPv4, or let discovery record another contact row with the same login, host, and group. A ZeroTier-only contact list, with no LAN unicast in that list, can still be closed by FeiQ. `qipmsgP1Check` covers the candidate order, including 192.168.14.164 versus 192.168.14.3 and a ZeroTier source.
- Personal signature is sent as a third NUL field after `nickname\0group\0` on `BR_ENTRY` / `ANSENTRY` / `BR_ABSENCE`. Classic IPMSG ignores extra fields; FeiQ `PersonSign` uses the same extra slot.
- Window shake is a normal `IPMSG_SENDMSG` whose text is `#@#shake#@#` (also accepted: `{shake}`, `SHAKE`, `窗口抖动`, `#qipmsg-shake#`, and ESC+SHAKE). FeiQ peers that do not implement shake treat it as ordinary text.
- Screenshot uses `QScreen::grabWindow` (full screen or rubber-band region). A basic editor (pen, rectangle, arrow, text, undo) runs before the PNG is saved and sent through the existing file-attach path. Wayland/headless sessions may return an empty grab.
- Outbound TCP file sends honor `sendSpeedLimitKBps` in `qipmsg.ini` (`[FileRecv]`, 0 = unlimited, cap 102400). Parallel sends share one token bucket. The transfer window shows and can change the same value.
- Chat bubbles show a thumbnail for `png` / `jpg` / `jpeg` / `gif` / `bmp` / `webp` attachments (outgoing local path, incoming after receive into the save folder, plus path/filename in history). Click opens the file with the desktop handler. Clipboard paste (`Ctrl+V` of an image) saves under `screenshots/` and attaches it — the same IPMSG file-attach path FeiQ uses.
- The toolbar emoji pack inserts Unicode into the compose box and sends it as ordinary `IPMSG_SENDMSG` text (no new protocol, no Qt Sql). FeiQ 2013 typically uses GBK, which cannot encode most emoji; those peers may see `?`. Use Preferences → Transfer → UTF-8 for Unicode emoji interop. Incoming FeiQ-style tokens such as `/微笑` are rendered as emoji locally only.
- Custom stickers are files, not Unicode. Import copies PNG / GIF / WebP / JPEG / BMP into `stickers/` (max 8 MB, 400 files, 40 named groups). A click sends `[Sticker] …` plus `IPMSG_FILEATTACHOPT`, the same image-attach path as paste and screenshot. GIF animation is the file itself; the picker shows the first frame (WebP needs Qt's imageformats plugin for a thumbnail, but the file is still sent). FeiQ receives an ordinary image attachment, not a private face-id. Offline queue still refuses attachments.
- UI language defaults to **Simplified Chinese**. `qipmsg_zh_CN.qm` is embedded in `icons.qrc` (`:/translations`) and also copied next to the executable as `translations/qipmsg_zh_CN.qm`. Chinese system locales (`zh_*`) try that locale first, then `zh_CN`. Other locales load `zh_CN` as the product default. Qt `qt`/`qtbase` catalogs are loaded from the same search path plus Qt’s own translations directory when present (so standard OK/Cancel buttons are Chinese when those files exist).
- To refresh the catalog after editing `tr()` strings: `lupdate src -ts src/translations/qipmsg_zh_CN.ts` then `lrelease src/translations/qipmsg_zh_CN.ts -qm src/translations/qipmsg_zh_CN.qm`.

## Session window layout (2.52.16)

The chat toolbar is grouped so a long row of text buttons does not set the window's minimum width. **Transfer** is send file and send folder. **Chat** is screenshot (full screen or region), emoji, shake, and find. **More** holds browse peer share, group folder, doodle, favorites, zone, and log, then the still-disabled remote assist, remote control, and voice items. Everyday actions stay one click, or one click plus the screenshot menu. The header **Profile** button shows or hides the peer card (shown by default). Closing a chat writes `[ChatWindow]` in `qipmsg.ini`: `width`, `height`, `maximized`, `profileVisible`, and `splitter` when the card is open. The next chat window reuses that size. No Qt Sql, and no new packet.

## Startup window and listen port (2.50.15)

The contact window opens at launch. `[System] startInTray` (Preferences → Detail, default off) hides it only when the tray icon is actually visible; a missing tray still shows the window and quits when that window closes. The tray menu's first item is **Open main window**. Closing the contact window does not quit while a tray is available.

UDP and TCP are probed on the configured port before the real sockets listen. The probe is exclusive, so another FeiQ / IPMSG on 2425 counts as in use even though the live UDP socket still sets `ShareAddress`. Access denied is classified separately from "address already in use". On Windows, an access-denied failure also checks `netsh interface ipv4 show excludedportrange` for that port. Accepting a free port writes `udpPort` and, when it is not 2425, `portFallback=true`. The next successful start then asks once whether to move back to 2425. `qipmsgPortCheck` covers classification, excluded-range text, the 2425-first candidate list, and rebinding a port after the probe releases it. On Windows the exclusive probe can still see the port as busy for a moment after `close`, so the self-check retries that probe for about two seconds and prints the UDP and TCP errors if it never clears.

## Network neighborhood, local send, and notes (2.48.14)

No Qt Sql. No plugin API. Remote assist, remote control, and voice stay disabled.

### Windows shares

On Windows, the contact menu **Open network neighborhood** and the chat profile button **Network neighborhood** call `ShellExecuteW` on `\\<ipv4>\`. The address must be four decimal octets with no leading zeros, not `0.0.0.0`, and not class D or E. Explorer asks for SMB credentials. qipmsg does not collect them. On Linux and macOS the action stays visible and disabled, with a tooltip that it is Windows-only. `qipmsgP3Check` checks the address filter and does not launch Explorer.

### Local command

While qipmsg is running it listens on a per-user local socket (a named pipe on Windows). Access is limited to the current user. The name is `qipmsg-msg-` plus 12 hex digits of the SHA-256 of `~/.qipmsg`. It is not a TCP port.

```
qipmsg --msg 192.168.1.20 --text "hello"
qipmsg --msg login@host --text "hello"
```

`login@host` matches one known peer, case-insensitive. An IPv4 that is not in the contact list is sent to port 2425, or to that peer's port when it is known. A known offline peer is queued only when the offline-queue preference is on; otherwise the command prints `ERR offline`. Success prints `OK sent <ip>` or `OK queued <ip>`. Exit 2 is a bad command line. Exit 3 means qipmsg is not running; this process does not open a window. The text is a normal `SENDMSG`, including the encrypt preference. The limit is 4000 characters and 12000 UTF-8 bytes. This is not a plugin marketplace.

### Local notes

**More → Zone** and **Log** open the same browser for `~/.qipmsg/zone/` (created if missing). Create, edit, and delete `.md`, `.txt`, and `.markdown` files there. Nothing is sent to peers. This is not FeiQ's `zone/*.db` blog. The private-data zip includes `zone/`.

## Schedule, favorites, and dial-up hosts (2.45.13)

No Qt Sql. No new wire opcode.

### Reminders

Main window **Schedule** and the tray item edit `~/.qipmsg/schedule.json` (at most 200). A timer checks while the process is running and shows a tray balloon. If the platform has no tray balloons, a dialog is used. The notice sound plays unless **No sound alarm** is on. Do-not-disturb does not hide a reminder you created. Quit does not leave a daemon. A time that passed while the program was closed is shown once on the next launch. **Notify peer** sends ordinary `SENDMSG` text (`Reminder: …`) through the existing send path, including the offline queue and the encrypt preference. There is no FeiQ `CLOCK` packet.

### Favorites

**More → Favorites** used to be a disabled toolbar button. It now writes `~/.qipmsg/favorites.json` (at most 300): selected chat text, an absolute file path, or a note about that peer. Search, open, and delete stay on this computer. Opening a file uses the desktop handler. Nothing is synced.

### Dial-up

`IPMSG_DIALUPOPT` (`0x00010000`) is the IPMSG flag for a member who may not receive subnet broadcasts. IPMSG `protocol.txt` says member-recognition commands are sent to that member one by one. qipmsg reads the bit on entry, answer, and absence (not on chat text). The contact row shows **Dial-up** and an amber badge. `BR_ENTRY`, `BR_ABSENCE`, and `BR_EXIT` are still broadcast, and the same command is unicast to known dial-up peers. Their `BR_ENTRY` is still answered with the existing `ANSENTRY` only, so two dial-up qipmsg peers do not bounce entry packets. **I am a dial-up host** (off by default, next to the broadcast list) sets the bit on our own presence packets. Peers without the bit are unchanged. The mark is stored in `peers.json` until a later presence packet clears it. FeiQ's dial-up icon was not compared packet by packet.

The private-data zip includes `schedule.json` and `favorites.json`.

## Avatars, group log, and doodle (2.42.12)

No Qt Sql. Version stays `1`.

### Peer photo

FeiQ stores custom faces locally (`UserHeadPhoto`, `CustomFace`, `RecvFace`, `SendFace`). Public IPMSG text and the captures used for typing (121/122) do not define a photo opcode. qipmsg does not switch its version string to `1_lbt…` and does not attach the photo as a file, because FeiQ would show a receive dialog.

When `avatars/self.png` exists, presence keeps the fourth NUL field that already says `qipmsg` or `qipmsg,share` and appends `,av:<12 hex>` of a scaled PNG (`avatars/self-wire.png`, at most 96 KB). Another qipmsg reads that tag and asks on the file port with TCP command 126, verb `AVATAR`. The reply is `QIP1` plus the PNG, saved under `avatars/wire/`. A manual per-IP photo still wins. FeiQ and classic IPMSG keep the color block; the profile line says so. Clear of a fetched-only photo hides that id until the peer publishes a different one. A human still needs to confirm FeiQ ignores the extra comma tag.

### Discussion log

`~/.qipmsg/grouplog/<id>.jsonl` is append-only, one JSON object per line (`t`, `who`, `text`). The discussion window lists the latest entries and can export `.txt` or `.jsonl`. The file stops accepting entries at about 2 MB. **Tell online members** sends one plain `SENDMSG` (`MULTICASTOPT` when more than one member is reachable). Offline members are not queued. This is not a FeiQ room log. The private-data zip includes `grouplog/`.

### Doodle

**More → Doodle** is a pen, four colors, and clear. Send writes a PNG under `screenshots/` and uses the existing image attachment. An empty canvas is refused. There is no pressure-sensitive tablet path.

## User skins, NIC choice, and discussion folders (2.39.11)

No Qt Sql. Light / dark / system appearance stays.

### Skin pack

Put a folder or a zip under `~/.qipmsg/skins/` (Windows: `%USERPROFILE%\.qipmsg\skins\`). The id is the folder name, or the zip name without `.zip`. Ids are 1–64 characters: letters, digits, `.` and `-`. Preferences → Detail → **Import skin** copies a folder or zip into that directory. `[Appearance] theme` is `light`, `dark`, `system`, or `skin:<id>`.

`skin.json`:

```json
{
  "name": "Ocean",
  "base": "dark",
  "qss": "theme.qss",
  "background": "background.png",
  "colors": {
    "window": "#0B1D36",
    "windowText": "#E8F1FF",
    "base": "#12263F",
    "text": "#E8F1FF",
    "button": "#16324F",
    "buttonText": "#E8F1FF",
    "highlight": "#3B82F6",
    "highlightedText": "#FFFFFF",
    "bubbleIncomingBg": "#16324F",
    "bubbleIncomingFg": "#E8F1FF",
    "bubbleOutgoingBg": "#2563EB",
    "bubbleOutgoingFg": "#FFFFFF",
    "bubbleMeta": "#93C5FD"
  }
}
```

`base` is `light` or `dark` (default light) and selects the built-in sheet and palette. Color keys are optional. `theme.qss` is appended after that sheet. It is ignored when it is empty, contains a NUL, is over 256 KB, has unbalanced braces, or contains `url(`, `@import`, `expression(`, `javascript:`, `http://`, or `https://`. The built-in sheet stays. `background` is an optional PNG/JPEG/BMP/WebP inside the pack (max 4 MB), drawn behind the main window and chats. A zip may use store or deflate (the inflater is zlib's puff, vendored as `src/puffInflate.cpp`). Encrypted entries, `..`, and archives over 8 MB are rejected. A missing pack falls back to light. Skin files are not in the private-data zip.

### Discussion folder

A discussion object in `discussions.json` may include `sharePath`, an absolute directory on that computer. Empty means the key is omitted. The discussion window lists names in that folder. **Tell the group** sends one plain `SENDMSG` (with `MULTICASTOPT` when more than one member is reachable) containing the names and a line `QIPSHARE:<id>`. Grey offline members are not queued.

A member opens the 1:1 chat and chooses **More → Group folder**. That sends TCP command 126 `GLIST` / `GGET` on the file port, only to a reachable qipmsg peer, and only while the owner is online. The owner serves the file when the TCP peer's IPv4 is a member of that discussion. Paths cannot leave `sharePath`. Personal `LIST` / `GET` is unchanged; an older qipmsg that does not know `GLIST` rejects the verb and does not open the personal share. FeiQ shows the text and has no browser for this folder. This is not a FeiQ multicast room.

## Version

**2.42.12** adds qipmsg photo exchange on command 126, a local discussion log, and a doodle sent as an image attachment, on top of **2.39.11**. FeiQ photo packets were not verified.

**2.39.11** adds the skin pack, NIC interface id, and discussion folder above, on top of **2.36.10**.

**2.30.7** includes `QStyleHints` so Qt 6.5 follow-system appearance builds on Windows MSVC, on top of **2.30.6**.

**2.30.6** adds a local file-hash index with qipmsg instant skip, qipmsg shared-folder browse and download on the existing file port, and a light / dark / system theme, on top of **2.27.5**.

## Trying 2.30.6 instant skip, share browse, and dark theme

1. Send the same regular file between two qipmsg instances. The first copy uses TCP and is recorded in `~/.qipmsg/filehash.json` (size + SHA-256, cap 2000, missing files pruned). Send it again. If the receive folder already has the same name, size, and hash, or the index has another copy of that content, TCP is skipped and both chats show an instant-send note. Directories are never skipped. The hash extend-attr (`0x71`) and UDP command 126 (`INSTANT:…`) are sent only when the peer's presence token starts with `qipmsg`. FeiQ file lists stay the previous shape and still transfer the whole file. Resume is unchanged. A human FeiQ capture should confirm the file-list shape. `filehash.json` is an exact file in the private-data zip.
2. Preferences → Detail → File: share one folder and an optional password (stored in `qipmsg.ini`, same as other settings). Saving re-announces presence. A fourth NUL field after the signature is `qipmsg`, plus `,share` when sharing is on and `,lock` when a password is set. Classic readers stop at the signature NUL. The contact row shows a green 享 badge. **More → Browse peer share** lists up to 200 names and downloads into the receive folder. The TCP request is command 126 (`LIST` / `GET`) on the existing file port, and only when that peer advertised a qipmsg share. `..`, absolute paths, and symlink escapes outside the share root are rejected. FeiQ's share protocol has no verified packet format here, so the button explains the limit and does not connect.
3. Preferences → Detail → Appearance: Light, Dark, or Follow system. Light is the 2.4 messenger QSS. Dark is `themeDark.qss` plus palette colors for bubbles and the contact tree. The choice is `[Appearance] theme` = `light` / `dark` / `system`. Follow system uses `QStyleHints::colorScheme` on Qt 6.5+ and otherwise the window lightness at startup. Bubbles already drawn in an open chat keep their old colors until that chat is reopened.

**2.27.5** adds an outbound send cap, uncompressed zip backup/restore of private data, basic screenshot annotation, and local-only avatars, on top of **2.23.4**.

## Trying 2.27.5 send cap, backup, annotation, and local avatars

1. Preferences → Detail → File, or the transfer monitor: set the send limit to 256 KB/s (0 is unlimited). A large TCP send should stay near that rate. Cancel still aborts the socket. The limit is not advertised to the peer.
2. Backup from the tray or the File tab. The zip is STORE-only (no compression) so it round-trips in qipmsg. Restore asks before overwrite, then asks to restart. Do not recompress the archive. `recv/` and `screenshots/` are not part of the backup.
3. Ctrl+Shift+S and Ctrl+Shift+A still start a capture. Annotate, then Send. Esc drops the capture and does not attach a file.
4. The header avatar and a peer's **Set local photo** copy a PNG into `avatars/`. There is no FeiQ photo opcode. Other clients keep showing a color block.

**2.23.4** adds department-group send, a recent-contacts panel, FeiQ typing commands 121/122, and in-progress transfer cancel with an ETA, on top of **2.19.3** (per-group hide, mute, sticker groups).

## Trying 2.23.4 group send, recent contacts, typing, and transfer ETA

1. Right-click a department header (the FeiQ group field, not a local 讨论组) and choose **Send to this group**. Only online and away members outside the blacklist are selected. If the composer already has text or files, confirm to send through the existing per-IP path with `IPMSG_MULTICASTOPT` when more than one peer is selected. Grey offline peers are not queued. **Select group** lists the same live departments.
2. **Recent contacts** at the top of the list is built from the last history line per peer (`recent.json`, no Qt Sql). A single click opens that chat. `history/disc-*` discussion logs are skipped.
3. Preferences → Send/Receive → **Show my typing status** (on by default) sends command **121** while typing and **122** after about 2 seconds of idle, on clear, or on send. No option bits are OR-ed, and the version field stays `1`. Incoming 121/122 always show **Typing…** on the chat caption and the contact row. Classic IPMSG peers ignore the unknown mode. Captured FeiQ packets use a `1_lbt…` version string; a FeiQ build that only paints typing for that token may not show our hint. That needs a human check. Do not switch the global version to `1_lbt`.
4. Transfer window → **Cancel transfer**. A row that has not started TCP is removed at once. An active send calls `shutdown` on the socket (the serve thread closes it). Cancelling a receive also sends `IPMSG_RELEASEFILES` with the original packet number so the sender drops the offer. There is no IPMSG “sender cancelled” command; the receiver sees the TCP close. **Remaining** is speed and ETA from bytes written in this session. A folder has no total size, so the column shows speed and “remaining unknown”.

**2.19.3** adds per-department-group hide, per-group message mute, and importable sticker groups on top of **2.15.2** (login autostart, do-not-disturb, presence sounds, read receipts), **2.11.1** (cross-subnet ranges, broadcast-to-online, sealed letters, UTF-8/clipboard bits), and **2.7.0** (FeiQ/IPMSG TCP file interop).

## Trying 2.19.3 group permissions and stickers

1. Right-click a department header (the FeiQ group field, not a local 讨论组) or one of its members. **Hide from group** and **Mute messages** match Preferences → Send/Receive → **Group permissions**. Flags are in `group_policy.json`.
2. Hide: that group's `BR_ENTRY` is recorded locally but not answered. A unicast `BR_EXIT` is sent to known members so clients that already listed us can drop us. `GETABSENCEINFO` from a hidden group is not answered. Global stealth is unchanged (it still answers absence info). While any group is hidden, `announcePresence()` does not subnet-broadcast; it unicasts entry/absence to known peers outside hidden groups and unicast exit to hidden groups. Unknown hosts learn about us only when they send `BR_ENTRY` and their group is not hidden. Clearing every hidden group restores the subnet broadcast.
3. Mute: `WindowManager` still appends history and unread. It skips the tray balloon, notice sound, auto-raise, and shake. File-receive windows stay hidden the same way do-not-disturb hides them; a tray click can show them. Auto-reply is unchanged. A peer with no known department (never seen on entry) is not muted.
4. Emoji picker: Classic Unicode still inserts text. Import stores the image under `stickers/` and `stickers.json`. Click sends a file attachment. New/rename/delete group and right-click move/delete are local only.

**2.15.2** adds opt-in login autostart, do-not-disturb, presence sounds, and read-receipt bubble labels on top of **2.11.1** (cross-subnet ranges, broadcast-to-online, sealed letters, UTF-8/clipboard bits) and **2.7.0** (FeiQ/IPMSG TCP file interop).

## Trying the 2.15.2 tray and settings

All four are off until you turn them on (do-not-disturb and presence sound default off; autostart defaults off and never writes a login entry by itself).

1. Preferences → Detail → **Start with the system**. Windows: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` value `QIpMsg` (quoted when the path has spaces). Linux: `$XDG_CONFIG_HOME/autostart/qipmsg-autostart.desktop` or `~/.config/autostart/`. macOS: `~/Library/LaunchAgents/org.qipmsg.login.plist` (`RunAtLoad`, loaded on the next Aqua login, not via `launchctl` in the current session). Uncheck to delete only that entry.
2. Tray item **Do not disturb (don't pop message windows)** (not under Leave) or Send/Receive checkbox. While on, `shouldPopup()` is false and tray balloons are skipped. `recordIncomingText` still appends history and increments unread. Manual tray click / Open chat still works. `BR_ABSENCE` is unchanged.
3. Send/Receive → **Sound when peers come online or go offline**. Uses `sounds/info.wav` and `sounds/warning.wav` (override with `qipmsg.ini` `[Other] onlineSound` / `offlineSound`). Armed 8 seconds after start. Same IP is silent for 20 seconds after a beep; any presence beep closer than 1.5 seconds is dropped. Online ↔ away does not beep.
4. An outgoing message sent with **Encap** (`SECRETOPT|READCHECKOPT`) shows **Unread** under the bubble. A `READMSG` whose body is the original decimal packet number (or an empty body with that number in the packet-number field) switches the label to **Read** and sets `"rd": true` on that history line (`pkt` / `er` are optional JSONL fields). Opening a seal still sends `READMSG|READCHECKOPT` with the original packet number in the body. No new command or option bit.

## Trying the 2.11.1 protocol gaps

1. Preferences → broadcast addresses. Add `192.168.9.10-192.168.9.12` or `192.168.9.0/30`, click Apply. On the next refresh, a capture should show unicast UDP/2425 to each host, not a new server. A `/16` is rejected (more than 1024 hosts).
2. With two online peers, use tray **Broadcast to all online** or the main-window button. Each peer's chat shows `[Broadcast]` and the packet command includes bit `0x400`. An offline peer gets nothing and the offline queue stays empty.
3. From a chat, check **Encap** and send. The receiver's bubble says the letter is sealed and history has no body yet. Click the bubble: the text appears, and a `READMSG` packet goes back. `SECRETOPT` (`0x200` on `SENDMSG` only — the same bit is `SERVEROPT` on entry) and the older `READCHECKOPT` seal both take this path.
4. Default codec GBK: a FeiQ 2013 peer (no `CAPUTF8OPT`) still receives GBK without `UTF8OPT`. A peer that sets `CAPUTF8OPT` (`0x01000000`) on entry is answered in UTF-8 with `UTF8OPT` (`0x00800000`). Our entry packet itself carries `CAPUTF8OPT` and `CLIPBOARDOPT` (`0x08000000`). `qipmsgP1Check` covers range expansion and these flag helpers.

## Trying the 2.7.0 UI

1. Launch `qipmsg` (UI is Simplified Chinese, light messenger theme). The main window shows a rounded-avatar grouped list, signature field, and online count. Last-seen peers appear grey until presence refresh. A **讨论组** folder sits apart from FeiQ department groups.
2. Filter with 搜索好友 / 拼音 (`zs` / `zhang` for 张三). Right-click **设置备注** for a local memo. Multi-select two or more contacts, then 创建讨论组. Reopen the group from the list; the same text is sent to each member. @mention highlight is local only — the wire stays plain text. This is **not** FeiQ multicast chatroom.
3. Chat uses message bubbles (incoming left, outgoing right). Toolbar: 发送文件 / 发送文件夹 / 表情 / 窗口抖动 / 截图 / 查找; remote/voice/share/zone are disabled (暂未实现). Right side is 对方资料 with avatar, status, and signature. Image attachments show a thumbnail in the bubble; click to open. Paste an image with Ctrl+V. History JSONL may include an `img` array.
4. Tray **Find in history** searches every `history/*.jsonl` file.
5. Send a message from another IPMSG/FeiQ client: expect a tray balloon, unread tooltip, and the same chat window to append history.
6. Enable **Away / Auto-reply** from the tray Leave menu and send another message from a peer; you should see one automatic reply per cooldown. Shake packets are not auto-replied.
7. Enable **Stealth / Hide mode**. A second IPMSG/FeiQ client that refreshes after you hide should no longer see you (no `ANSENTRY`). A client that already had your IP can still send a directed message.
8. Send text or a window shake to a grey/offline peer: it sits in the chat **Offline queue** and tray **Offline message queue**, and is still there after a restart. When the peer comes back (`BR_ENTRY` / `ANSENTRY`), items flush in order. A file attachment is not queued; the dialog says so.
9. Preferences → Detail → Network: change UDP/TCP listen port or bind NIC, restart, then send/receive a file with FeiQ or another IPMSG client. Files larger than 4GB keep 64-bit size/offset on the wire and in `resume.json`.

## FeiQ stealth interop (best-effort)

IPMSG/FeiQ discovery is LAN UDP with no presence server. HideMode can only refuse to announce:

| Event | Stealth off | Stealth on |
| --- | --- | --- |
| Own `BR_ENTRY` / `BR_ABSENCE` (startup, refresh, nickname change) | send | skip |
| Peer `BR_ENTRY` | record peer + `ANSENTRY` | record peer, **no** `ANSENTRY` |
| Enter stealth | — | send `BR_EXIT` once |
| Directed `SENDMSG` / `GETABSENCEINFO` | answer | answer (protocol-safe) |
| Quit | `BR_EXIT` | skip (already left) |

Assumptions: FeiQ HideMode is treated as “do not answer discovery.” We still list others who broadcast. Global stealth still cannot be enforced by a server.

## FeiQ per-group hide (best-effort)

IPMSG has no group-scoped presence bit and FeiQ's per-group HideMode is not a published extension, so this build does not emit a private opcode.

| Event | Group not hidden | Group hidden |
| --- | --- | --- |
| Subnet `BR_ENTRY` / `BR_ABSENCE` | sent when no group is hidden | suppressed while any group is hidden |
| Known peer outside hidden groups | included in that broadcast | unicast entry/absence |
| Known peer inside a hidden group | unicast entry when the group is shown again | unicast `BR_EXIT` (also on each of their `BR_ENTRY`) |
| Their `BR_ENTRY` | record + `ANSENTRY` | record, no `ANSENTRY` |
| Their `GETABSENCEINFO` | answer | no answer |
| Directed `SENDMSG` from them | normal | still received (use mute to silence alerts) |

Limits that need a human FeiQ check: a client that ignores unicast `BR_EXIT`, or that only times us out on a missed subnet refresh, can keep showing us. A peer we have never seen is not in the unicast list until they announce. Group match is the exact trimmed department string, not a discussion-group id. Hiding does not stop someone who already knows the IP from sending. Mute does not drop the UDP message.
