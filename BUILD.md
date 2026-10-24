# QIpMsg Build Guide / 构建指南

QIpMsg now uses **CMake** and supports **Qt 6 (preferred) / Qt 5.12+** on Windows, Linux and macOS.

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

## Legacy qmake build (optional)

`src/qipmsg.pro` is still maintained for quick qmake builds:

```bash
cd src
qmake
make
```

---

## Runtime notes

- On first run the app creates `~/.qipmsg/` (Linux/macOS) or `%USERPROFILE%\.qipmsg\` (Windows) for settings (`qipmsg.ini`), the lock file, internal log, **per-peer chat history** in `history/<ip>.jsonl`, last-seen peers in `peers.json`, file-resume offsets in `resume.json` (plain JSON, no Qt Sql), default receive files in `recv/`, and screenshots in `screenshots/`.
- Transfer codec defaults to **GBK** (FeiQ / 飞鸽). Change it under Preferences → Detail/Log → Transfer. Decode tries the configured codec, then GBK, GB2312, and UTF-8.
- Presence refresh interval and auto-reply live in Preferences (Detail tab + Send/Receive). Tray **Leave → Away / Auto-reply** toggles absence (`BR_ABSENCE`).
- File receive options live in Preferences → Detail → File: resume (`GETFILEDATA` hex offset), auto-accept, auto-rename, default folder. Directory transfers restart from the tree root (IPMSG dir stream); only regular files persist an offset.
- Personal signature is sent as a third NUL field after `nickname\0group\0` on `BR_ENTRY` / `ANSENTRY` / `BR_ABSENCE`. Classic IPMSG ignores extra fields; FeiQ `PersonSign` uses the same extra slot.
- Window shake is a normal `IPMSG_SENDMSG` whose text is `#@#shake#@#` (also accepted: `{shake}`, `SHAKE`, `窗口抖动`, `#qipmsg-shake#`, and ESC+SHAKE). FeiQ peers that do not implement shake treat it as ordinary text.
- Screenshot uses `QScreen::grabWindow` (full screen or rubber-band region) and sends a PNG through the existing file-attach path. Wayland/headless sessions may return an empty grab.
- UI language defaults to **Simplified Chinese**. `qipmsg_zh_CN.qm` is embedded in `icons.qrc` (`:/translations`) and also copied next to the executable as `translations/qipmsg_zh_CN.qm`. Chinese system locales (`zh_*`) try that locale first, then `zh_CN`. Other locales load `zh_CN` as the product default. Qt `qt`/`qtbase` catalogs are loaded from the same search path plus Qt’s own translations directory when present (so standard OK/Cancel buttons are Chinese when those files exist).
- To refresh the catalog after editing `tr()` strings: `lupdate src -ts src/translations/qipmsg_zh_CN.ts` then `lrelease src/translations/qipmsg_zh_CN.ts -qm src/translations/qipmsg_zh_CN.qm`.

## Trying the 2.4.0 UI

1. Launch `qipmsg` (UI is Simplified Chinese, light messenger theme). The main window shows a rounded-avatar grouped list, signature field, and online count. Last-seen peers appear grey until presence refresh.
2. Filter with 搜索好友. Double-click a peer to open chat. Right-click can blacklist.
3. Chat uses message bubbles (incoming left, outgoing right). Toolbar: 发送文件 / 发送文件夹 / 窗口抖动 / 截图 / 查找; remote/voice/share/zone are disabled (暂未实现). Right side is 对方资料 with avatar, status, and signature.
4. Tray **Find in history** searches every `history/*.jsonl` file.
5. Send a message from another IPMSG/FeiQ client: expect a tray balloon, unread tooltip, and the same chat window to append history.
6. Enable **Away / Auto-reply** from the tray Leave menu and send another message from a peer; you should see one automatic reply per cooldown. Shake packets are not auto-replied.
