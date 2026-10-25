<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../about_dialog.cpp" line="35"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="36"/>
        <source>Contributors</source>
        <translation>贡献者</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="37"/>
        <source>Translators</source>
        <translation>翻译</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="38"/>
        <source>License</source>
        <translation>协议</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="50"/>
        <source>About QIpMsg</source>
        <translation>关于 QIpMsg</translation>
    </message>
</context>
<context>
    <name>AutoStart</name>
    <message>
        <location filename="../autostart.cpp" line="186"/>
        <source>Off until you enable it. The next login starts QIpMsg from the current-user Run key (QIpMsg). FeiQ&apos;s own startup entry is left alone.</source>
        <translation>默认关闭。下次登录时从当前用户的 Run 启动项（名称 QIpMsg）启动。不会改动飞秋自己的启动项。</translation>
    </message>
    <message>
        <location filename="../autostart.cpp" line="190"/>
        <source>Off until you enable it. A per-user LaunchAgent starts QIpMsg at the next login. It is not loaded into the current session.</source>
        <translation>默认关闭。用户 LaunchAgent 在下次登录时启动，不会在当前这次会话里再拉起一份。</translation>
    </message>
    <message>
        <location filename="../autostart.cpp" line="194"/>
        <source>Off until you enable it. The next graphical login starts QIpMsg from an XDG autostart entry under your config directory.</source>
        <translation>默认关闭。下次图形登录时从配置目录里的 XDG 自启项启动。</translation>
    </message>
    <message>
        <location filename="../autostart.cpp" line="198"/>
        <source>Start with the system is not available on this platform.</source>
        <translation>当前平台不支持开机启动。</translation>
    </message>
</context>
<context>
    <name>AvatarFetch</name>
    <message>
        <location filename="../avatarFetch.cpp" line="62"/>
        <source>The peer has no photo to send.</source>
        <translation>对方没有可发送的头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="65"/>
        <location filename="../avatarFetch.cpp" line="209"/>
        <source>The photo is larger than qipmsg will transfer.</source>
        <translation>这张头像超过 qipmsg 愿意传输的大小。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="68"/>
        <location filename="../avatarFetch.cpp" line="243"/>
        <source>The photo data was not a readable PNG.</source>
        <translation>头像数据不是可读的 PNG。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="71"/>
        <location filename="../avatarFetch.cpp" line="270"/>
        <source>The peer closed the photo connection.</source>
        <translation>对方关闭了头像连接。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="73"/>
        <source>The peer refused the photo (%1).</source>
        <translation>对方拒绝了头像（%1）。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="83"/>
        <source>Using a photo set on this computer. It overrides the peer photo.</source>
        <translation>正在使用本机设置的头像，它优先于对方传来的头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="94"/>
        <location filename="../avatarFetch.cpp" line="152"/>
        <source>Photo hidden on this computer until the peer publishes a different one.</source>
        <translation>本机已隐藏这张头像，直到对方公布另一张。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="99"/>
        <source>FeiQ and classic IPMSG have no photo packet this client can use. The color block stays. You can still set a local photo for this IP.</source>
        <translation>飞秋和经典飞鸽没有本程序能用的头像报文，仍显示色块。你仍可为本机这个 IP 设置头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="106"/>
        <source>This qipmsg peer has not published a photo.</source>
        <translation>这个 qipmsg 对方没有公布头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="111"/>
        <location filename="../avatarFetch.cpp" line="257"/>
        <source>Photo from this qipmsg peer.</source>
        <translation>这是从 qipmsg 对方取回的头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="115"/>
        <location filename="../avatarFetch.cpp" line="170"/>
        <source>Photo %1 is published, but the peer is not reachable.</source>
        <translation>对方公布了头像 %1，但现在联系不上。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="120"/>
        <location filename="../avatarFetch.cpp" line="129"/>
        <source>Fetching photo…</source>
        <translation>正在获取头像…</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="124"/>
        <source>Photo fetch is waiting. Too many peers are queued.</source>
        <translation>头像获取在排队，等待的人太多。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="156"/>
        <source>No saved photo for this IP.</source>
        <translation>这个 IP 没有已保存的头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="221"/>
        <source>The peer sent a photo reply this client could not read.</source>
        <translation>对方的头像回复无法识别。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="230"/>
        <source>The photo did not match the advertised id.</source>
        <translation>头像和对方公布的标识不一致。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="254"/>
        <source>Could not store that photo.</source>
        <translation>无法保存这张头像。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="279"/>
        <source>Could not fetch the photo (%1).</source>
        <translation>无法获取头像（%1）。</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="280"/>
        <source>connection failed</source>
        <translation>连接失败</translation>
    </message>
    <message>
        <location filename="../avatarFetch.cpp" line="288"/>
        <source>Could not fetch the photo (timed out).</source>
        <translation>获取头像超时。</translation>
    </message>
</context>
<context>
    <name>AvatarStore</name>
    <message>
        <location filename="../avatarStore.cpp" line="146"/>
        <location filename="../avatarStore.cpp" line="187"/>
        <location filename="../avatarStore.cpp" line="240"/>
        <location filename="../avatarStore.cpp" line="460"/>
        <location filename="../avatarStore.cpp" line="468"/>
        <location filename="../avatarStore.cpp" line="476"/>
        <source>Could not store that photo.</source>
        <translation>无法保存这张头像。</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="153"/>
        <location filename="../avatarStore.cpp" line="163"/>
        <source>Choose a PNG or JPEG smaller than 20 MB.</source>
        <translation>请选择小于 20 MB 的 PNG 或 JPEG。</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="175"/>
        <source>Could not read that image.</source>
        <translation>无法读取这张图片。</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="199"/>
        <source>Using a photo set on this computer. It overrides the peer photo.</source>
        <translation>正在使用本机设置的头像，它优先于对方传来的头像。</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="230"/>
        <location filename="../avatarStore.cpp" line="238"/>
        <source>Choose photo</source>
        <translation>选择头像</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="232"/>
        <source>Images (*.png *.jpg *.jpeg)</source>
        <translation>图片 (*.png *.jpg *.jpeg)</translation>
    </message>
    <message>
        <location filename="../avatarStore.cpp" line="452"/>
        <source>The photo data was not a readable PNG.</source>
        <translation>头像数据不是可读的 PNG。</translation>
    </message>
</context>
<context>
    <name>ChatWindow</name>
    <message>
        <location filename="../chat_window.cpp" line="190"/>
        <location filename="../chat_window.cpp" line="191"/>
        <location filename="../chat_window.cpp" line="1187"/>
        <location filename="../chat_window.cpp" line="1197"/>
        <location filename="../chat_window.cpp" line="1203"/>
        <source>Screenshot</source>
        <translation>截图</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="193"/>
        <source>Full screen</source>
        <translation>全屏</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="194"/>
        <source>Region</source>
        <translation>框选</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="206"/>
        <source>Find</source>
        <translation>查找</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="252"/>
        <source>&amp;Send</source>
        <translation>发送(&amp;S)</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="144"/>
        <source>Not implemented yet</source>
        <translation>暂未实现</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="177"/>
        <source>Remote assist</source>
        <translation>远程协助</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="178"/>
        <source>Remote control</source>
        <translation>远程控制</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="179"/>
        <source>Voice call</source>
        <translation>语音通话</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="180"/>
        <location filename="../chat_window.cpp" line="181"/>
        <location filename="../chat_window.cpp" line="272"/>
        <source>Send file</source>
        <translation>发送文件</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="182"/>
        <location filename="../chat_window.cpp" line="183"/>
        <source>Send folder</source>
        <translation>发送文件夹</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="198"/>
        <location filename="../chat_window.cpp" line="559"/>
        <source>Doodle</source>
        <translation>涂鸦</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="199"/>
        <source>Draw a picture and send it as an image attachment.</source>
        <translation>画一张图，并按图片附件发送。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="201"/>
        <source>Emoji</source>
        <translation>表情</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="204"/>
        <location filename="../chat_window.cpp" line="205"/>
        <source>Window shake</source>
        <translation>窗口抖动</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="207"/>
        <source>Find in history</source>
        <translation>查找聊天记录</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="209"/>
        <source>Favorites</source>
        <translation>收藏</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="212"/>
        <source>Zone</source>
        <translation>空间</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="273"/>
        <source>Monitor</source>
        <translation>监视</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="274"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="275"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="296"/>
        <source>&amp;Encap</source>
        <translation>封装(&amp;E)</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="297"/>
        <source>Send a sealed envelope (hidden until the peer opens it)</source>
        <translation>发送密封信件（对方拆开前不显示正文）</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="319"/>
        <source>Cancel selected</source>
        <translation>取消选中</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="320"/>
        <source>Clear queue</source>
        <translation>清空队列</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="376"/>
        <source>Nickname:</source>
        <translation>昵称:</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="382"/>
        <source>Peer profile</source>
        <translation>对方资料</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="397"/>
        <source>PNG or JPEG for this IP on this computer. It overrides a photo fetched from another qipmsg. FeiQ still has no photo packet.</source>
        <translation>为本机这个 IP 指定 PNG 或 JPEG。它优先于从 qipmsg 取回的头像。飞秋仍然没有头像报文。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="401"/>
        <source>Remove the photo set on this computer. A fetched qipmsg photo can show again. If only a fetched photo is saved, it stays hidden until that peer publishes a different one.</source>
        <translation>去掉本机设置的头像后，已取回的 qipmsg 头像会再显示。如果只有取回的头像，会一直隐藏，直到对方公布另一张。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="498"/>
        <source>No peer photo yet.</source>
        <translation>还没有对方头像。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="560"/>
        <source>Could not save the doodle.</source>
        <translation>无法保存涂鸦。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="563"/>
        <source>[Doodle] %1</source>
        <translation>[涂鸦] %1</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1061"/>
        <source>Files stay out of the offline queue because there is no transfer server to resume them. Only the text can be queued.</source>
        <translation>文件不会进入离线队列，因为没有传输服务可以续传。只有文字可以排队。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1063"/>
        <source>Files stay out of the offline queue because there is no transfer server to resume them. Nothing was queued.</source>
        <translation>文件不会进入离线队列，因为没有传输服务可以续传。没有内容入队。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="240"/>
        <source>Type a message...</source>
        <translation>输入消息...</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="473"/>
        <source>Offline</source>
        <translation>离线</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="475"/>
        <source>Online</source>
        <translation>在线</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="477"/>
        <source>Away</source>
        <translation>离开</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="486"/>
        <source>No signature</source>
        <translation>暂无签名</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="377"/>
        <source>Host:</source>
        <translation>主机:</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="184"/>
        <source>Browse peer share</source>
        <translation>浏览对方共享</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="185"/>
        <source>List a qipmsg shared folder. FeiQ share has no verified packet format.</source>
        <translation>列出 qipmsg 对方的共享目录。飞秋共享没有已核对的报文。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="187"/>
        <location filename="../chat_window.cpp" line="625"/>
        <source>Group folder</source>
        <translation>讨论组文件夹</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="188"/>
        <source>Request files from a discussion folder this peer published.</source>
        <translation>向对方索取他在讨论组里公布的文件夹。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="202"/>
        <source>Emoji and custom stickers</source>
        <translation>表情与自定义表情</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="210"/>
        <source>Save chat text, a file path, or a note about this peer on this computer.</source>
        <translation>把选中的聊天文字、文件路径或对该好友的备注保存在本机。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="378"/>
        <source>Login:</source>
        <translation>登录名:</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="379"/>
        <source>Group:</source>
        <translation>组名:</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="380"/>
        <source>IP address:</source>
        <translation>IP地址:</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="396"/>
        <source>Local photo...</source>
        <translation>本地头像…</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="400"/>
        <source>Clear local photo</source>
        <translation>清除本地头像</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="455"/>
        <location filename="../chat_window.cpp" line="480"/>
        <source>Typing…</source>
        <translation>正在输入…</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="459"/>
        <source>Key ready</source>
        <translation>公钥已就绪</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="626"/>
        <source>No discussion-folder token from this peer yet. They publish it from their discussion window. FeiQ has no packet for this.</source>
        <translation>还没有收到这位好友公布的讨论组文件夹口令。对方要在讨论组窗口里点「告诉大家」。飞秋没有对应的报文。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="730"/>
        <source>Encrypted</source>
        <translation>已加密</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="878"/>
        <source>Sealed letter — click to open</source>
        <translation>密封信件 — 点击拆开</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="903"/>
        <source>Envelope opened</source>
        <translation>已拆开</translation>
    </message>
    <message numerus="yes">
        <location filename="../chat_window.cpp" line="1030"/>
        <source>Offline queue · %n message(s) — send when the peer comes online</source>
        <translation>
            <numerusform>离线队列 · %n 条，对方上线后自动发送</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1059"/>
        <source>Offline queue</source>
        <translation>离线队列</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1119"/>
        <source>[File attachment] %1</source>
        <translation>[文件附件] %1</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1164"/>
        <source>[Window shake]</source>
        <translation>[窗口抖动]</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1188"/>
        <location filename="../chat_window.cpp" line="1198"/>
        <source>Could not capture the screen.</source>
        <translation>无法截取屏幕。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1204"/>
        <source>Could not save the screenshot.</source>
        <translation>无法保存截图。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1207"/>
        <source>[Screenshot] %1</source>
        <translation>[截图] %1</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1214"/>
        <source>Select files to send</source>
        <translation>选择要发送的文件</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1227"/>
        <source>Select folder to send</source>
        <translation>选择要发送的文件夹</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1253"/>
        <source>Show send file list (%1)</source>
        <translation>显示传送文件列表 (%1)</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1316"/>
        <source> To: </source>
        <translation> 发往: </translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1337"/>
        <source> Time: </source>
        <translation> 时间: </translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="1458"/>
        <source>[Sticker] %1</source>
        <translation>[表情] %1</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="780"/>
        <location filename="../chat_window.cpp" line="819"/>
        <source>Read</source>
        <translation>已读</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="783"/>
        <source>Unread</source>
        <translation>未读</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="213"/>
        <source>Open local notes under the zone folder. This is not a FeiQ zone database.</source>
        <translation>打开本机空间目录里的笔记。这不是飞秋的空间数据库。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="215"/>
        <source>Log</source>
        <translation>日志</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="216"/>
        <source>Open the same local note folder. This is not a FeiQ zone log and not the message log file.</source>
        <translation>打开同一个本机笔记目录。这不是飞秋空间日志，也不是消息日志文件。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="409"/>
        <location filename="../chat_window.cpp" line="579"/>
        <source>Network neighborhood</source>
        <translation>网上邻居</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="506"/>
        <source>Windows network neighborhood is only available on Windows.</source>
        <translation>网上邻居只在 Windows 上可用。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="510"/>
        <source>The address is not a plain IPv4 address.</source>
        <translation>这个地址不是普通的 IPv4 地址。</translation>
    </message>
    <message>
        <location filename="../chat_window.cpp" line="513"/>
        <source>Open \\%1\ in Explorer. Windows asks for a password if one is required. This program does not ask for share credentials.</source>
        <translation>在资源管理器中打开 \\%1\。如果共享需要密码，由 Windows 询问。本程序不收集共享凭据。</translation>
    </message>
</context>
<context>
    <name>ContactDelegate</name>
    <message>
        <location filename="../contactDelegate.cpp" line="113"/>
        <source>D</source>
        <comment>one-letter dial-up badge</comment>
        <translation>拨</translation>
    </message>
    <message>
        <location filename="../contactDelegate.cpp" line="135"/>
        <source>Typing…</source>
        <translation>正在输入…</translation>
    </message>
</context>
<context>
    <name>ContributorTab</name>
    <message>
        <location filename="../about_dialog.cpp" line="118"/>
        <source>Some people have helped with their contributions:</source>
        <translation>贡献者:</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="120"/>
        <source>Original Author, Packager</source>
        <translation>原作者、打包</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="123"/>
        <location filename="../about_dialog.cpp" line="129"/>
        <source>Packager</source>
        <translation>打包</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="126"/>
        <source>Send file bug fix</source>
        <translation>修复发送文件问题</translation>
    </message>
</context>
<context>
    <name>DataBackup</name>
    <message>
        <location filename="../dataBackup.cpp" line="211"/>
        <source>File is too large to back up: %1</source>
        <translation>文件太大，无法备份：%1</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="275"/>
        <location filename="../dataBackup.cpp" line="281"/>
        <location filename="../dataBackup.cpp" line="288"/>
        <location filename="../dataBackup.cpp" line="512"/>
        <location filename="../dataBackup.cpp" line="519"/>
        <location filename="../dataBackup.cpp" line="542"/>
        <location filename="../dataBackup.cpp" line="601"/>
        <source>The archive is truncated.</source>
        <translation>压缩包不完整。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="309"/>
        <location filename="../dataBackup.cpp" line="428"/>
        <source>The data folder does not exist.</source>
        <translation>数据目录不存在。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="328"/>
        <location filename="../dataBackup.cpp" line="369"/>
        <location filename="../dataBackup.cpp" line="390"/>
        <location filename="../dataBackup.cpp" line="406"/>
        <location filename="../dataBackup.cpp" line="643"/>
        <location filename="../dataBackup.cpp" line="649"/>
        <source>Could not write %1.</source>
        <translation>无法写入 %1。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="347"/>
        <location filename="../dataBackup.cpp" line="354"/>
        <location filename="../dataBackup.cpp" line="437"/>
        <source>Could not read %1.</source>
        <translation>无法读取 %1。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="376"/>
        <source>The backup is larger than 4 GB.</source>
        <translation>备份超过 4 GB。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="444"/>
        <location filename="../dataBackup.cpp" line="464"/>
        <location filename="../dataBackup.cpp" line="475"/>
        <location filename="../dataBackup.cpp" line="491"/>
        <source>The archive is not a zip file.</source>
        <translation>这不是 zip 压缩包。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="450"/>
        <source>The archive is larger than 1 GB.</source>
        <translation>压缩包超过 1 GB。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="534"/>
        <source>The archive has too many files.</source>
        <translation>压缩包里的文件太多。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="557"/>
        <source>This zip uses a data descriptor. Export the backup from QIpMsg.</source>
        <translation>这个 zip 使用了数据描述符。请用 QIpMsg 重新导出备份。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="563"/>
        <source>This zip is compressed. Export the backup from QIpMsg.</source>
        <translation>这个 zip 带压缩。请用 QIpMsg 重新导出备份。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="569"/>
        <source>A file in the archive is too large.</source>
        <translation>压缩包里有一个文件太大。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="585"/>
        <location filename="../dataBackup.cpp" line="635"/>
        <source>The archive contains a path that is not private qipmsg data.</source>
        <translation>压缩包里有不属于 qipmsg 私人数据的路径。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="613"/>
        <source>The archive checksum does not match.</source>
        <translation>压缩包校验和不匹配。</translation>
    </message>
    <message>
        <location filename="../dataBackup.cpp" line="625"/>
        <source>The archive has no private qipmsg files.</source>
        <translation>压缩包里没有 qipmsg 私人数据文件。</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="76"/>
        <location filename="../dataBackupUi.cpp" line="90"/>
        <location filename="../dataBackupUi.cpp" line="97"/>
        <source>Backup private data</source>
        <translation>备份私人数据</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="78"/>
        <location filename="../dataBackupUi.cpp" line="114"/>
        <source>Zip archive (*.zip)</source>
        <translation>Zip 压缩包 (*.zip)</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="92"/>
        <source>Could not write the backup.</source>
        <translation>无法写入备份。</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="112"/>
        <location filename="../dataBackupUi.cpp" line="122"/>
        <location filename="../dataBackupUi.cpp" line="141"/>
        <location filename="../dataBackupUi.cpp" line="155"/>
        <location filename="../dataBackupUi.cpp" line="168"/>
        <location filename="../dataBackupUi.cpp" line="182"/>
        <source>Restore private data</source>
        <translation>还原私人数据</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="98"/>
        <source>Saved %1 files to %2.

Included: preferences, peers, history, stickers, group policy, recent contacts, resume data, discussions, the offline queue, RSA keys, local avatars, group logs, schedules, and favorites. Received files and screenshots are not included.</source>
        <translation>已保存 %1 个文件到 %2。

包含：偏好、联系人、历史、表情、组权限、最近联系人、续传、讨论组、离线队列、RSA 密钥、本地头像、讨论组日志、日程和收藏。已接收文件和截图不包含在内。</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="123"/>
        <source>This replaces matching files under %1.

Preferences, peers, history, stickers, group policy, recent contacts, resume data, discussions, the offline queue, RSA keys, local avatars, group logs, schedules, and favorites in the archive overwrite the same paths. Other files, including received files and screenshots, stay. This cannot be undone.

Continue?</source>
        <translation>这将替换 %1 下的对应文件。

压缩包里的偏好、联系人、历史、表情、组权限、最近联系人、续传、讨论组、离线队列、RSA 密钥、本地头像、讨论组日志、日程和收藏会覆盖相同路径。其它文件（包括已接收文件和截图）保持不变。此操作不能撤销。

继续？</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="143"/>
        <source>Could not restore the backup.</source>
        <translation>无法还原备份。</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="156"/>
        <source>Restored %1 files. Restart QIpMsg now so this session does not write older contacts back over the restored files.

Restart now?</source>
        <translation>已还原 %1 个文件。请现在重新启动 QIpMsg，以免本次会话把较旧的联系人写回已还原的文件。

现在重新启动？</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="169"/>
        <source>Could not start a new copy. Quit QIpMsg and open it again. Avoid changing settings before you quit.</source>
        <translation>无法启动新的副本。请退出 QIpMsg 后再打开。退出前不要修改设置。</translation>
    </message>
    <message>
        <location filename="../dataBackupUi.cpp" line="183"/>
        <source>This session is still using contacts and windows from before the restore. Restart when you can. Until then, avoid changing stickers, groups, or the offline queue, or those files may be saved from this session.</source>
        <translation>当前会话仍在使用还原前的联系人和窗口。请尽快重新启动。在此之前不要修改表情、讨论组或离线队列，否则可能用本次会话覆盖已还原的文件。</translation>
    </message>
</context>
<context>
    <name>DetailSetupDialog</name>
    <message>
        <location filename="../setup_window.cpp" line="765"/>
        <source>Detail/Log Settings</source>
        <translation>详细/记录 设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="777"/>
        <source>Detail</source>
        <translation>详细</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="778"/>
        <source>Log</source>
        <translation>日志</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="779"/>
        <source>Other</source>
        <translation>其它</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="780"/>
        <source>Transfer</source>
        <translation>传输</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="781"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="782"/>
        <source>Network</source>
        <translation>网络</translation>
    </message>
</context>
<context>
    <name>DetailTab</name>
    <message>
        <location filename="../setup_window.cpp" line="49"/>
        <source>Minimize message readed alert window</source>
        <translation>自动最小化回执提示窗口</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="53"/>
        <source>Single click to open send window (default double click)</source>
        <translation>单击图标打开发送窗口 (不勾选则为双击)</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="58"/>
        <source>Enable HotKey (Ctrl+Alt+s, R)</source>
        <translation>启用热键 (Ctrl+Alt+S, R)</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="62"/>
        <source>Change [Send/Reply] button name to [Fire/Retrun Fire]</source>
        <translation>将按钮 [发送/回复] 改称为 [开火/还击]</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="67"/>
        <source>No auto close receive message window when reply</source>
        <translation>回复时不自动关闭收到的消息</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="72"/>
        <source>Not popup message when away</source>
        <translation>离开时不自动弹出收到的消息</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="77"/>
        <source>Auto refresh user list (FeiQ AutoRefresh)</source>
        <translation>自动刷新用户列表（飞秋 AutoRefresh）</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="84"/>
        <source> seconds</source>
        <translation> 秒</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="91"/>
        <source>Show tray notification on new message</source>
        <translation>收到新消息时弹出托盘提示</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="105"/>
        <source>Start hidden in the notification area</source>
        <translation>启动时只留在通知区域</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="106"/>
        <source>Leave this off to open the contact window at launch, including when the tray icon is buried in the Windows overflow. Turn it on only if you can already find the icon. Closing the contact window still leaves QIpMsg in the tray when a tray is available.</source>
        <translation>保持不勾选时，启动就会打开联系人窗口，即使托盘图标藏在 Windows 溢出区也能用。只有已经能找到图标时才勾选。关掉联系人窗口后，只要系统有通知区域，QIpMsg 仍会留在托盘里。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="117"/>
        <location filename="../setup_window.cpp" line="229"/>
        <location filename="../setup_window.cpp" line="247"/>
        <source>Import skin</source>
        <translation>导入皮肤</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="119"/>
        <source>From folder...</source>
        <translation>从文件夹…</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="120"/>
        <source>From zip...</source>
        <translation>从 zip…</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="148"/>
        <source>Light keeps the 2.4 messenger theme. Dark uses the same layout. Follow system uses the OS color scheme on Qt 6.5+, otherwise the window color at startup. A skin is a folder or zip under the skins directory with skin.json, optional theme.qss, and an optional background image. A broken QSS falls back to the built-in sheet. Open chats keep the bubbles they already drew.</source>
        <translation>浅色仍是 2.4 的会话主题，暗色沿用同一布局。跟随系统在 Qt 6.5 及以上读取系统配色，更早的 Qt 用启动时窗口的明暗。皮肤是 skins 目录下的文件夹或 zip，内含 skin.json，可选 theme.qss 和背景图。QSS 无效时仍用内置样式。已经打开的会话要重新打开，气泡才按新颜色重画。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="187"/>
        <source>The skin is active, but its QSS was not applied (%1). The built-in light or dark sheet is still in use.</source>
        <translation>皮肤已启用，但它的 QSS 没有套用（%1）。界面仍使用内置的浅色或暗色样式。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="198"/>
        <source>Light</source>
        <translation>浅色</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="199"/>
        <source>Dark</source>
        <translation>暗色</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="200"/>
        <source>Follow system</source>
        <translation>跟随系统</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="206"/>
        <source>Skin: %1</source>
        <translation>皮肤：%1</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="212"/>
        <source>Missing skin: %1</source>
        <translation>找不到皮肤：%1</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="222"/>
        <source>Skin folder</source>
        <translation>皮肤文件夹</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="230"/>
        <source>Could not import that folder (%1).</source>
        <translation>无法导入该文件夹（%1）。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="239"/>
        <source>Skin zip</source>
        <translation>皮肤压缩包</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="240"/>
        <source>Zip files (*.zip)</source>
        <translation>Zip 文件 (*.zip)</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="248"/>
        <source>Could not import that zip (%1).</source>
        <translation>无法导入该压缩包（%1）。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="123"/>
        <source>Refresh interval:</source>
        <translation>刷新间隔：</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="124"/>
        <source>Mark stale after missed refreshes:</source>
        <translation>连续未应答次数后标为过期：</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="125"/>
        <location filename="../setup_window.cpp" line="186"/>
        <source>Appearance</source>
        <translation>外观</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="96"/>
        <source>Start with the system</source>
        <translation>开机时启动</translation>
    </message>
</context>
<context>
    <name>DirDialog</name>
    <message>
        <location filename="../dir_dialog.cpp" line="58"/>
        <source>Select folder to send</source>
        <translation>选择要发送的文件夹</translation>
    </message>
    <message>
        <location filename="../dir_dialog.cpp" line="72"/>
        <source>Browser Folders</source>
        <translation>浏览文件夹</translation>
    </message>
    <message>
        <location filename="../dir_dialog.cpp" line="108"/>
        <source>Ok</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../dir_dialog.cpp" line="109"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>DiscussionWindow</name>
    <message>
        <location filename="../discussion_window.cpp" line="93"/>
        <location filename="../discussion_window.cpp" line="461"/>
        <source>Group notice</source>
        <translation>群公告</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="94"/>
        <source>Set, edit, or clear the notice for this discussion</source>
        <translation>设置、修改或清除这个讨论组的公告</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="122"/>
        <source>Message the discussion…  @name to mention</source>
        <translation>给讨论组发消息…  输入 @姓名 提及</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="126"/>
        <source>[image — send it from a private chat]</source>
        <translation>［图片请在单聊里发送］</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="129"/>
        <source>&amp;Send</source>
        <translation>发送(&amp;S)</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="137"/>
        <source>@ Mention</source>
        <translation>@ 提及</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="160"/>
        <source>Members</source>
        <translation>成员</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="162"/>
        <source>Local multi-send. Not a FeiQ multicast room.</source>
        <translation>本地群发。不是飞秋组播聊天室。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="179"/>
        <location filename="../discussion_window.cpp" line="686"/>
        <location filename="../discussion_window.cpp" line="700"/>
        <location filename="../discussion_window.cpp" line="718"/>
        <location filename="../discussion_window.cpp" line="733"/>
        <location filename="../discussion_window.cpp" line="744"/>
        <source>Group log</source>
        <translation>群日志</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="181"/>
        <source>Local notes for this discussion. Not a FeiQ room.</source>
        <translation>这个讨论组的本地记录。不是飞秋群。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="190"/>
        <source>Short log note</source>
        <translation>写一条简短日志</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="191"/>
        <source>Tell online members</source>
        <translation>告诉在线成员</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="193"/>
        <source>Also send this note as plain text. Offline members are not queued. This is not a FeiQ room protocol.</source>
        <translation>同时把这条记录以纯文本发给在线成员。离线成员不入队。这不是飞秋群协议。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="196"/>
        <source>Add log entry</source>
        <translation>记一笔</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="197"/>
        <source>Export log</source>
        <translation>导出日志</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="209"/>
        <source>No group folder yet.</source>
        <translation>还没有讨论组文件夹。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="214"/>
        <source>Set folder</source>
        <translation>设置文件夹</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="215"/>
        <source>Clear folder</source>
        <translation>清除文件夹</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="216"/>
        <source>Tell the group</source>
        <translation>告诉大家</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="217"/>
        <source>Folder on this computer for this discussion only.</source>
        <translation>只在这台电脑上、并且只给这个讨论组用的文件夹。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="219"/>
        <source>Send the file names and a qipmsg token. Members request files while you are online. This is not a FeiQ room.</source>
        <translation>把文件名和 qipmsg 口令发给成员。你在线时对方才能取文件。这不是飞秋的群。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="253"/>
        <location filename="../discussion_window.cpp" line="549"/>
        <source>Discussion</source>
        <translation>讨论组</translation>
    </message>
    <message numerus="yes">
        <location filename="../discussion_window.cpp" line="255"/>
        <source>%n member(s) · same text is sent to each peer</source>
        <translation>
            <numerusform>%n 位成员 · 同一文本发给每位成员</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="257"/>
        <source>Discussion: %1</source>
        <translation>讨论组: %1</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="264"/>
        <source>Notice: %1</source>
        <translation>公告：%1</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="371"/>
        <source>Encrypted</source>
        <translation>已加密</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="463"/>
        <source>Shown at the top of this discussion. Online members receive a plain text copy. Leave it empty to clear.</source>
        <translation>显示在本讨论组顶部。在线成员会收到一份纯文本。留空即清除。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="469"/>
        <source>Notice</source>
        <translation>公告</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="474"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="497"/>
        <source>[Group notice cleared] %1</source>
        <translation>［群公告已清除］%1</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="498"/>
        <source>[Group notice] %1
%2</source>
        <translation>［群公告］%1
%2</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="550"/>
        <source>Saved on this computer. No online member received it.</source>
        <translation>已保存在本机。没有在线成员收到。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="562"/>
        <source>No group folder yet. Set one on this computer, then tell the group. Members can fetch files only while you are online.</source>
        <translation>还没有讨论组文件夹。先在这台电脑上指定一个，再告诉大家。你在线时成员才能取文件。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="567"/>
        <source>This discussion folder: %1</source>
        <translation>本讨论组文件夹：%1</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="572"/>
        <source>Folder is not available (%1).</source>
        <translation>文件夹目前不可用（%1）。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="576"/>
        <location filename="../discussion_window.cpp" line="651"/>
        <source>(empty)</source>
        <translation>（空）</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="580"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="580"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="584"/>
        <source>List truncated.</source>
        <translation>列表已截断。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="596"/>
        <location filename="../discussion_window.cpp" line="604"/>
        <location filename="../discussion_window.cpp" line="625"/>
        <source>Discussion folder</source>
        <translation>讨论组文件夹</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="605"/>
        <source>That folder could not be saved.</source>
        <translation>这个文件夹没能保存。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="626"/>
        <source>Set a folder on this computer first.</source>
        <translation>请先在这台电脑上指定文件夹。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="639"/>
        <source>Folder is not available right now.</source>
        <translation>文件夹现在不可用。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="648"/>
        <source>...</source>
        <translation>…</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="654"/>
        <source>[Group share] %1
%2
%3
qipmsg peers can request these files while I am online. FeiQ cannot browse this folder.</source>
        <translation>[讨论组共享] %1
%2
%3
qipmsg 在我在线时可以来取这些文件。飞秋不能浏览这个文件夹。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="669"/>
        <source>No log entries yet.</source>
        <translation>还没有日志。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="687"/>
        <location filename="../discussion_window.cpp" line="696"/>
        <source>Write a short note first.</source>
        <translation>先写一条简短记录。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="692"/>
        <source>Could not write the log.</source>
        <translation>无法写入日志。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="694"/>
        <source>This log file is full (2 MB). Export it before adding more.</source>
        <translation>日志文件已满（2 MB）。请先导出，再继续添加。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="698"/>
        <source>This discussion cannot store a log.</source>
        <translation>这个讨论组无法保存日志。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="706"/>
        <source>Saved on this computer.</source>
        <translation>已保存在本机。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="709"/>
        <source>[Group log] %1
%2</source>
        <translation>[群日志] %1
%2</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="719"/>
        <source>There is nothing to export.</source>
        <translation>没有可导出的内容。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="723"/>
        <source>Export group log</source>
        <translation>导出群日志</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="725"/>
        <source>Text (*.txt);;JSON lines (*.jsonl)</source>
        <translation>文本 (*.txt);;JSON 行 (*.jsonl)</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="734"/>
        <source>Could not read the log.</source>
        <translation>无法读取日志。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="745"/>
        <source>Could not write %1.</source>
        <translation>无法写入 %1。</translation>
    </message>
    <message>
        <location filename="../discussion_window.cpp" line="749"/>
        <source>Exported to %1.</source>
        <translation>已导出到 %1。</translation>
    </message>
</context>
<context>
    <name>DoodleCanvas</name>
    <message>
        <location filename="../doodleCanvas.cpp" line="98"/>
        <location filename="../doodleCanvas.cpp" line="206"/>
        <source>Doodle</source>
        <translation>涂鸦</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="103"/>
        <source>Black</source>
        <translation>黑色</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="104"/>
        <source>Red</source>
        <translation>红色</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="105"/>
        <source>Blue</source>
        <translation>蓝色</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="106"/>
        <source>Green</source>
        <translation>绿色</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="107"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="122"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="123"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../doodleCanvas.cpp" line="207"/>
        <source>Draw something first.</source>
        <translation>请先画一点内容。</translation>
    </message>
</context>
<context>
    <name>EmojiPack</name>
    <message>
        <location filename="../emoji_pack.cpp" line="30"/>
        <source>Smile</source>
        <translation>微笑</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="31"/>
        <source>Happy</source>
        <translation>开心</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="32"/>
        <source>Grin</source>
        <translation>偷笑</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="33"/>
        <source>Blush</source>
        <translation>害羞</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="34"/>
        <source>Heart eyes</source>
        <translation>色</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="35"/>
        <source>Smirk</source>
        <translation>得意</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="36"/>
        <source>Tear</source>
        <translation>流泪</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="37"/>
        <source>Sob</source>
        <translation>大哭</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="38"/>
        <source>Sweat smile</source>
        <translation>尴尬</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="39"/>
        <source>Angry</source>
        <translation>发怒</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="40"/>
        <source>Wink</source>
        <translation>调皮</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="41"/>
        <source>Surprised</source>
        <translation>惊讶</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="42"/>
        <source>Sad</source>
        <translation>难过</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="43"/>
        <source>Cool</source>
        <translation>酷</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="44"/>
        <source>Cold sweat</source>
        <translation>冷汗</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="45"/>
        <source>Tired</source>
        <translation>抓狂</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="46"/>
        <source>Sleep</source>
        <translation>睡</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="47"/>
        <source>Zipper</source>
        <translation>闭嘴</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="48"/>
        <source>Thumbs up</source>
        <translation>强</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="49"/>
        <source>Thumbs down</source>
        <translation>弱</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="50"/>
        <source>Clap</source>
        <translation>鼓掌</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="51"/>
        <source>Heart</source>
        <translation>爱心</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="52"/>
        <source>Rose</source>
        <translation>玫瑰</translation>
    </message>
    <message>
        <location filename="../emoji_pack.cpp" line="53"/>
        <source>Party</source>
        <translation>庆祝</translation>
    </message>
</context>
<context>
    <name>FavoriteDialog</name>
    <message>
        <location filename="../favoriteDialog.cpp" line="57"/>
        <location filename="../favoriteDialog.cpp" line="223"/>
        <location filename="../favoriteDialog.cpp" line="249"/>
        <location filename="../favoriteDialog.cpp" line="279"/>
        <location filename="../favoriteDialog.cpp" line="308"/>
        <location filename="../favoriteDialog.cpp" line="318"/>
        <location filename="../favoriteDialog.cpp" line="344"/>
        <source>Favorites</source>
        <translation>收藏</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="62"/>
        <source>Search favorites</source>
        <translation>搜索收藏</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="74"/>
        <source>Save selected text</source>
        <translation>保存选中文字</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="75"/>
        <location filename="../favoriteDialog.cpp" line="237"/>
        <source>Save file path</source>
        <translation>保存文件路径</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="76"/>
        <location filename="../favoriteDialog.cpp" line="264"/>
        <source>Save peer note</source>
        <translation>保存好友备注</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="77"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="78"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="79"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="87"/>
        <source>Saved in favorites.json on this computer. Nothing is synced or sent unless you paste it into a chat yourself.</source>
        <translation>保存在本机的 favorites.json。不会同步，也不会自动发给对方，除非你自己贴进会话。</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="127"/>
        <source>Choose a file on this computer.</source>
        <translation>选择本机上的一个文件。</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="178"/>
        <source>Text</source>
        <translation>文字</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="179"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="180"/>
        <source>Note</source>
        <translation>备注</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="224"/>
        <source>Could not save the selected text.</source>
        <translation>无法保存选中的文字。</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="239"/>
        <source>All files (*)</source>
        <translation>所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="250"/>
        <source>Could not save the file path.</source>
        <translation>无法保存文件路径。</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="265"/>
        <source>Note about %1:</source>
        <translation>关于 %1 的备注：</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="274"/>
        <source>Note about %1</source>
        <translation>关于 %1 的备注</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="280"/>
        <source>Could not save the note.</source>
        <translation>无法保存备注。</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="309"/>
        <source>That path is not on this computer:
%1</source>
        <translation>本机上没有这个路径：
%1</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="321"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="324"/>
        <source>Open chat</source>
        <translation>打开会话</translation>
    </message>
    <message>
        <location filename="../favoriteDialog.cpp" line="345"/>
        <source>Could not delete the favorite.</source>
        <translation>无法删除这条收藏。</translation>
    </message>
</context>
<context>
    <name>FileTab</name>
    <message>
        <location filename="../setup_window.cpp" line="377"/>
        <source>Resume interrupted file transfers</source>
        <translation>断点续传</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="381"/>
        <source>Auto-accept incoming files</source>
        <translation>自动接收文件</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="385"/>
        <source>Auto-rename if the file already exists</source>
        <translation>文件重名时自动改名</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="390"/>
        <source>Receive folder</source>
        <translation>接收目录</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="398"/>
        <source> KB/s</source>
        <translation> KB/秒</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="399"/>
        <source>Unlimited</source>
        <translation>不限</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="408"/>
        <source>Share a folder with qipmsg peers</source>
        <translation>向 qipmsg 对端共享文件夹</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="412"/>
        <location filename="../setup_window.cpp" line="517"/>
        <source>Shared folder</source>
        <translation>共享文件夹</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="418"/>
        <source>Optional password</source>
        <translation>可选密码</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="419"/>
        <source>Only other qipmsg clients can list this folder. The path cannot escape the folder you pick. FeiQ has no verified share packet, so FeiQ peers still cannot browse it. The password is stored in qipmsg.ini like the other settings.</source>
        <translation>只有其他 qipmsg 能列出这个目录，路径不能超出所选文件夹。飞秋没有已核对的共享报文，所以飞秋仍不能浏览。密码和其他设置一样写在 qipmsg.ini 里。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="426"/>
        <source>Backup...</source>
        <translation>备份…</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="427"/>
        <source>Restore...</source>
        <translation>还原…</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="428"/>
        <source>Save preferences, history, stickers, and other private files to a zip.</source>
        <translation>把偏好设置、历史、表情和其它私人文件保存为 zip。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="429"/>
        <source>Replace matching private files from a zip. You will be asked before overwrite.</source>
        <translation>用 zip 中的同名私人文件替换。覆盖前会再次确认。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="442"/>
        <source>Send speed limit</source>
        <translation>发送限速</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="453"/>
        <source>Share password</source>
        <translation>共享密码</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="468"/>
        <source>Outbound TCP file sends use this cap. 0 is unlimited. 1024 KB/s is 1 MB/s. Current: %1.</source>
        <translation>对外 TCP 文件发送使用此上限。0 为不限。1024 KB/秒等于 1 MB/秒。当前：%1。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="487"/>
        <source>Select shared folder</source>
        <translation>选择共享文件夹</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="496"/>
        <source>Select receive folder</source>
        <translation>选择接收目录</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="518"/>
        <source>Pick a folder before turning sharing on.</source>
        <translation>先选择文件夹，再打开共享。</translation>
    </message>
</context>
<context>
    <name>GroupPolicyDialog</name>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="35"/>
        <source>Department group permissions</source>
        <translation>部门组权限</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="39"/>
        <source>These are FeiQ department groups (the group name in entry packets), not local discussion groups. Hide skips discovery answers for that group and, while any group is hidden, announces presence by unicast instead of a subnet broadcast. Mute keeps history and the unread count, but does not raise a window, play a sound, or show a tray balloon. IPMSG has no per-group hide bit, so a peer that already cached you may keep showing you until it accepts a unicast leave.</source>
        <translation>这里是飞秋部门分组（入网包里的组名），不是本地讨论组。对某组隐身时，不回复该组的发现请求；只要还有任意一组处于隐身，就改为向已知好友单播在线，而不再向子网广播。屏蔽消息仍写入历史并保留未读数，但不弹窗、不响铃、不显示托盘气泡。IPMSG 没有按组隐身标志，已经记住你的对端要收到单播离开后才会把你从列表去掉。</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="52"/>
        <source>Group</source>
        <translation>分组</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="53"/>
        <source>Hide from group</source>
        <translation>对该组隐身</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="54"/>
        <source>Mute messages</source>
        <translation>屏蔽消息</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="61"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="63"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="138"/>
        <source>Hide</source>
        <translation>隐身</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="139"/>
        <source>Mute</source>
        <translation>屏蔽</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="144"/>
        <source>Do not announce presence to peers in this group</source>
        <translation>不向该组的好友宣告在线</translation>
    </message>
    <message>
        <location filename="../groupPolicyDialog.cpp" line="145"/>
        <source>Keep history, but do not raise or notify</source>
        <translation>保留历史，但不弹窗、不提醒</translation>
    </message>
</context>
<context>
    <name>HistorySearchWindow</name>
    <message>
        <location filename="../history_search_window.cpp" line="37"/>
        <source>Find in history</source>
        <translation>查找聊天记录</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="51"/>
        <source>Search all local chat history</source>
        <translation>搜索全部本地聊天记录</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="52"/>
        <source>&amp;Find</source>
        <translation>查找(&amp;F)</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="62"/>
        <source>Time</source>
        <translation>时间</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="62"/>
        <source>Peer</source>
        <translation>对象</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="62"/>
        <source>Dir</source>
        <translation>方向</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="62"/>
        <source>Text</source>
        <translation>内容</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="72"/>
        <source>Type a keyword and press Find.</source>
        <translation>输入关键字后按查找。</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="88"/>
        <source>Enter a search keyword.</source>
        <translation>请输入搜索关键字。</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="105"/>
        <source>To</source>
        <translation>发往</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="105"/>
        <source>From</source>
        <translation>来自</translation>
    </message>
    <message>
        <location filename="../history_search_window.cpp" line="113"/>
        <source>Found %1 message(s). Double-click to open chat.</source>
        <translation>找到 %1 条消息。双击打开会话。</translation>
    </message>
</context>
<context>
    <name>InfoTab</name>
    <message>
        <location filename="../about_dialog.cpp" line="74"/>
        <source>Version: %1</source>
        <translation>版本: %1</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="76"/>
        <source>Compiled with Qt %1</source>
        <translation>由 Qt %1 编译</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="77"/>
        <source>Visit our web for updates:</source>
        <translation>请访问我们的网站获取更新:</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="80"/>
        <source>Join qipmsg group to report a bug or request new features:</source>
        <translation>报告 BUG 或提出建议请使用 qipmsg 讨论组:</translation>
    </message>
</context>
<context>
    <name>LicenseTab</name>
    <message>
        <location filename="../about_dialog.cpp" line="162"/>
        <source>QIpMsg is free software: you can redistribute it and/or modifyit under the terms of the GNU General Public License as published bythe Free Software Foundation, either version 3 of the License, or(at your option) any later version.

QIpMsg is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without even the implied warranty ofMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU General Public License for more details.

QIpMsg is distributed in the hope that it will be useful,You should have received a copy of the GNU General Public Licensealong with QIpMsg.  If not, see &lt;http://www.gnu.org/licenses&gt;.</source>
        <translation>QIpMsg is free software: you can redistribute it and/or modifyit under the terms of the GNU General Public License as published bythe Free Software Foundation, either version 3 of the License, or(at your option) any later version.

QIpMsg is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without even the implied warranty ofMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See theGNU General Public License for more details.

QIpMsg is distributed in the hope that it will be useful,You should have received a copy of the GNU General Public Licensealong with QIpMsg.  If not, see &lt;http://www.gnu.org/licenses&gt;.</translation>
    </message>
</context>
<context>
    <name>LogTab</name>
    <message>
        <location filename="../setup_window.cpp" line="257"/>
        <source>Enable log message</source>
        <translation>启用通讯记录</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="262"/>
        <source>Not log lock message before open</source>
        <translation>上锁信件在被打开之前不得记录</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="267"/>
        <source>Log nickname and login name</source>
        <translation>记录用户/登录名</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="271"/>
        <source>Log ip address</source>
        <translation>记录 IP 地址</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="276"/>
        <source>Log File</source>
        <translation>记录文件</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="669"/>
        <source>Select Log File</source>
        <translation>选择记录文件</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="671"/>
        <source>Text Files (*.log);;All Files (*)</source>
        <translation>记录文件 (*.log);;所有文件 (*)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../main_window.cpp" line="222"/>
        <source>&amp;Refresh</source>
        <translation>刷新(&amp;R)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="304"/>
        <source>&amp;Send</source>
        <translation>发送(&amp;S)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="308"/>
        <source>&amp;Encap</source>
        <translation>封装(&amp;E)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="314"/>
        <source>Loc&amp;k</source>
        <translation>上锁(&amp;K)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="362"/>
        <source>Remove from blacklist</source>
        <translation>移出黑名单</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="364"/>
        <location filename="../main_window.cpp" line="423"/>
        <source>Add to blacklist</source>
        <translation>加入黑名单</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="413"/>
        <source>Send folder</source>
        <translation>发送文件夹</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="419"/>
        <source>Search user</source>
        <translation>搜索好友</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="424"/>
        <source>Set memo...</source>
        <translation>设置备注...</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="428"/>
        <source>Photo for the contact list and header. A small PNG is offered to qipmsg peers. FeiQ is not offered a photo packet.</source>
        <translation>用于联系人列表和顶栏。会向 qipmsg 对方提供一张小 PNG。不会向飞秋发送头像报文。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="434"/>
        <source>Show a PNG or JPEG for this IP on this computer. It overrides a photo fetched from qipmsg. FeiQ still has no photo packet.</source>
        <translation>在本机为这个 IP 显示 PNG 或 JPEG。它优先于从 qipmsg 取回的头像。飞秋仍然没有头像报文。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="730"/>
        <source>set level 1 (user %1 / %2)</source>
        <translation>设置 优先级为 1 (用户 %1 / %2)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="732"/>
        <source>set level 2 (user %1 / %2)</source>
        <translation>设置 优先级为 3 (用户 %1 / %2)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="79"/>
        <source>IP Messenger</source>
        <translation>飞鸽传书</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="191"/>
        <source>Enter a personal signature</source>
        <translation>请输入个性签名</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="223"/>
        <source>Refresh contacts</source>
        <translation>刷新联系人</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="754"/>
        <source>%1 · %n online</source>
        <translation>
            <numerusform>%1 · %n 人在线</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1408"/>
        <source>Files stay out of the offline queue because there is no transfer server to resume them. Only the text can be queued.</source>
        <translation>文件不会进入离线队列，因为没有传输服务可以续传。只有文字可以排队。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1410"/>
        <source>Files stay out of the offline queue because there is no transfer server to resume them. Nothing was queued.</source>
        <translation>文件不会进入离线队列，因为没有传输服务可以续传。没有内容入队。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1581"/>
        <source> To: </source>
        <translation> 发往: </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1603"/>
        <source> Time: </source>
        <translation> 时间: </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1606"/>
        <source> (Sealed)</source>
        <translation> (封装)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="402"/>
        <source>Open chat</source>
        <translation>打开会话</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="180"/>
        <source>Click to choose a PNG or JPEG. Other qipmsg peers can fetch a small copy. FeiQ has no photo packet, so those peers still see a color block.</source>
        <translation>点击选择 PNG 或 JPEG。其它 qipmsg 可以取回一张小图。飞秋没有头像报文，那些对方仍看到色块。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="219"/>
        <source>Search friends / pinyin...</source>
        <translation>搜索好友 / 拼音...</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="412"/>
        <source>Send file</source>
        <translation>传送文件</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="420"/>
        <source>Find in history...</source>
        <translation>查找聊天记录...</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="460"/>
        <source>Set level 1 (user %1 / %2)</source>
        <translation>设置 优先级为 2 (用户 %1 / %2)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="464"/>
        <source>Set level 2 (user %1 / %2)</source>
        <translation>设置 优先级为 4 (用户 %1 / %2)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="468"/>
        <source>Set display level</source>
        <translation>设置显示优先级</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="472"/>
        <source>Select group</source>
        <translation>选择工作组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="743"/>
        <source>Stealth</source>
        <translation>隐身</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="746"/>
        <source>Away</source>
        <translation>离开</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="740"/>
        <source>Online</source>
        <translation>在线</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="224"/>
        <source>Schedule</source>
        <translation>日程</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="225"/>
        <source>Local reminders. They fire only while QIpMsg is running.</source>
        <translation>本机提醒。只有 QIpMsg 正在运行时才会响起。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="306"/>
        <location filename="../main_window.cpp" line="1484"/>
        <location filename="../main_window.cpp" line="1490"/>
        <location filename="../main_window.cpp" line="1494"/>
        <source>Broadcast</source>
        <translation>全体广播</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="307"/>
        <source>Send to every online peer with the broadcast flag</source>
        <translation>向每个在线好友发送，并带上广播标志</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="377"/>
        <source>Hide from group &quot;%1&quot;</source>
        <translation>对组「%1」隐身</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="378"/>
        <source>Mute messages from group &quot;%1&quot;</source>
        <translation>屏蔽组「%1」的消息提醒</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="391"/>
        <source>Send to &quot;%1&quot; (%n online/away)</source>
        <translation>
            <numerusform>发给「%1」（%n 人在线或离开）</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="405"/>
        <source>Broadcast to all online</source>
        <translation>向所有在线好友广播</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="408"/>
        <source>Create discussion group</source>
        <translation>创建讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="409"/>
        <location filename="../main_window.cpp" line="1317"/>
        <source>Rename discussion group</source>
        <translation>重命名讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="410"/>
        <location filename="../main_window.cpp" line="1337"/>
        <source>Delete discussion group</source>
        <translation>删除讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="411"/>
        <source>Add selected contacts to discussion</source>
        <translation>将所选联系人加入讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="427"/>
        <source>Set my photo...</source>
        <translation>设置我的头像…</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="433"/>
        <source>Set local photo...</source>
        <translation>设置本地头像…</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="439"/>
        <source>Clear local photo</source>
        <translation>清除本地头像</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="442"/>
        <source>Hide from this group</source>
        <translation>对该组隐身</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="446"/>
        <source>Mute messages from this group</source>
        <translation>屏蔽该组消息</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="450"/>
        <source>Group permissions...</source>
        <translation>组权限...</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="453"/>
        <source>Send to this group (online and away)</source>
        <translation>发给本组（在线与离开）</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="454"/>
        <source>Multicast the current text to online and away members of this department. Offline peers are not queued.</source>
        <translation>把当前内容发给该部门里在线和离开的成员。灰色离线不入队。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="519"/>
        <source>No department groups</source>
        <translation>没有部门组</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="526"/>
        <source>%1 (%n online/away)</source>
        <translation>
            <numerusform>%1（%n 人在线或离开）</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="528"/>
        <source>Select online and away members. Grey offline peers stay out.</source>
        <translation>选中在线和离开的成员。灰色离线不选。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="592"/>
        <location filename="../main_window.cpp" line="670"/>
        <source>No online or away members in &quot;%1&quot;.</source>
        <translation>「%1」里没有在线或离开的成员。</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="650"/>
        <source>Group send: &quot;%1&quot; · %n online or away member(s). Offline peers are not included. Press Send.</source>
        <translation>
            <numerusform>按组发送：「%1」· %n 人在线或离开。不含离线。请按发送。</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="667"/>
        <location filename="../main_window.cpp" line="681"/>
        <source>Group send</source>
        <translation>按组发送</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="668"/>
        <source>No online or away members in &quot;%1&quot;. Offline peers are not queued.</source>
        <translation>「%1」里没有在线或离开的成员。离线不会入队。</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="682"/>
        <source>Send the current message to %n member(s) of &quot;%1&quot;?</source>
        <translation>
            <numerusform>把当前消息发给「%1」的 %n 位成员？</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="751"/>
        <source>%1, hidden from some groups</source>
        <translation>%1，已对部分组隐身</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="757"/>
        <source> · %n offline queued</source>
        <translation>
            <numerusform> · 离线队列 %n 条</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="861"/>
        <location filename="../main_window.cpp" line="892"/>
        <location filename="../main_window.cpp" line="987"/>
        <source>Dial-up</source>
        <translation>拨号</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="890"/>
        <location filename="../main_window.cpp" line="906"/>
        <source>This is you</source>
        <translation>这是你自己</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="905"/>
        <source>This is you
Sharing %1</source>
        <translation>这是你
正在共享 %1</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="909"/>
        <source>No photo is published to qipmsg peers.</source>
        <translation>没有向 qipmsg 对方公布头像。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="912"/>
        <source>A small photo is published to qipmsg peers. FeiQ cannot fetch it.</source>
        <translation>已向 qipmsg 对方公布一张小头像。飞秋无法获取。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="916"/>
        <source>Dial-up host. Presence packets set IPMSG_DIALUPOPT.</source>
        <translation>拨号主机。在线包会带上 IPMSG_DIALUPOPT。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="942"/>
        <source>%1
%2
Local multi-send (not a FeiQ multicast room)</source>
        <translation>%1
%2
本地群发（不是飞秋组播聊天室）</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="969"/>
        <source>(no text)</source>
        <translation>（无文字）</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="996"/>
        <source>Click to open chat
%1</source>
        <translation>单击打开会话
%1</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="998"/>
        <location filename="../main_window.cpp" line="1050"/>
        <source>Recent contacts</source>
        <translation>最近联系人</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1053"/>
        <location filename="../main_window.cpp" line="1093"/>
        <source>My Friends</source>
        <translation>我的好友</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1054"/>
        <location filename="../main_window.cpp" line="1091"/>
        <source>Ungrouped friends</source>
        <translation>对方未分组好友</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1059"/>
        <location filename="../main_window.cpp" line="1094"/>
        <source>Discussions</source>
        <translation>讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1060"/>
        <location filename="../main_window.cpp" line="1095"/>
        <source>Myself</source>
        <translation>自己</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1061"/>
        <location filename="../main_window.cpp" line="1096"/>
        <source>Blacklist</source>
        <translation>黑名单</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1108"/>
        <source>hidden</source>
        <translation>隐身</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1111"/>
        <source>muted</source>
        <translation>屏蔽</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1280"/>
        <location filename="../main_window.cpp" line="1298"/>
        <source>Discussion group</source>
        <translation>讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1281"/>
        <source>Select at least two contacts to create a discussion group.</source>
        <translation>请至少选择两名联系人以创建讨论组。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1286"/>
        <source>New discussion group</source>
        <translation>新建讨论组</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1286"/>
        <location filename="../main_window.cpp" line="1317"/>
        <source>Name:</source>
        <translation>名称:</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1288"/>
        <source>Discussion (%1)</source>
        <translation>讨论组（%1）</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1299"/>
        <source>Could not create the discussion group.</source>
        <translation>无法创建讨论组。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1335"/>
        <source>Delete this discussion group? Local history stays on disk.</source>
        <translation>删除此讨论组？本地历史仍保留在磁盘上。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1336"/>
        <source>Delete the selected discussion groups? Local history stays on disk.</source>
        <translation>删除所选讨论组？本地历史仍保留在磁盘上。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1375"/>
        <source>Set memo</source>
        <translation>设置备注</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1376"/>
        <source>Local memo for %1:</source>
        <translation>为 %1 设置本地备注:</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1406"/>
        <location filename="../main_window.cpp" line="1471"/>
        <source>Offline queue</source>
        <translation>离线队列</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="1472"/>
        <source>%n message(s) queued and will send when the peer comes online.</source>
        <translation>
            <numerusform>已排队 %n 条，对方上线后自动发送。</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1485"/>
        <source>Type a message, then broadcast it to every online peer.</source>
        <translation>先输入内容，再向每个在线好友广播。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1491"/>
        <source>No online peers to broadcast to.</source>
        <translation>没有在线好友可广播。</translation>
    </message>
    <message numerus="yes">
        <location filename="../main_window.cpp" line="1495"/>
        <source>Broadcast sent to %n online peer(s).</source>
        <translation>
            <numerusform>已向 %n 位在线好友广播。</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1510"/>
        <location filename="../main_window.cpp" line="1544"/>
        <source>Show send file list (%1)</source>
        <translation>显示传送文件列表 (%1)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1676"/>
        <source>Add Files</source>
        <translation>添加文件</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="1678"/>
        <source>All Files (*)</source>
        <translation>所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="414"/>
        <location filename="../main_window.cpp" line="488"/>
        <source>Open network neighborhood</source>
        <translation>打开网上邻居</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="415"/>
        <source>Open \\IP\ in Explorer on Windows. Windows asks for a password if one is required.</source>
        <translation>在 Windows 资源管理器中打开 \\IP\。如果共享需要密码，由 Windows 询问。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="482"/>
        <source>Open network neighborhood (Windows only)</source>
        <translation>打开网上邻居（仅 Windows）</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="484"/>
        <source>Windows network neighborhood is only available on Windows.</source>
        <translation>网上邻居只在 Windows 上可用。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="492"/>
        <source>Open \\%1\ in Explorer. Windows asks for a password if one is required. This program does not ask for share credentials.</source>
        <translation>在资源管理器中打开 \\%1\。如果共享需要密码，由 Windows 询问。本程序不收集共享凭据。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="496"/>
        <source>Select one peer with an IPv4 address.</source>
        <translation>请选择一个有 IPv4 地址的好友。</translation>
    </message>
    <message>
        <location filename="../main_window.cpp" line="510"/>
        <source>Network neighborhood</source>
        <translation>网上邻居</translation>
    </message>
</context>
<context>
    <name>MsgReadedWindow</name>
    <message>
        <location filename="../msg_readed_window.cpp" line="53"/>
        <source>Envelope Opened</source>
        <translation>信封已经被打开</translation>
    </message>
    <message>
        <location filename="../msg_readed_window.cpp" line="64"/>
        <source>Ok</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../msg_readed_window.cpp" line="33"/>
        <source>Envelope Opened: </source>
        <translation>信封已打开: </translation>
    </message>
</context>
<context>
    <name>MsgServer</name>
    <message>
        <location filename="../msg_server.cpp" line="296"/>
        <source>I am away from my computer.</source>
        <translation>我现在不在电脑旁。</translation>
    </message>
    <message>
        <location filename="../msg_server.cpp" line="382"/>
        <source>Encrypted message (cannot decrypt). FeiQ and official IPMsg may pack RSA differently from qipmsg.</source>
        <translation>无法解密的加密消息。飞秋和官方 IPMsg 的 RSA 封装可能与 qipmsg 不同。</translation>
    </message>
</context>
<context>
    <name>MsgWindow</name>
    <message>
        <location filename="../msg_window.cpp" line="84"/>
        <source>&amp;Open Envelope</source>
        <translation>打开信封(&amp;O)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="130"/>
        <source>receive message from ...</source>
        <translation>消息来自...</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="138"/>
        <source>%1
%2</source>
        <translation>%1
%2</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="184"/>
        <source>&amp;Close</source>
        <translation>关闭(&amp;C)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="188"/>
        <source>&amp;Reply</source>
        <translation>回复(&amp;R)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="192"/>
        <source>&amp;Quote Message</source>
        <translation>引用原文(&amp;Q)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="98"/>
        <source>Receiving Message</source>
        <translation>收到消息</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="134"/>
        <source>Time: </source>
        <translation>时间: </translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="373"/>
        <source> From: </source>
        <translation> 来自: </translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="392"/>
        <source> Time: </source>
        <translation> 时间: </translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="397"/>
        <source> (Sealed)</source>
        <translation> (封装)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="56"/>
        <location filename="../msg_window.cpp" line="425"/>
        <source>Show receive file list (%1)</source>
        <translation>显示接收文件列表 (%1)</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="457"/>
        <location filename="../msg_window.cpp" line="664"/>
        <source>Preparing to get file... </source>
        <translation>准备传送文件...</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="558"/>
        <source>[Received image] %1</source>
        <translation>[已接收图片] %1</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="602"/>
        <source>Alert</source>
        <translation>提示</translation>
    </message>
    <message>
        <location filename="../msg_window.cpp" line="603"/>
        <source>Stop transfer?</source>
        <translation>是否要终止传送？</translation>
    </message>
</context>
<context>
    <name>NetworkTab</name>
    <message>
        <location filename="../setup_window.cpp" line="545"/>
        <source>UDP listen port</source>
        <translation>UDP 监听端口</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="546"/>
        <source>Bind interface</source>
        <translation>绑定网卡</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="547"/>
        <source>Changes take effect after restart. UDP discovery and TCP file transfer use the same port (IPMSG / FeiQ, default 2425). Any IPv4 keeps discovery on every NIC. Choosing a NIC stores its MAC as the interface id (or the interface name when no MAC is reported) and listens on that NIC&apos;s current IPv4. %1 If the port cannot be opened at startup, QIpMsg explains why and can save a free port for this run. Port 2425 is kept whenever it is free. A chosen NIC that is down does not fall back to the other NICs.</source>
        <translation>修改后需要重新启动。UDP 发现和 TCP 文件传输使用同一个端口（IPMSG / 飞秋，默认 2425）。选「任意 IPv4」时，每块网卡都会参与发现。选定一块网卡后，会把它的 MAC 存成接口标识（没有 MAC 时改存接口名），并监听这块网卡当前的 IPv4。%1 启动时如果端口打不开，会说明原因，并可以把一个空闲端口写入配置后立即使用。2425 空闲时仍使用它。选定的网卡如果不在线，不会改用其它网卡。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="577"/>
        <source>no MAC</source>
        <translation>无 MAC</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="619"/>
        <source>Saved interface %1</source>
        <translation>已保存的网卡 %1</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="573"/>
        <source>Any IPv4 (0.0.0.0)</source>
        <translation>任意 IPv4（0.0.0.0）</translation>
    </message>
</context>
<context>
    <name>OfflineQueue</name>
    <message>
        <location filename="../offline_queue.cpp" line="64"/>
        <source>[Window shake]</source>
        <translation>[窗口抖动]</translation>
    </message>
</context>
<context>
    <name>OfflineQueueDialog</name>
    <message>
        <location filename="../offline_queue.cpp" line="350"/>
        <source>Offline message queue</source>
        <translation>离线消息队列</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="357"/>
        <source>Peer</source>
        <translation>对端</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="357"/>
        <source>Time</source>
        <translation>时间</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="357"/>
        <source>Message</source>
        <translation>消息</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="363"/>
        <source>Delete selected</source>
        <translation>删除选中</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="377"/>
        <source>Queued text and window-shake tokens are sent in order when the peer answers discovery. File attachments are not queued (no server).</source>
        <translation>排队的文字和窗口抖动会在对方应答发现后按顺序发送。文件附件不入队（没有服务器）。</translation>
    </message>
    <message>
        <location filename="../offline_queue.cpp" line="397"/>
        <source>Offline message queue (%1)</source>
        <translation>离线消息队列（%1）</translation>
    </message>
    <message numerus="yes">
        <location filename="../offline_queue.cpp" line="399"/>
        <source>%n offline queued</source>
        <translation>
            <numerusform>离线队列 %n 条</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>OtherTab</name>
    <message>
        <location filename="../setup_window.cpp" line="694"/>
        <source>Quote Mark</source>
        <translation>引文标识</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="701"/>
        <source>Notice Sound</source>
        <translation>提示声音</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="707"/>
        <source>Online Icon</source>
        <translation>在线图标</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="712"/>
        <source>Away Icon</source>
        <translation>离线图标</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="740"/>
        <source>Select Sound File</source>
        <translation>选择声音文件</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="742"/>
        <source>Audio Files (*.wav)</source>
        <translation>声音文件 (*.wav)</translation>
    </message>
</context>
<context>
    <name>PortConflict</name>
    <message>
        <location filename="../portConflict.cpp" line="60"/>
        <source>%1: listening</source>
        <translation>%1：正在监听</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="65"/>
        <source>already in use</source>
        <translation>已被占用</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="69"/>
        <source>blocked by a Windows excluded port range</source>
        <translation>落在 Windows 排除端口范围内</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="70"/>
        <source>access denied</source>
        <translation>系统拒绝访问</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="73"/>
        <source>address is not available</source>
        <translation>地址不可用</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="77"/>
        <source>could not bind</source>
        <translation>无法绑定</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="81"/>
        <source>%1: %2</source>
        <translation>%1：%2</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="83"/>
        <source>%1: %2 (%3)</source>
        <translation>%1：%2（%3）</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="90"/>
        <source>Windows refused this port because it is inside an excluded range. Hyper-V, NAT, and some VPN clients reserve blocks that include 2425. An administrator can list them with: netsh interface ipv4 show excludedportrange protocol=tcp</source>
        <translation>Windows 拒绝了这个端口，因为它落在排除范围内。Hyper-V、NAT 和部分 VPN 会划出包含 2425 的端口段。管理员可以用下面的命令查看：netsh interface ipv4 show excludedportrange protocol=tcp</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="99"/>
        <source>The system refused this port (access denied). On Windows that often means an excluded port range rather than FeiQ itself. An administrator can check: netsh interface ipv4 show excludedportrange protocol=tcp</source>
        <translation>系统拒绝了这个端口（访问被拒绝）。在 Windows 上这通常是排除端口范围，而不一定是飞秋本身占用。管理员可以查看：netsh interface ipv4 show excludedportrange protocol=tcp</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="108"/>
        <source>Another program already has this port. That is often FeiQ, IP Messenger, or a second copy of qipmsg. Quit that program to stay on 2425, or accept a different port below.</source>
        <translation>已有程序占用这个端口，常见是飞秋、飞鸽传书或另一份 qipmsg。退出那个程序就可以继续用 2425，也可以接受下面的其它端口。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="116"/>
        <source>The selected network address is not available. Preferences → Detail → Network can switch back to Any IPv4.</source>
        <translation>选定的网络地址不可用。可以在偏好设置 → 详细 → 网络里改回「任意 IPv4」。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="120"/>
        <source>The port could not be opened. Preferences → Detail → Network can change it. A restart is required after editing it there.</source>
        <translation>端口无法打开。可以在偏好设置 → 详细 → 网络里修改。在那里改完需要重新启动。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="144"/>
        <source>Port 2425 is free</source>
        <translation>2425 端口已经空闲</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="145"/>
        <source>QIpMsg is using port %1 because an earlier start could not open 2425. Port 2425 (IPMSG / FeiQ) is free now. Switch back so other clients can find you on the usual port?

Closing this keeps port %1. You can also change it later in Preferences → Detail → Network.</source>
        <translation>QIpMsg 正在使用端口 %1，因为之前有一次启动打不开 2425。现在 2425（IPMSG / 飞秋）已经空闲。要改回去，方便其它客户端在常用端口上找到你吗？

关闭这个对话框会继续使用端口 %1。以后也可以在偏好设置 → 详细 → 网络里修改。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="153"/>
        <source>Switch back to 2425</source>
        <translation>改回 2425</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="155"/>
        <source>Keep port %1</source>
        <translation>继续使用端口 %1</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="173"/>
        <location filename="../portConflict.cpp" line="264"/>
        <source>Listen port unavailable</source>
        <translation>监听端口不可用</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="178"/>
        <source>A free port is available: %1. It will be saved as udpPort and used for both UDP discovery and TCP file transfer.</source>
        <translation>有一个空闲端口：%1。它会写入 udpPort，UDP 发现和 TCP 文件传输都用它。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="183"/>
        <source>No free alternate port was found. The contact window stays open. Discovery or file transfer stays off until a port can be opened.</source>
        <translation>没有找到空闲的备用端口。联系人窗口会保持打开。在有端口可以监听之前，发现或文件传输不会工作。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="188"/>
        <source>QIpMsg could not listen on port %1 (%2).

%3
%4

%5

%6

Port 2425 is the IPMSG / FeiQ default. qipmsg keeps it whenever it is free, and changes it only if you accept. Peers that still use 2425 will not see a client that moved. If this choice moved you off 2425, a later start can offer to switch back once 2425 is free. Closing this dialog keeps the current port and leaves the contact window open.</source>
        <translation>QIpMsg 无法在端口 %1（%2）上监听。

%3
%4

%5

%6

2425 是 IPMSG / 飞秋的默认端口。只要它空闲，qipmsg 就继续用它，只有你同意才会改。仍使用 2425 的对端看不到已经改端口的客户端。如果这次离开了 2425，以后 2425 再次空闲时，启动时会询问是否改回。关闭这个对话框会保留当前端口，联系人窗口仍然开着。</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="201"/>
        <source>UDP</source>
        <translation>UDP</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="202"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="209"/>
        <source>Use port %1</source>
        <translation>改用端口 %1</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="214"/>
        <location filename="../portConflict.cpp" line="271"/>
        <location filename="../portConflict.cpp" line="295"/>
        <source>Continue anyway</source>
        <translation>仍然继续</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="265"/>
        <source>Port %1 also failed to open. The contact window stays open, but discovery or file transfer may not work.

UDP: %2
TCP: %3</source>
        <translation>端口 %1 同样打不开。联系人窗口会保持打开，但发现或文件传输可能无法工作。

UDP：%2
TCP：%3</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="269"/>
        <location filename="../portConflict.cpp" line="270"/>
        <source>failed</source>
        <translation>失败</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="288"/>
        <source>Start QIpMsg</source>
        <translation>启动 QIpMsg</translation>
    </message>
    <message>
        <location filename="../portConflict.cpp" line="289"/>
        <source>UDP discovery or the TCP file service is not listening on port %1. The contact window stays open. Change the port in Preferences → Detail → Network if this keeps happening.

%2</source>
        <translation>UDP 发现或 TCP 文件服务没有在端口 %1 上监听。联系人窗口会保持打开。如果一直这样，请到偏好设置 → 详细 → 网络里修改端口。

%2</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../about_dialog.cpp" line="198"/>
        <source>&lt;b&gt;%1&lt;/b&gt;: %2</source>
        <translation>&lt;b&gt;%1&lt;/b&gt;：%2</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="203"/>
        <source>&lt;b&gt;%1&lt;/b&gt; (%2)</source>
        <translation>&lt;b&gt;%1&lt;/b&gt;（%2）</translation>
    </message>
    <message>
        <location filename="../global.cpp" line="183"/>
        <location filename="../helper.cpp" line="242"/>
        <source>%1 file</source>
        <translation>%1 文件</translation>
    </message>
    <message>
        <location filename="../global.cpp" line="185"/>
        <location filename="../helper.cpp" line="244"/>
        <source>%1 files</source>
        <translation>%1 文件</translation>
    </message>
    <message>
        <location filename="../helper.cpp" line="140"/>
        <source>Unknown hostname</source>
        <translation>未知主机名</translation>
    </message>
    <message>
        <location filename="../helper.cpp" line="352"/>
        <source>%1 second</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="../helper.cpp" line="354"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="../recv_file_map.cpp" line="37"/>
        <source>current: %1</source>
        <translation>当前：%1</translation>
    </message>
    <message>
        <location filename="../recv_file_map.cpp" line="44"/>
        <source>%1 folder(s)</source>
        <translation>%1 个文件夹</translation>
    </message>
    <message>
        <location filename="../recv_file_map.cpp" line="47"/>
        <source>Total:</source>
        <translation>总计:</translation>
    </message>
    <message>
        <location filename="../recv_file_model.cpp" line="27"/>
        <source>fileId</source>
        <translation>文件编号</translation>
    </message>
    <message>
        <location filename="../recv_file_model.cpp" line="27"/>
        <source>Name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="../recv_file_model.cpp" line="28"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../recv_file.cpp" line="61"/>
        <location filename="../recv_file.cpp" line="83"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="44"/>
        <source>The share listing is too large.</source>
        <translation>共享列表太大。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="72"/>
        <source>Wrong share password.</source>
        <translation>共享密码不正确。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="75"/>
        <source>That peer is not sharing a folder.</source>
        <translation>对方没有打开共享文件夹。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="78"/>
        <source>That shared path is gone.</source>
        <translation>该共享路径已经不存在。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="81"/>
        <source>That path is outside the shared folder.</source>
        <translation>该路径超出了共享文件夹。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="84"/>
        <source>The peer closed the share connection.</source>
        <translation>对方关闭了共享连接。</translation>
    </message>
</context>
<context>
    <name>RecvFileFinishDialog</name>
    <message>
        <location filename="../recv_file_finish_dialog.cpp" line="50"/>
        <source>Congratulation! Get File Successfully!</source>
        <translation>恭喜恭喜!文件传送成功!</translation>
    </message>
    <message>
        <location filename="../recv_file_finish_dialog.cpp" line="69"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../recv_file_finish_dialog.cpp" line="70"/>
        <source>Open File</source>
        <translation>打开文件</translation>
    </message>
    <message>
        <location filename="../recv_file_finish_dialog.cpp" line="71"/>
        <source>Open Folder</source>
        <translation>打开所在的文件夹</translation>
    </message>
</context>
<context>
    <name>RecvFileTransfer</name>
    <message>
        <location filename="../recv_file_transfer.cpp" line="694"/>
        <location filename="../recv_file_transfer.cpp" line="702"/>
        <source>Transfer cancelled</source>
        <translation>传送已取消</translation>
    </message>
</context>
<context>
    <name>RecvFileWindow</name>
    <message>
        <location filename="../recv_file_window.cpp" line="63"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="64"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="65"/>
        <source>Select All</source>
        <translation>选择所有</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="66"/>
        <source>Clear Select</source>
        <translation>清空选择</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="97"/>
        <source>Select Files First</source>
        <translation>选择文件</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="98"/>
        <source>Please select files you want to save first.</source>
        <translation>请首先选择你要保存的文件。</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="113"/>
        <source>Select Save Folder</source>
        <translation>选择存放目录</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="115"/>
        <source>Alert</source>
        <translation>提示</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="119"/>
        <source>File exists, overwrite?</source>
        <translation>文件已经存在!是否覆盖?</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="123"/>
        <source>Folder exists, select other place to save.</source>
        <translation>目录已经存在，请选择其它位置存放。</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="157"/>
        <source>Save File</source>
        <translation>保存文件</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="197"/>
        <source>normal-length-of-a-file-name.file</source>
        <translation>普通长度的文件名.file</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="199"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../recv_file_window.cpp" line="47"/>
        <source>Receive File List</source>
        <translation>接收文件列表</translation>
    </message>
</context>
<context>
    <name>RetryRecvFileDialog</name>
    <message>
        <location filename="../retry_recv_file_dialog.cpp" line="41"/>
        <source>Sorry! Get File Failed!</source>
        <translation>对不起!文件传送失败!</translation>
    </message>
    <message>
        <location filename="../retry_recv_file_dialog.cpp" line="60"/>
        <source>Retry</source>
        <translation>重试</translation>
    </message>
    <message>
        <location filename="../retry_recv_file_dialog.cpp" line="61"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>RichComposerBar</name>
    <message>
        <location filename="../richText.cpp" line="558"/>
        <source>Bold</source>
        <translation>粗体</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="567"/>
        <source>Italic</source>
        <translation>斜体</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="576"/>
        <source>Underline</source>
        <translation>下划线</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="582"/>
        <source>Size</source>
        <translation>字号</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="585"/>
        <source>Font size</source>
        <translation>字号</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="587"/>
        <source>Small</source>
        <translation>小</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="588"/>
        <source>Normal</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="589"/>
        <source>Large</source>
        <translation>大</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="593"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="596"/>
        <source>Font color</source>
        <translation>字体颜色</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="599"/>
        <source>Red</source>
        <translation>红</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="599"/>
        <source>Blue</source>
        <translation>蓝</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="599"/>
        <source>Green</source>
        <translation>绿</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="600"/>
        <source>Orange</source>
        <translation>橙</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="600"/>
        <source>Purple</source>
        <translation>紫</translation>
    </message>
    <message>
        <location filename="../richText.cpp" line="606"/>
        <source>Default color</source>
        <translation>默认颜色</translation>
    </message>
</context>
<context>
    <name>ScheduleDialog</name>
    <message>
        <location filename="../scheduleDialog.cpp" line="56"/>
        <location filename="../scheduleDialog.cpp" line="245"/>
        <location filename="../scheduleDialog.cpp" line="252"/>
        <location filename="../scheduleDialog.cpp" line="270"/>
        <location filename="../scheduleDialog.cpp" line="284"/>
        <location filename="../scheduleDialog.cpp" line="297"/>
        <source>Schedule</source>
        <translation>日程</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="66"/>
        <source>Reminder text</source>
        <translation>提醒内容</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="73"/>
        <source>Notify peer with an ordinary message</source>
        <translation>用普通消息通知对方</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="74"/>
        <source>When this reminder is due, send the text as a normal SENDMSG. There is no FeiQ clock packet.</source>
        <translation>到时间后把文字按普通 SENDMSG 发出。没有飞秋时钟报文。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="79"/>
        <source>Saved in schedule.json on this computer. A reminder rings only while QIpMsg is running. If the time passes while the program is quit, the next launch shows it once. There is no background service, and nothing is uploaded.</source>
        <translation>保存在本机的 schedule.json。只有 QIpMsg 正在运行时才会响起。程序退出期间错过的时间，下次启动会补响一次。没有后台服务，也不会上传。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="86"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="87"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="88"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="89"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="96"/>
        <source>When:</source>
        <translation>时间：</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="128"/>
        <source>(no peer)</source>
        <translation>（不指定好友）</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="181"/>
        <source>Done</source>
        <translation>已完成</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="246"/>
        <source>Enter the reminder text.</source>
        <translation>请填写提醒内容。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="253"/>
        <source>Choose a peer, or turn off notify peer.</source>
        <translation>请选择好友，或关掉通知对方。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="271"/>
        <source>Could not save the reminder. The list may be full.</source>
        <translation>无法保存提醒。列表可能已满。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="285"/>
        <source>Could not update the reminder.</source>
        <translation>无法更新提醒。</translation>
    </message>
    <message>
        <location filename="../scheduleDialog.cpp" line="298"/>
        <source>Could not delete the reminder.</source>
        <translation>无法删除提醒。</translation>
    </message>
</context>
<context>
    <name>ScreenCapture</name>
    <message>
        <location filename="../screen_capture.cpp" line="121"/>
        <source>Snap is off. Release Alt to snap again. Enter/double-click captures, Esc cancels.</source>
        <translation>吸附已关闭。松开 Alt（macOS 为 Option）后恢复。回车或双击完成，Esc 取消。</translation>
    </message>
    <message>
        <location filename="../screen_capture.cpp" line="123"/>
        <source>Drag to select. Snaps to nearby windows. Hold Alt to turn snap off. Enter/double-click captures, Esc cancels.</source>
        <translation>拖动选择。选区会贴近视窗。按住 Alt（macOS 为 Option）暂时关闭吸附。回车或双击完成，Esc 取消。</translation>
    </message>
    <message>
        <location filename="../screen_capture.cpp" line="129"/>
        <source>Drag to select a region. It snaps to nearby windows. Hold Alt to turn snap off. Esc cancels.</source>
        <translation>拖动选择区域，选区会贴近视窗。按住 Alt（macOS 为 Option）暂时关闭吸附。Esc 取消。</translation>
    </message>
</context>
<context>
    <name>SearchUserDialog</name>
    <message>
        <location filename="../search_user_dialog.cpp" line="65"/>
        <source>Search all columns</source>
        <translation>搜索所有项目</translation>
    </message>
    <message>
        <location filename="../search_user_dialog.cpp" line="70"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../search_user_dialog.cpp" line="71"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>SendFileMap</name>
    <message>
        <location filename="../send_file_map.cpp" line="85"/>
        <source>Cancelling…</source>
        <translation>正在取消…</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="88"/>
        <location filename="../send_file_map.cpp" line="99"/>
        <location filename="../send_file_map.cpp" line="109"/>
        <source>Estimating…</source>
        <translation>估算中…</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="88"/>
        <source>Waiting</source>
        <translation>等待</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="96"/>
        <source>/s</source>
        <translation>/秒</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="100"/>
        <source>%1 · remaining unknown</source>
        <translation>%1 · 剩余未知</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="107"/>
        <source>Done</source>
        <translation>完成</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="110"/>
        <source>%1 · estimating</source>
        <translation>%1 · 估算中</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="114"/>
        <source>%1 s left</source>
        <translation>剩余 %1 秒</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="116"/>
        <source>%1 min %2 s left</source>
        <translation>剩余 %1 分 %2 秒</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="118"/>
        <source>%1 h %2 min left</source>
        <translation>剩余 %1 小时 %2 分</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="236"/>
        <source>Folders</source>
        <translation>目录</translation>
    </message>
    <message>
        <location filename="../send_file_map.cpp" line="236"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
</context>
<context>
    <name>SendFileModel</name>
    <message>
        <location filename="../send_file_model.cpp" line="27"/>
        <source>Name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="../send_file_model.cpp" line="27"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../send_file_model.cpp" line="28"/>
        <source>Location</source>
        <translation>位置</translation>
    </message>
    <message>
        <location filename="../send_file_model.cpp" line="28"/>
        <source>Absolute Path</source>
        <translation>绝对路径</translation>
    </message>
    <message>
        <location filename="../send_file_model.cpp" line="129"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
</context>
<context>
    <name>SendFileWindow</name>
    <message>
        <location filename="../send_file_window.cpp" line="85"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="83"/>
        <source>normal-length-of-a-file-name</source>
        <translation>普通长度的文件名</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="88"/>
        <source>/normal/path/of/a/directory/</source>
        <translation>/普通/目录/路径/</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="93"/>
        <source>Add File</source>
        <translation>添加文件</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="94"/>
        <source>Add Directory</source>
        <translation>添加文件夹</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="95"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="96"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="132"/>
        <source>Add Files</source>
        <translation>添加文件</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="134"/>
        <source>All Files (*)</source>
        <translation>所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../send_file_window.cpp" line="44"/>
        <source>Send File List</source>
        <translation>传送文件列表</translation>
    </message>
</context>
<context>
    <name>SendThrottle</name>
    <message>
        <location filename="../sendThrottle.cpp" line="83"/>
        <source>Unlimited</source>
        <translation>不限</translation>
    </message>
    <message>
        <location filename="../sendThrottle.cpp" line="86"/>
        <source>%1 MB/s</source>
        <translation>%1 MB/秒</translation>
    </message>
    <message>
        <location filename="../sendThrottle.cpp" line="90"/>
        <source>%1 KB/s (%2 MB/s)</source>
        <translation>%1 KB/秒（%2 MB/秒）</translation>
    </message>
    <message>
        <location filename="../sendThrottle.cpp" line="94"/>
        <source>%1 KB/s</source>
        <translation>%1 KB/秒</translation>
    </message>
</context>
<context>
    <name>SetupWindow</name>
    <message>
        <location filename="../setup_window.cpp" line="1113"/>
        <source>Click right button on send
window to set display level</source>
        <translation>要设置优先级请
在发送窗口上点右键</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="854"/>
        <source>Setup Preferences</source>
        <translation>服务设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="874"/>
        <source>Personal signature</source>
        <translation>个性签名</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="909"/>
        <source>Send/Receive Settings</source>
        <translation>发送/接收 选项</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="911"/>
        <source>Envelope open check</source>
        <translation>检查信封是否被打开</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="919"/>
        <source>No sound alarm</source>
        <translation>无声音提示</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="937"/>
        <source>Show my typing status</source>
        <translation>显示我的输入状态</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="938"/>
        <source>Sends FeiQ typing commands 121 and 122 (no extra option bits, version stays 1). Classic IPMSG peers ignore them. FeiQ builds that only accept a 1_lbt version string may not show the hint. Incoming 121/122 are still displayed.</source>
        <translation>发送飞秋正在输入命令 121 和 122（不加选项位，版本号仍是 1）。经典 IPMSG 会忽略。只认 1_lbt 版本串的飞秋可能不显示提示。对方发来的 121/122 仍会显示。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="946"/>
        <source>Send sealed message by default</source>
        <translation>默认为封装发送</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="950"/>
        <source>Quote message by default</source>
        <translation>默认为回复时引用原文</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="954"/>
        <source>Queue messages when peer is offline</source>
        <translation>对方离线时自动排队发送</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="958"/>
        <source>Encrypt messages when the peer has a public key</source>
        <translation>对方有公钥时加密消息</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="959"/>
        <source>Off by default. Uses RSA with Blowfish-128 or AES-256 (IPMSG GETPUBKEY / ANSPUBKEY, zero IV, PKCS#1 and PKCS#7). The private key stays in rsa_private.json. Attachments stay plaintext. Presence advertises ENCRYPTOPT only while this is on. FeiQ and official IPMsg may not read this packing; plaintext is used when there is no parsed key.</source>
        <translation>默认关闭。使用 RSA，以及 Blowfish-128 或 AES-256（IPMSG 的 GETPUBKEY / ANSPUBKEY，IV 为 0，PKCS#1 与 PKCS#7）。私钥保存在 rsa_private.json。附件仍是明文。只有打开此项时，在线包才带 ENCRYPTOPT。飞秋和官方 IPMsg 可能读不懂这种封装；没有解析到公钥时仍发明文。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="970"/>
        <source>Away</source>
        <translation>离开</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="974"/>
        <source>Stealth / Hide mode (do not answer discovery)</source>
        <translation>隐身 / 隐藏模式（不回应发现）</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="978"/>
        <source>Group permissions...</source>
        <translation>组权限...</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="979"/>
        <source>Hide from a department group, or mute message alerts from it.</source>
        <translation>对某个部门组隐身，或屏蔽该组的消息提醒。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="983"/>
        <source>Enable auto-reply</source>
        <translation>启用自动回复</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="988"/>
        <source>I am away from my computer.</source>
        <translation>我现在不在电脑旁。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="992"/>
        <source> seconds</source>
        <translation> 秒</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="996"/>
        <source>Reply text:</source>
        <translation>回复内容：</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="999"/>
        <source>Reply cooldown:</source>
        <translation>回复冷却：</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1025"/>
        <source>None Local Broadcast Address Settings</source>
        <translation>局域网外广播地址</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1035"/>
        <source>Add Broadcast Address</source>
        <translation>添加要广播的IP地址</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1037"/>
        <source>Broadcast IP, start-end, or CIDR</source>
        <translation>广播地址、起止范围或 CIDR</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1068"/>
        <source>Cross-subnet peers: add a start-end range or CIDR. Discovery unicasts BR_ENTRY to each address (no server). One entry is limited to %1 hosts and is saved with the broadcast list.</source>
        <translation>跨网段：填写起止 IP 或 CIDR。发现时向每个地址单播入网包（无服务器）。每一项最多探测 %1 个地址，并与广播列表一起保存。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1076"/>
        <source>Start IP</source>
        <translation>起始 IP</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1078"/>
        <source>End IP</source>
        <translation>结束 IP</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1080"/>
        <source>CIDR, e.g. 192.168.2.0/24</source>
        <translation>CIDR，例如 192.168.2.0/24</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1081"/>
        <source>Add range</source>
        <translation>添加网段</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1099"/>
        <source>Ok</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1100"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1110"/>
        <source>Other Settings</source>
        <translation>其它设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1183"/>
        <source>Encrypt</source>
        <translation>加密</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1184"/>
        <source>Could not create an RSA key under the qipmsg data folder. Messages stay plaintext.</source>
        <translation>无法在 qipmsg 数据目录创建 RSA 密钥。消息保持明文。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1235"/>
        <source>Enter an IPv4 address, a start-end range, or a CIDR prefix.</source>
        <translation>请输入 IPv4 地址、起止范围或 CIDR。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1237"/>
        <source>CIDR prefix must be between 0 and 32.</source>
        <translation>CIDR 前缀必须在 0 到 32 之间。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1239"/>
        <source>That range has more than %1 addresses. Split it into smaller ranges.</source>
        <translation>该范围超过 %1 个地址，请拆成更小的网段。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1242"/>
        <source>Loopback and unspecified addresses are not probed.</source>
        <translation>不探测回环地址和未指定地址。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1244"/>
        <source>That is not a valid IPv4 address or range.</source>
        <translation>不是有效的 IPv4 地址或范围。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1253"/>
        <location filename="../setup_window.cpp" line="1282"/>
        <location filename="../setup_window.cpp" line="1288"/>
        <source>Broadcast address</source>
        <translation>广播地址</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1283"/>
        <source>Enter both a start and an end address, or a CIDR prefix.</source>
        <translation>请同时填写起始和结束地址，或填写 CIDR。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="868"/>
        <source>Nickname</source>
        <translation>用户名</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="886"/>
        <source>Group</source>
        <translation>工作组</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1038"/>
        <source>I am a dial-up host</source>
        <translation>我是拨号主机</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1040"/>
        <source>Sets IPMSG_DIALUPOPT on presence packets. Peers that follow the IPMSG spec send member recognition to this host one by one, because a dial-up link may miss subnet broadcasts. Ordinary LAN peers are left unchanged. This is not a FeiQ private extension.</source>
        <translation>在线包带上 IPMSG_DIALUPOPT。按 IPMSG 说明，对方会逐个把成员识别发给本机，因为拨号链路可能收不到子网广播。普通局域网好友不受影响。这不是飞秋私有扩展。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1116"/>
        <source>Detail/Log settings</source>
        <translation>详细/记录 设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1117"/>
        <source>URL link settings</source>
        <translation>URL 链接设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="1119"/>
        <source>Password settings</source>
        <translation>密码设置</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="915"/>
        <source>Not popup message</source>
        <translation>不自动弹出收到的消息</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="923"/>
        <source>Sound when peers come online or go offline</source>
        <translation>好友上线或下线时播放声音</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="924"/>
        <source>Off by default. After startup, the first few seconds stay quiet. The same peer will not beep again for 20 seconds, and bursts share one beep.</source>
        <translation>默认关闭。启动后前几秒保持安静。同一好友 20 秒内不会再次出声，短时间一批上下线只响一声。</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="930"/>
        <source>Do not disturb (don&apos;t pop message windows)</source>
        <translation>勿扰（不弹出消息窗口）</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="931"/>
        <source>Independent of Away / auto-reply. Hides tray balloons and does not raise chat windows. History and the unread count still update.</source>
        <translation>与离开/自动回复无关。不弹托盘气泡，也不把会话窗口抬到前面。历史和未读数仍会更新。</translation>
    </message>
</context>
<context>
    <name>ShareBrowser</name>
    <message>
        <location filename="../shareBrowser.cpp" line="110"/>
        <location filename="../shareBrowser.cpp" line="339"/>
        <source>Browse peer share</source>
        <translation>浏览对方共享</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="110"/>
        <source>Discussion folder</source>
        <translation>讨论组文件夹</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="118"/>
        <source>Share password, if the peer set one</source>
        <translation>如果对方设置了共享密码，填在这里</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="120"/>
        <source>Up</source>
        <translation>上级</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="121"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="122"/>
        <source>Download</source>
        <translation>下载</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="129"/>
        <source>Name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="129"/>
        <source>Kind</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="129"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="143"/>
        <source>Password</source>
        <translation>密码</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="155"/>
        <source>Group folder uses membership, not a password</source>
        <translation>讨论组文件夹按成员身份访问，不使用密码</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="164"/>
        <source>This peer is not a reachable qipmsg. FeiQ cannot serve a discussion folder. The owner must be online with this version.</source>
        <translation>对方不是在线的 qipmsg。飞秋不能提供讨论组文件夹。对方必须使用这一版本并保持在线。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="168"/>
        <source>This peer is not advertising a qipmsg shared folder. FeiQ&apos;s share protocol is not in the IPMSG spec and has no verified packet format here, so a FeiQ share cannot be listed. Turn on sharing in qipmsg Preferences, Detail, File.</source>
        <translation>对方没有宣告 qipmsg 共享文件夹。飞秋共享协议不在 IPMSG 说明里，这里也没有核对过的报文，因此不能列出飞秋共享。请在 qipmsg 的偏好设置、详细、文件里打开共享。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="177"/>
        <source>Listing this discussion folder. Files stay under that folder only. FeiQ peers still have no share browser.</source>
        <translation>正在列出这个讨论组文件夹。文件不会超出该目录。飞秋仍然不能浏览。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="182"/>
        <source>This share asks for a password.</source>
        <translation>这个共享需要密码。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="184"/>
        <source>Listing a qipmsg shared folder. FeiQ peers still have no share browser.</source>
        <translation>正在列出 qipmsg 共享目录。飞秋对端仍然不能这样浏览。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="299"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="299"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="306"/>
        <source>Showing the first %1 entries.</source>
        <translation>只显示前 %1 项。</translation>
    </message>
    <message numerus="yes">
        <location filename="../shareBrowser.cpp" line="307"/>
        <source>%n item(s)</source>
        <translation>
            <numerusform>%n 项</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="347"/>
        <source>Saved %1</source>
        <translation>已保存 %1</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="399"/>
        <location filename="../shareBrowser.cpp" line="410"/>
        <location filename="../shareBrowser.cpp" line="422"/>
        <location filename="../shareBrowser.cpp" line="430"/>
        <source>Unexpected share reply.</source>
        <translation>共享回复无法识别。</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="443"/>
        <source>Downloading…</source>
        <translation>正在下载…</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="443"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="450"/>
        <source>Cancelled</source>
        <translation>已取消</translation>
    </message>
    <message>
        <location filename="../shareBrowser.cpp" line="490"/>
        <source>The download ended early.</source>
        <translation>下载提前结束。</translation>
    </message>
</context>
<context>
    <name>ShotCanvas</name>
    <message>
        <location filename="../shotEditor.cpp" line="189"/>
        <source>Text</source>
        <translation>文字</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="189"/>
        <source>Annotation text:</source>
        <translation>标注文字：</translation>
    </message>
</context>
<context>
    <name>ShotEditor</name>
    <message>
        <location filename="../shotEditor.cpp" line="261"/>
        <source>Annotate screenshot</source>
        <translation>标注截图</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="266"/>
        <source>Pen</source>
        <translation>画笔</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="267"/>
        <source>Rectangle</source>
        <translation>矩形</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="268"/>
        <source>Arrow</source>
        <translation>箭头</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="269"/>
        <source>Text</source>
        <translation>文字</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="287"/>
        <source>Red</source>
        <translation>红</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="288"/>
        <source>Orange</source>
        <translation>橙</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="289"/>
        <source>Yellow</source>
        <translation>黄</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="290"/>
        <source>Green</source>
        <translation>绿</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="291"/>
        <source>Blue</source>
        <translation>蓝</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="292"/>
        <source>White</source>
        <translation>白</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="293"/>
        <source>Black</source>
        <translation>黑</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="295"/>
        <source>Thin</source>
        <translation>细</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="296"/>
        <source>Medium</source>
        <translation>中</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="297"/>
        <source>Thick</source>
        <translation>粗</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="300"/>
        <source>Undo</source>
        <translation>撤销</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="317"/>
        <source>Draw on the capture, then Send. Esc cancels. The picture is still sent as a PNG attachment.</source>
        <translation>在截图上涂画，然后发送。Esc 取消。图片仍按 PNG 附件发出。</translation>
    </message>
    <message>
        <location filename="../shotEditor.cpp" line="323"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
</context>
<context>
    <name>SizeColumnDelegate</name>
    <message>
        <location filename="../sizecolumndelegate.cpp" line="38"/>
        <location filename="../sizecolumndelegate.cpp" line="39"/>
        <source>Folder</source>
        <translation>文件夹</translation>
    </message>
    <message>
        <location filename="../sizecolumndelegate.cpp" line="43"/>
        <source>%1 KB</source>
        <translation>%1 KB</translation>
    </message>
</context>
<context>
    <name>StickerPicker</name>
    <message>
        <location filename="../stickerPicker.cpp" line="50"/>
        <source>Emoji</source>
        <translation>表情</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="70"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="71"/>
        <source>Copy PNG, GIF, WebP, or JPEG into your sticker folder</source>
        <translation>把 PNG、GIF、WebP 或 JPEG 复制到本机表情目录</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="72"/>
        <source>New group</source>
        <translation>新建分组</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="73"/>
        <source>Rename</source>
        <translation>重命名</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="74"/>
        <source>Delete group</source>
        <translation>删除分组</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="110"/>
        <source>Classic</source>
        <translation>经典</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="186"/>
        <source>Classic emoji are inserted as text. Custom stickers are sent as image files.</source>
        <translation>经典表情以文字插入。自定义表情按图片文件发送。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="202"/>
        <source>%1
Right-click to delete or move</source>
        <translation>%1
右键可删除或移动</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="212"/>
        <source>GIF</source>
        <translation>GIF</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="225"/>
        <source>Delete sticker</source>
        <translation>删除表情</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="226"/>
        <source>Move to</source>
        <translation>移动到</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="250"/>
        <source>No stickers in this group. Import a PNG, GIF, or WebP.</source>
        <translation>这个分组还没有表情。可导入 PNG、GIF 或 WebP。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="252"/>
        <source>Click a sticker to send it as a file attachment.</source>
        <translation>点击表情，将以文件附件发送。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="262"/>
        <source>Import stickers</source>
        <translation>导入表情</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="263"/>
        <source>Images (*.png *.gif *.webp *.jpg *.jpeg *.bmp)</source>
        <translation>图片 (*.png *.gif *.webp *.jpg *.jpeg *.bmp)</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="274"/>
        <source>Imported %1, skipped %2.</source>
        <translation>已导入 %1 个，跳过 %2 个。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="287"/>
        <source>New sticker group</source>
        <translation>新建表情分组</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="287"/>
        <location filename="../stickerPicker.cpp" line="309"/>
        <source>Group name:</source>
        <translation>分组名称：</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="294"/>
        <location filename="../stickerPicker.cpp" line="315"/>
        <location filename="../stickerPicker.cpp" line="329"/>
        <location filename="../stickerPicker.cpp" line="335"/>
        <source>Emoji groups</source>
        <translation>表情分组</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="295"/>
        <source>Could not add that group. The name may be empty, too long, or the list is full.</source>
        <translation>无法添加该分组。名称可能为空、过长，或分组数量已满。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="309"/>
        <source>Rename sticker group</source>
        <translation>重命名表情分组</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="316"/>
        <source>Could not rename. The name may be empty or already used.</source>
        <translation>无法重命名。名称可能为空或已被使用。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="330"/>
        <source>Delete group &quot;%1&quot;? Stickers move to another group when one exists.</source>
        <translation>删除分组「%1」？若还有其他分组，表情会移过去。</translation>
    </message>
    <message>
        <location filename="../stickerPicker.cpp" line="336"/>
        <source>Delete the stickers in the last group before removing it.</source>
        <translation>请先删除最后一个分组里的表情，再删除分组。</translation>
    </message>
</context>
<context>
    <name>StickerStore</name>
    <message>
        <location filename="../stickerStore.cpp" line="343"/>
        <source>My stickers</source>
        <translation>我的表情</translation>
    </message>
</context>
<context>
    <name>Systray</name>
    <message>
        <location filename="../systray.cpp" line="201"/>
        <source>Delete all acknowledge windows</source>
        <translation>删除所有回执窗口</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="202"/>
        <source>Top all windows</source>
        <translation>将窗口置于最前端</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="206"/>
        <source>Read message log file</source>
        <translation>查看通信记录</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="207"/>
        <source>Find in history...</source>
        <translation>查找聊天记录...</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="227"/>
        <source>Quit IP Messenger</source>
        <translation>退出飞鸽传书</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="258"/>
        <source>Leave</source>
        <translation>离开</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="268"/>
        <source>Away / Auto-reply</source>
        <translation>离开 / 自动回复</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="203"/>
        <source>Setup preferences...</source>
        <translation>服务设置...</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="99"/>
        <source>IP Messenger (%1 users, %2 unread)</source>
        <translation>飞鸽传书（%1 位用户，%2 条未读）</translation>
    </message>
    <message numerus="yes">
        <location filename="../systray.cpp" line="105"/>
        <source>%n queued</source>
        <translation>
            <numerusform>%n 条排队</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="108"/>
        <source>Stealth</source>
        <translation>隐身</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="110"/>
        <source>Hidden from some groups</source>
        <translation>已对部分组隐身</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="138"/>
        <source>New message from %1</source>
        <translation>来自 %1 的新消息</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="195"/>
        <source>Open main window</source>
        <translation>打开主窗口</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="196"/>
        <source>Show the contact window. Use this if the tray icon is hidden in the notification overflow.</source>
        <translation>显示联系人窗口。托盘图标藏在通知区域的溢出区时用这一项。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="204"/>
        <source>About...</source>
        <translation>关于...</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="205"/>
        <source>About Qt</source>
        <translation>关于 Qt</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="208"/>
        <source>Offline message queue</source>
        <translation>离线消息队列</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="209"/>
        <source>Broadcast to all online...</source>
        <translation>向所有在线好友广播...</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="210"/>
        <source>Backup private data...</source>
        <translation>备份私人数据…</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="211"/>
        <source>Restore private data...</source>
        <translation>还原私人数据…</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="212"/>
        <source>Schedule...</source>
        <translation>日程安排...</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="213"/>
        <source>Local reminders. They fire only while QIpMsg is running.</source>
        <translation>本机提醒。只有 QIpMsg 正在运行时才会响起。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="215"/>
        <source>Zip preferences, history, stickers, and other private files. Received files and screenshots are not included.</source>
        <translation>把偏好设置、历史、表情和其它私人文件打成 zip。已接收的文件和截图不包含在内。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="218"/>
        <source>Replace matching private files from a zip. You will be asked before anything is overwritten.</source>
        <translation>用 zip 中的同名私人文件替换。覆盖前会再次确认。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="275"/>
        <source>Stealth / Hide mode</source>
        <translation>隐身 / 隐藏模式</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="308"/>
        <source>QIpMsg is running</source>
        <translation>QIpMsg 正在运行</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="309"/>
        <source>The contact window is hidden. Choose Open main window on this icon if you do not see it.</source>
        <translation>联系人窗口已隐藏。如果看不到它，请在这个图标的菜单里选「打开主窗口」。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="356"/>
        <source>port %1 is not listening</source>
        <translation>端口 %1 未在监听</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="490"/>
        <source>Read Message Log</source>
        <translation>查看通信记录</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="491"/>
        <source>No message log file specified.</source>
        <translation>没有指定通信记录文件。</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="502"/>
        <location filename="../systray.cpp" line="510"/>
        <location filename="../systray.cpp" line="514"/>
        <source>Broadcast</source>
        <translation>全体广播</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="503"/>
        <source>Message sent to every online peer:</source>
        <translation>发给每个在线好友的内容：</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="511"/>
        <source>No online peers to broadcast to.</source>
        <translation>没有在线好友可广播。</translation>
    </message>
    <message numerus="yes">
        <location filename="../systray.cpp" line="515"/>
        <source>Broadcast sent to %n online peer(s).</source>
        <translation>
            <numerusform>已向 %n 位在线好友广播。</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="539"/>
        <source>Reminder</source>
        <translation>提醒</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="555"/>
        <source>Reminder: %1</source>
        <translation>提醒：%1</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="570"/>
        <source>Offline message queue (%1)</source>
        <translation>离线消息队列（%1）</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="102"/>
        <source>IP Messenger (%1)</source>
        <translation>飞鸽传书 (%1)</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="200"/>
        <location filename="../systray.cpp" line="613"/>
        <source>Show file transfer monitor (%1)</source>
        <translation>文件传送监视器 (%1)</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="113"/>
        <source>Do not disturb</source>
        <translation>勿扰</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="221"/>
        <source>Do not disturb (don&apos;t pop message windows)</source>
        <translation>勿扰（不弹出消息窗口）</translation>
    </message>
    <message>
        <location filename="../systray.cpp" line="224"/>
        <source>Suppress tray balloons and auto-raised chat windows. Not the same as Away / auto-reply.</source>
        <translation>不弹托盘气泡，也不自动升起会话窗口。这不是离开/自动回复。</translation>
    </message>
</context>
<context>
    <name>TransferFileModel</name>
    <message>
        <location filename="../transfer_file_model.cpp" line="42"/>
        <source>No.</source>
        <translation>序号</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="42"/>
        <source>file</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="42"/>
        <source>size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="43"/>
        <source>Total/Sended/Send</source>
        <translation>总数/完成/传送</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="43"/>
        <source>User</source>
        <translation>用户</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="43"/>
        <source>Packet ID</source>
        <translation>数据包号</translation>
    </message>
    <message>
        <location filename="../transfer_file_model.cpp" line="44"/>
        <source>Remaining</source>
        <translation>剩余时间</translation>
    </message>
</context>
<context>
    <name>TransferFileWindow</name>
    <message>
        <location filename="../transfer_file_window.cpp" line="42"/>
        <source>Send limit</source>
        <translation>发送限速</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="86"/>
        <source>00000</source>
        <translation>00000</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="88"/>
        <source>length-of-a-file-name.txt</source>
        <translation>普通长度的文件名.txt</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="90"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="97"/>
        <source>default-user-name</source>
        <translation>默认用户名</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="99"/>
        <source>12.4MB/s · 00 min 00 s left</source>
        <translation>12.4MB/秒 · 剩余 00 分 00 秒</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="109"/>
        <source> KB/s</source>
        <translation> KB/秒</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="110"/>
        <source>Unlimited</source>
        <translation>不限</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="116"/>
        <source>Cancel transfer</source>
        <translation>取消传送</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="117"/>
        <source>Stop the selected transfer. An active TCP send is closed so the peer stops reading. A transfer that has not started is removed.</source>
        <translation>停止所选传送。正在进行的 TCP 发送会被关掉，对方无法继续读。尚未开始的传送会直接移除。</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="151"/>
        <source>Outbound TCP: %1. 1024 KB/s = 1 MB/s.</source>
        <translation>对外 TCP：%1。1024 KB/秒 = 1 MB/秒。</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="120"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="121"/>
        <source>Stop transfer if file changed</source>
        <translation>若目标文件已经改变，则不允许传送</translation>
    </message>
    <message>
        <location filename="../transfer_file_window.cpp" line="50"/>
        <source>Send File Monitor</source>
        <translation>文件传送监视器</translation>
    </message>
</context>
<context>
    <name>TransferTab</name>
    <message>
        <location filename="../setup_window.cpp" line="307"/>
        <source>Transfer Codec:</source>
        <translation>传输编码:</translation>
    </message>
    <message>
        <location filename="../setup_window.cpp" line="308"/>
        <source>GBK/GB2312 is recommended for FeiQ / 飞鸽 peers. UTF-8 is optional for modern clients.</source>
        <translation>与飞秋 / 飞鸽互通时建议使用 GBK/GB2312。现代客户端可选 UTF-8。</translation>
    </message>
</context>
<context>
    <name>TranslatorTab</name>
    <message>
        <location filename="../about_dialog.cpp" line="189"/>
        <source>The following people have contributed with translations:</source>
        <translation>以下对翻译作出贡献:</translation>
    </message>
    <message>
        <location filename="../about_dialog.cpp" line="191"/>
        <source>Simplified-Chinese</source>
        <translation>简体中文</translation>
    </message>
</context>
<context>
    <name>UserManager</name>
    <message>
        <location filename="../user_manager.cpp" line="95"/>
        <source>user</source>
        <translation>用户名</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="95"/>
        <source>group</source>
        <translation>工作组</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="95"/>
        <source>host</source>
        <translation>主机名</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="95"/>
        <source>ip</source>
        <translation>IP 地址</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="96"/>
        <source>logname</source>
        <translation>登录名</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="96"/>
        <source>display level</source>
        <translation>优先级</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="96"/>
        <source>status</source>
        <translation>状态</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="205"/>
        <source>Away</source>
        <translation>离开</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="209"/>
        <source>Stale</source>
        <translation>离线</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="213"/>
        <source>Online</source>
        <translation>在线</translation>
    </message>
    <message>
        <location filename="../user_manager.cpp" line="871"/>
        <source>Dial-up host</source>
        <translation>拨号主机</translation>
    </message>
</context>
<context>
    <name>WinShare</name>
    <message>
        <location filename="../winShare.cpp" line="133"/>
        <source>The address is not a plain IPv4 address.</source>
        <translation>这个地址不是普通的 IPv4 地址。</translation>
    </message>
    <message>
        <location filename="../winShare.cpp" line="137"/>
        <source>Windows network neighborhood is only available on Windows.</source>
        <translation>网上邻居只在 Windows 上可用。</translation>
    </message>
    <message>
        <location filename="../winShare.cpp" line="141"/>
        <source>Windows could not open %1 (code %2). If the share needs a password, Windows asks for it. This program does not collect share credentials.</source>
        <translation>Windows 无法打开 %1（代码 %2）。如果共享需要密码，由 Windows 询问。本程序不收集共享凭据。</translation>
    </message>
    <message>
        <location filename="../winShare.cpp" line="146"/>
        <source>The share could not be opened.</source>
        <translation>无法打开该共享。</translation>
    </message>
</context>
<context>
    <name>WindowManager</name>
    <message>
        <location filename="../window_manager.cpp" line="193"/>
        <location filename="../window_manager.cpp" line="225"/>
        <location filename="../window_manager.cpp" line="498"/>
        <source>[Window shake]</source>
        <translation>[窗口抖动]</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="198"/>
        <location filename="../window_manager.cpp" line="237"/>
        <location filename="../window_manager.cpp" line="418"/>
        <source>[Broadcast] %1</source>
        <translation>[广播] %1</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="244"/>
        <source>[File attachment]</source>
        <translation>[文件附件]</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="262"/>
        <source>(Sealed message)</source>
        <translation>（密信）</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="325"/>
        <source>[Instant] %1 already present, skipped transfer</source>
        <translation>[秒传] %1 已在本地，跳过传输</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="331"/>
        <source>[Instant] %1 already on the peer, skipped transfer</source>
        <translation>[秒传] %1 对方已有，跳过传输</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="556"/>
        <location filename="../window_manager.cpp" line="583"/>
        <source>Peer offline</source>
        <translation>对方离线</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="557"/>
        <source>%1 is offline. Queue this message and send when they come online?</source>
        <translation>%1 当前离线。将这条消息加入队列，等对方上线后再发送？</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="584"/>
        <source>%1 is offline. Queue this window shake and send when they come online?</source>
        <translation>%1 当前离线。把这次窗口抖动加入队列，等对方上线后再发送？</translation>
    </message>
    <message>
        <location filename="../window_manager.cpp" line="766"/>
        <source>I am away from my computer.</source>
        <translation>我现在不在电脑旁。</translation>
    </message>
</context>
<context>
    <name>ZoneBrowser</name>
    <message>
        <location filename="../zoneBrowser.cpp" line="36"/>
        <location filename="../zoneBrowser.cpp" line="161"/>
        <location filename="../zoneBrowser.cpp" line="169"/>
        <location filename="../zoneBrowser.cpp" line="185"/>
        <location filename="../zoneBrowser.cpp" line="199"/>
        <location filename="../zoneBrowser.cpp" line="207"/>
        <location filename="../zoneBrowser.cpp" line="220"/>
        <location filename="../zoneBrowser.cpp" line="233"/>
        <location filename="../zoneBrowser.cpp" line="246"/>
        <source>Local notes</source>
        <translation>本地笔记</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="40"/>
        <location filename="../zoneBrowser.cpp" line="270"/>
        <source>Notes in %1. Markdown and text only. They stay on this computer. This is not a FeiQ zone database and nothing here is sent to peers.</source>
        <translation>%1 里的笔记，仅限 Markdown 和文本。它们只留在这台电脑上。这不是飞秋空间数据库，也不会发给对方。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="52"/>
        <source>Select a note, or create one.</source>
        <translation>选择一条笔记，或新建一条。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="55"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="56"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="57"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="58"/>
        <source>Open folder</source>
        <translation>打开文件夹</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="59"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="161"/>
        <source>Title</source>
        <translation>标题</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="170"/>
        <source>Could not create the note.</source>
        <translation>无法新建笔记。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="186"/>
        <location filename="../zoneBrowser.cpp" line="247"/>
        <source>Could not save the note.</source>
        <translation>无法保存笔记。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="200"/>
        <source>Delete %1? This only removes the file on this computer.</source>
        <translation>删除 %1？这只会删掉这台电脑上的文件。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="208"/>
        <source>Could not delete the note.</source>
        <translation>无法删除笔记。</translation>
    </message>
    <message>
        <location filename="../zoneBrowser.cpp" line="234"/>
        <source>Save changes to %1?</source>
        <translation>保存对 %1 的修改？</translation>
    </message>
</context>
</TS>
