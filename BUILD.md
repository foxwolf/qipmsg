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

CMake ≥ 3.16, C++17, Qt 6 + Core5Compat. No Qt Sql. Each job uploads a `qipmsg-<os>` artifact.

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

- On first run the app creates `~/.qipmsg/` (Linux/macOS) or `%USERPROFILE%\.qipmsg\` (Windows) for settings (`qipmsg.ini`), the lock file, internal log, **per-peer chat history** in `history/<ip>.jsonl` (discussion transcripts use `history/disc-<id>.jsonl`), last-seen peers in `peers.json`, discussion groups in `discussions.json` (named member keys `user@host/ip`; local multi-send UX, **not** FeiQ multicast group protocol; no Qt Sql), file-resume offsets in `resume.json`, **offline text queue** in `offline_queue.json` (plain JSON, no Qt Sql), default receive files in `recv/`, and screenshots in `screenshots/`.
- Transfer codec defaults to **GBK** (FeiQ / 飞鸽). Change it under Preferences → Detail/Log → Transfer. Decode tries the configured codec, then GBK, GB2312, and UTF-8.
- Presence refresh interval and auto-reply live in Preferences (Detail tab + Send/Receive). Tray **Leave → Away / Auto-reply** toggles absence (`BR_ABSENCE`). Tray **Leave → Stealth / Hide mode** (also Preferences → Send/Receive) skips `ANSENTRY` and own `BR_ENTRY`/`BR_ABSENCE`; turning it on sends `BR_EXIT` first.
- Local memo nicknames are stored in `peers.json` as `"memo"` (same sidecar file, no Qt Sql). The contact tree and chat title show the memo; peer-profile **Nickname** keeps the real name.
- Contact search matches nickname / host / login / memo / IP / group. Pinyin initials use public-domain GB2312 first-letter ranges via `QTextCodec` GBK/GB2312 (no third-party library). Full pinyin is a compact built-in table of surnames and common words in `src/pinyin.cpp` (public readings, GPL-3.0-or-later with the rest of QIpMsg).
- Offline text queue (FeiQ-like 在线自动发送, best-effort): sending to a stale/offline/unknown peer enqueues the body under `offline_queue.json`. Default is auto-queue (Preferences → Send/Receive; uncheck to confirm each time). Flush happens when `UserManager` marks the IP `PRESENCE_ONLINE` or `PRESENCE_AWAY` (typically `BR_ENTRY` / `ANSENTRY` / `BR_ABSENCE`). History is written only on flush. Files and window-shake are not queued — IPMSG has no store-and-forward server.
- File receive options live in Preferences → Detail → File: resume (`GETFILEDATA` hex offset), auto-accept, auto-rename, default folder. Directory transfers restart from the tree root (IPMSG dir stream); only regular files persist an offset.
- UDP listen port and NIC/bind address live in Preferences → Detail → Network (`qipmsg.ini` `[Network]` `udpPort` / `bindAddress`). Default **2425** / any IPv4 (`0.0.0.0`). UDP and TCP file listen share that port. Changes need a restart. Bind failure is logged and the app continues (soft-fail); file receive needs a successful TCP listen.
- FeiQ 2013 / IPMSG file-transfer assumptions (no new protocol):
  - TCP `GETFILEDATA` / `GETDIRFILES` command field is accepted as **decimal or hex** (FeiQ often sends hex `60`/`62`; classic IPMSG and older qipmsg send decimal `96`/`98`). Outgoing requests use hex.
  - File-list and stream sizes/offsets are hex. Qt `QString::arg` width is a minimum, so values above 4 GB are sent in full hex (not truncated to 8 digits).
  - Packet-id lookup tries the raw field, hex→decimal, and decimal string so FeiQ and qipmsg map keys match.
  - Resume offset is hex on the wire; `resume.json` stores size/offset as decimal strings (old double values still load). Directory streams have no per-file resume.
  - TCP connect uses the peer’s last UDP source port (fallback 2425), with an 8 s timeout. Regular-file reads are capped at remaining bytes.
- Personal signature is sent as a third NUL field after `nickname\0group\0` on `BR_ENTRY` / `ANSENTRY` / `BR_ABSENCE`. Classic IPMSG ignores extra fields; FeiQ `PersonSign` uses the same extra slot.
- Window shake is a normal `IPMSG_SENDMSG` whose text is `#@#shake#@#` (also accepted: `{shake}`, `SHAKE`, `窗口抖动`, `#qipmsg-shake#`, and ESC+SHAKE). FeiQ peers that do not implement shake treat it as ordinary text.
- Screenshot uses `QScreen::grabWindow` (full screen or rubber-band region) and sends a PNG through the existing file-attach path. Wayland/headless sessions may return an empty grab.
- Chat bubbles show a thumbnail for `png` / `jpg` / `jpeg` / `gif` / `bmp` / `webp` attachments (outgoing local path, incoming after receive into the save folder, plus path/filename in history). Click opens the file with the desktop handler. Clipboard paste (`Ctrl+V` of an image) saves under `screenshots/` and attaches it — the same IPMSG file-attach path FeiQ uses.
- The toolbar emoji pack inserts Unicode into the compose box and sends it as ordinary `IPMSG_SENDMSG` text (no new protocol, no Qt Sql). FeiQ 2013 typically uses GBK, which cannot encode most emoji; those peers may see `?`. Use Preferences → Transfer → UTF-8 for Unicode emoji interop. Incoming FeiQ-style tokens such as `/微笑` are rendered as emoji locally only.
- UI language defaults to **Simplified Chinese**. `qipmsg_zh_CN.qm` is embedded in `icons.qrc` (`:/translations`) and also copied next to the executable as `translations/qipmsg_zh_CN.qm`. Chinese system locales (`zh_*`) try that locale first, then `zh_CN`. Other locales load `zh_CN` as the product default. Qt `qt`/`qtbase` catalogs are loaded from the same search path plus Qt’s own translations directory when present (so standard OK/Cancel buttons are Chinese when those files exist).
- To refresh the catalog after editing `tr()` strings: `lupdate src -ts src/translations/qipmsg_zh_CN.ts` then `lrelease src/translations/qipmsg_zh_CN.ts -qm src/translations/qipmsg_zh_CN.qm`.

## Version

**2.7.0** is this slice (FeiQ/IPMSG TCP file interop, 64-bit sizes/offsets, configurable UDP/TCP port + NIC) on top of merged 2.6.0 (#28 images/emoji), 2.5.2 (#31 light discussion groups), 2.5.1 (#30 offline queue), and 2.5.0 (#29 memo/pinyin/stealth).

## Trying the 2.7.0 UI

1. Launch `qipmsg` (UI is Simplified Chinese, light messenger theme). The main window shows a rounded-avatar grouped list, signature field, and online count. Last-seen peers appear grey until presence refresh. A **讨论组** folder sits apart from FeiQ department groups.
2. Filter with 搜索好友 / 拼音 (`zs` / `zhang` for 张三). Right-click **设置备注** for a local memo. Multi-select two or more contacts, then 创建讨论组. Reopen the group from the list; the same text is sent to each member. @mention highlight is local only — the wire stays plain text. This is **not** FeiQ multicast chatroom.
3. Chat uses message bubbles (incoming left, outgoing right). Toolbar: 发送文件 / 发送文件夹 / 表情 / 窗口抖动 / 截图 / 查找; remote/voice/share/zone are disabled (暂未实现). Right side is 对方资料 with avatar, status, and signature. Image attachments show a thumbnail in the bubble; click to open. Paste an image with Ctrl+V. History JSONL may include an `img` array.
4. Tray **Find in history** searches every `history/*.jsonl` file.
5. Send a message from another IPMSG/FeiQ client: expect a tray balloon, unread tooltip, and the same chat window to append history.
6. Enable **Away / Auto-reply** from the tray Leave menu and send another message from a peer; you should see one automatic reply per cooldown. Shake packets are not auto-replied.
7. Enable **Stealth / Hide mode**. A second IPMSG/FeiQ client that refreshes after you hide should no longer see you (no `ANSENTRY`). A client that already had your IP can still send a directed message.
8. Send text to a grey/offline peer: it sits in the chat **Offline queue** and tray **Offline message queue**. When the peer comes back (`BR_ENTRY` / `ANSENTRY`), items flush in order. Attachments and shake are not queued.
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

Assumptions: FeiQ HideMode is treated as “do not answer discovery.” We still list others who broadcast. We cannot hide from a specific group. Windows/FeiQ confirmation is still needed.
