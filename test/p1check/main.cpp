// Self-check for cross-subnet range tokens and IPMSG option helpers.
// Not a GUI test: it links protocolutil.cpp and Qt Core/Network only.

#include "autostart.h"
#include "protocolutil.h"
#include "recvEndpoints.h"

#include <QCoreApplication>
#include <QHostAddress>

#include <cstdio>

static int g_fails = 0;

static void check(bool cond, const char *text, int line)
{
    if (!cond) {
        std::fprintf(stderr, "fail %d: %s\n", line, text);
        ++g_fails;
    }
}

#define CHECK(cond) check((cond), #cond, __LINE__)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString canonical;
    quint32 start = 0;
    quint32 end = 0;

    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.10", &canonical, &start, &end) == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.10"));
    CHECK(start == end);
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0).size() == 1);

    canonical.clear();
    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.12 - 192.168.2.10", &canonical, &start, &end)
          == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.10-192.168.2.12"));
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0)
          == (QStringList() << "192.168.2.10" << "192.168.2.11" << "192.168.2.12"));

    canonical.clear();
    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.15/24", &canonical, &start, &end) == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.0/24"));
    CHECK(QHostAddress(start).toString() == QLatin1String("192.168.2.0"));
    CHECK(QHostAddress(end).toString() == QLatin1String("192.168.2.255"));
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0).size() == 256);

    CHECK(ProtocolUtil::inspectIpv4Token("10.0.0.0/22", &canonical, &start, &end)
          == ProtocolUtil::Ipv4TokenOk);
    CHECK(quint64(end) - quint64(start) + 1 == 1024u);

    CHECK(ProtocolUtil::inspectIpv4Token("10.0.0.0/21", 0, &start, &end)
          == ProtocolUtil::Ipv4TokenTooLarge);
    bool truncated = false;
    const QStringList capped = ProtocolUtil::expandIpv4Hosts(start, end, &truncated);
    CHECK(truncated);
    CHECK(capped.size() == ProtocolUtil::maxIpv4RangeHosts());

    CHECK(ProtocolUtil::inspectIpv4Token("10.1.2.3/33", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenBadPrefix);
    CHECK(ProtocolUtil::inspectIpv4Token("not-an-ip", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenInvalid);
    CHECK(ProtocolUtil::inspectIpv4Token("127.0.0.1", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenUnusable);
    CHECK(ProtocolUtil::inspectIpv4Token("", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenEmpty);

    int invalid = 0;
    int truncatedCount = 0;
    const QList<QHostAddress> targets = ProtocolUtil::targetsFromBroadcastTokens(
                QStringList() << "192.168.2.10-192.168.2.12" << "bogus" << "10.9.8.7",
                &invalid, &truncatedCount);
    CHECK(invalid == 1);
    CHECK(truncatedCount == 0);
    CHECK(targets.size() == 4);

    const quint32 utf8Send = IPMSG_SENDMSG | IPMSG_UTF8OPT;
    QByteArray datagram = QByteArray("1:100:user:host:")
            + QByteArray::number(utf8Send)
            + QByteArray(":hello");
    datagram.append('\0');
    quint32 command = 0;
    CHECK(ProtocolUtil::commandFromDatagram(datagram, &command));
    CHECK((command & IPMSG_UTF8OPT) != 0);
    CHECK(GET_MODE(command) == IPMSG_SENDMSG);

    CHECK(ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG | IPMSG_SECRETOPT));
    CHECK(ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG | IPMSG_READCHECKOPT));
    CHECK(!ProtocolUtil::isSecretEnvelope(IPMSG_BR_ENTRY | IPMSG_SERVEROPT));
    CHECK(!ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG));

    CHECK(ProtocolUtil::readReceiptPacketNo("4321", "99") == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo(QString("4321") + QChar('\0') + "extra", "99")
          == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo("4321 extra", "99") == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo(QString(), "77") == QLatin1String("77"));
    CHECK(ProtocolUtil::readReceiptPacketNo("opened", "77").isEmpty());

    bool typing = false;
    CHECK(ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPING, &typing));
    CHECK(typing);
    CHECK(IPMSG_FEIQ_TYPING == 121u);
    CHECK(ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPINGSTOP, &typing));
    CHECK(!typing);
    CHECK(IPMSG_FEIQ_TYPINGSTOP == 122u);
    CHECK(!ProtocolUtil::isFeiqTypingCommand(IPMSG_SENDMSG, &typing));
    CHECK(!ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPING | IPMSG_UTF8OPT, &typing));
    command = 0;
    CHECK(ProtocolUtil::commandFromDatagram(
            QByteArray("1_lbt6_0#128#AABBCCDDEEFF#0#0#0#4001#9:9:user:host:121:"),
            &command));
    CHECK(command == IPMSG_FEIQ_TYPING);
    CHECK(ProtocolUtil::commandFromDatagram(
            QByteArray("1:9:user:host:122:"), &command));
    CHECK(command == IPMSG_FEIQ_TYPINGSTOP);
    CHECK(ProtocolUtil::etaSeconds(1000, 5000, 1000) == 4);
    CHECK(ProtocolUtil::etaSeconds(0, 5000, 1000) < 0);
    CHECK(ProtocolUtil::etaSeconds(5000, 5000, 1000) < 0);

    const QString desktop = AutoStart::xdgDesktopText("/tmp/my app/qipmsg");
    CHECK(desktop.contains("X-QIpMsg-Autostart=true"));
    CHECK(desktop.contains("X-GNOME-Autostart-enabled=true"));
    CHECK(desktop.contains("Exec=\"/tmp/my app/qipmsg\""));
    CHECK(AutoStart::xdgFileName() == QLatin1String("qipmsg-autostart.desktop"));
    const QString agent = AutoStart::macAgentText("/Applications/QIpMsg.app/Contents/MacOS/qipmsg");
    CHECK(agent.contains("org.qipmsg.login"));
    CHECK(agent.contains("<key>RunAtLoad</key>"));
    CHECK(agent.contains("/Applications/QIpMsg.app/Contents/MacOS/qipmsg"));
    CHECK(AutoStart::windowsValueName() == QLatin1String("QIpMsg"));

    // FeiQ on two NICs: UDP from .164, TCP file port on the configured .3.
    RecvPeerHint lan;
    lan.ip = QStringLiteral("192.168.14.164");
    lan.port = 2425;
    lan.login = QStringLiteral("win");
    lan.host = QStringLiteral("WORK-LAPTOP");
    lan.group = QString::fromUtf8("盛达物联");
    RecvPeerHint zt = lan;
    zt.ip = QStringLiteral("10.147.19.236");
    RecvPeerHint other = lan;
    other.ip = QStringLiteral("192.168.14.50");
    other.login = QStringLiteral("other");
    QList<RecvPeerHint> peers;
    peers << lan << zt << other;

    const QStringList configured = QStringList()
            << QStringLiteral("192.168.14.3")
            << QStringLiteral("192.168.14.0/24")
            << QStringLiteral("10.9.8.7")
            << QStringLiteral("192.168.14.255");
    QList<RecvEndpoint> fromLan = recvFileEndpoints(
                QStringLiteral("192.168.14.164"), 2425, peers, configured, 2425);
    CHECK(fromLan.size() == 3);
    CHECK(fromLan.at(0).ip == QLatin1String("192.168.14.164"));
    CHECK(fromLan.at(0).port == 2425);
    CHECK(fromLan.at(1).ip == QLatin1String("192.168.14.3"));
    CHECK(fromLan.at(1).port == 2425);
    CHECK(fromLan.at(2).ip == QLatin1String("10.147.19.236"));

    QList<RecvEndpoint> fromZt = recvFileEndpoints(
                QStringLiteral("10.147.19.236"), 2425, peers, configured, 2425);
    CHECK(fromZt.size() == 3);
    CHECK(fromZt.at(0).ip == QLatin1String("10.147.19.236"));
    CHECK(fromZt.at(1).ip == QLatin1String("192.168.14.3"));
    CHECK(fromZt.at(2).ip == QLatin1String("192.168.14.164"));

    RecvPeerHint sameLan = lan;
    sameLan.ip = QStringLiteral("192.168.14.165");
    sameLan.port = 2425;
    peers.insert(1, sameLan);
    QList<RecvEndpoint> withSibling = recvFileEndpoints(
                QStringLiteral("192.168.14.164"), 0, peers, configured, 2425);
    CHECK(withSibling.at(0).port == 2425);
    CHECK(withSibling.at(1).ip == QLatin1String("192.168.14.165"));
    CHECK(withSibling.at(2).ip == QLatin1String("192.168.14.3"));

    QList<RecvEndpoint> alone = recvFileEndpoints(
                QStringLiteral("192.168.4.170"), 12425, QList<RecvPeerHint>(),
                QStringList(), 2425);
    CHECK(alone.size() == 1);
    CHECK(alone.at(0).ip == QLatin1String("192.168.4.170"));
    CHECK(alone.at(0).port == 12425);

    RecvPeerHint folded = lan;
    folded.host = QStringLiteral("work-laptop");
    folded.ip = QStringLiteral("192.168.14.166");
    QList<RecvPeerHint> foldedPeers;
    foldedPeers << lan << folded;
    QList<RecvEndpoint> foldedEnds = recvFileEndpoints(
                lan.ip, 2425, foldedPeers, QStringList(), 2425);
    CHECK(foldedEnds.size() == 2);
    CHECK(foldedEnds.at(1).ip == QLatin1String("192.168.14.166"));

    RecvPeerHint blank = lan;
    blank.login.clear();
    blank.ip = QStringLiteral("192.168.14.167");
    RecvPeerHint blank2 = blank;
    blank2.ip = QStringLiteral("192.168.14.168");
    QList<RecvPeerHint> blanks;
    blanks << blank << blank2;
    QList<RecvEndpoint> noMerge = recvFileEndpoints(
                blank.ip, 2425, blanks, QStringList(), 2425);
    CHECK(noMerge.size() == 1);

    RecvPeerHint extraNic = lan;
    extraNic.ip = QStringLiteral("192.168.14.161");
    RecvPeerHint extraNic2 = lan;
    extraNic2.ip = QStringLiteral("192.168.14.162");
    RecvPeerHint extraNic3 = lan;
    extraNic3.ip = QStringLiteral("192.168.14.163");
    QList<RecvPeerHint> crowded;
    crowded << lan << extraNic << extraNic2 << extraNic3 << zt;
    QList<RecvEndpoint> keptConfig = recvFileEndpoints(
                lan.ip, 2425, crowded,
                QStringList() << QStringLiteral("192.168.14.3"), 2425);
    bool sawConfigured = false;
    for (int i = 0; i < keptConfig.size(); ++i) {
        if (keptConfig.at(i).ip == QLatin1String("192.168.14.3")) {
            sawConfigured = true;
        }
    }
    CHECK(sawConfigured);
    CHECK(keptConfig.size() <= 4);

    QList<RecvPeerHint> ztOnly;
    ztOnly << zt;
    QList<RecvEndpoint> ztConfigured = recvFileEndpoints(
                zt.ip, 2425, ztOnly,
                QStringList() << QStringLiteral("192.168.14.3")
                              << QStringLiteral("10.9.8.7"),
                2425);
    CHECK(ztConfigured.size() == 2);
    CHECK(ztConfigured.at(0).ip == QLatin1String("10.147.19.236"));
    CHECK(ztConfigured.at(1).ip == QLatin1String("192.168.14.3"));

    // FeiQ's own TCP GETFILE (captured) uses decimal command 96, a hex
    // packet id, and a trailing NUL. Hex command "60" is atol()'d as 60.
    const char feiqShape[] =
            "1:1501516970:Administrator:DONGGE-32E5DBE1:96:597ee52c:0:0:";
    const QByteArray pull = ProtocolUtil::makeFilePullRequest(
                QByteArray("1"), QByteArray("1501516970"),
                QByteArray("Administrator"), QByteArray("DONGGE-32E5DBE1"),
                IPMSG_GETFILEDATA, 0x597ee52cLL, 0, 0, true);
    CHECK(pull == QByteArray(feiqShape, int(sizeof(feiqShape))));
    CHECK(pull.contains(":96:"));
    CHECK(!pull.contains(":60:"));

    const QByteArray wide = ProtocolUtil::makeFilePullRequest(
                QByteArray("1_qipmsg"), QByteArray("9"),
                QByteArray("fox"), QByteArray("hw"),
                IPMSG_GETFILEDATA, 1523368691537LL, 10, 0, true);
    CHECK(wide.startsWith("1_qipmsg:9:fox:hw:96:162afd98751:a:0:"));
    CHECK(wide.endsWith(QByteArray(1, '\0')));
    CHECK(!wide.contains(":60:"));

    const QByteArray dirPull = ProtocolUtil::makeFilePullRequest(
                QByteArray("1"), QByteArray("3"),
                QByteArray("a"), QByteArray("b"),
                IPMSG_GETDIRFILES, 16, 2, 0, false);
    CHECK(dirPull.startsWith("1:3:a:b:98:10:2:"));
    CHECK(!dirPull.contains(":62:"));
    CHECK(dirPull.at(dirPull.size() - 1) == '\0');

    // Attachment fileattr: low 8 bits are the kind. Option bits must not
    // knock a later FeiQ offer off the regular/dir download path.
    const FileAttachParsed plain = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:1:"));
    CHECK(plain.parsed);
    CHECK(plain.fileId == 0);
    CHECK(plain.name == QLatin1String("a.txt"));
    CHECK(plain.size == 0x10);
    CHECK(plain.rawAttr == QLatin1String("1"));
    CHECK(plain.attrWord == 1u);
    CHECK(plain.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed readonly = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:101:"));
    CHECK(readonly.parsed);
    CHECK(readonly.attrWord == 0x101u);
    CHECK(readonly.type == IPMSG_FILE_REGULAR);
    CHECK(plain.type == IPMSG_FILE_REGULAR);
    CHECK(plain.attrWord == 1u);

    const FileAttachParsed archive = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:4001:"));
    CHECK(archive.parsed);
    CHECK(archive.attrWord == 0x4001u);
    CHECK(archive.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed folder = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:dir:0:20:2:"));
    CHECK(folder.parsed);
    CHECK(folder.type == IPMSG_FILE_DIR);

    const FileAttachParsed hiddenDir = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:dir:0:20:1002:"));
    CHECK(hiddenDir.parsed);
    CHECK(hiddenDir.attrWord == 0x1002u);
    CHECK(hiddenDir.type == IPMSG_FILE_DIR);

    const FileAttachParsed clip = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:clip.png:10:20:20:"));
    CHECK(clip.parsed);
    CHECK(clip.attrWord == 0x20u);
    CHECK(clip.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed zeroAttr = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:0:"));
    CHECK(zeroAttr.parsed);
    CHECK(zeroAttr.attrWord == 0u);
    CHECK(zeroAttr.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed symlink = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:link:10:20:4:"));
    CHECK(symlink.parsed);
    CHECK(symlink.type == IPMSG_FILE_SYMLINK);

    const FileAttachParsed reserved = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:4a:20:1:0:"));
    CHECK(reserved.parsed);
    CHECK(reserved.size == 0x4a);
    CHECK(reserved.rawAttr == QLatin1String("1"));
    CHECK(reserved.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed colonName = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:C:\\dir\\a.txt:4a:abcdef01:1:"));
    CHECK(colonName.parsed);
    CHECK(colonName.name == QLatin1String("C:\\dir\\a.txt"));
    CHECK(colonName.size == 0x4a);
    CHECK(colonName.rawAttr == QLatin1String("1"));
    CHECK(colonName.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed escaped = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:my::file.txt:10:20:1:"));
    CHECK(escaped.parsed);
    CHECK(escaped.name == QLatin1String("my:file.txt"));
    CHECK(escaped.size == 0x10);
    CHECK(escaped.type == IPMSG_FILE_REGULAR);

    const FileAttachParsed extend = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:101:14=abcdef:"));
    CHECK(extend.parsed);
    CHECK(extend.type == IPMSG_FILE_REGULAR);
    CHECK(extend.attrWord == 0x101u);
    CHECK(extend.extend.value(IPMSG_FILE_MTIME) == QLatin1String("abcdef"));

    const FileAttachParsed oddAttr = ProtocolUtil::parseFileAttachment(
                QStringLiteral("0:a.txt:10:20:weird:"));
    CHECK(oddAttr.parsed);
    CHECK(oddAttr.rawAttr == QLatin1String("weird"));
    CHECK(oddAttr.attrWord == 0u);
    CHECK(oddAttr.type == IPMSG_FILE_REGULAR);
    CHECK(oddAttr.size == 0x10);

    const FileAttachParsed font = ProtocolUtil::parseFileAttachment(
                QStringLiteral("font=a:size=1:color=0:style=0:extra=1"));
    CHECK(!font.parsed);

    CHECK(ProtocolUtil::fileAttrKind(0x101u, true) == IPMSG_FILE_REGULAR);
    CHECK(ProtocolUtil::fileAttrKind(0x20u, false) == IPMSG_FILE_REGULAR);
    CHECK(ProtocolUtil::fileAttrKind(0u, false) == -1);
    CHECK(ProtocolUtil::fileAttrKind(0u, true) == IPMSG_FILE_REGULAR);
    CHECK(ProtocolUtil::fileAttrKind(0x1002u, false) == IPMSG_FILE_DIR);
    CHECK(ProtocolUtil::fileAttrKind(IPMSG_FILE_RETPARENT, false)
          == IPMSG_FILE_RETPARENT);
    CHECK(ProtocolUtil::fileAttrKind(IPMSG_FILE_SYMLINK, true) == -1);

    const QString secondOffer = QStringLiteral("1:9:user:host:32:hi")
            + QChar('\0')
            + QStringLiteral("font=a:size=1:color=0:style=0:extra=1")
            + QChar('\0')
            + QStringLiteral("0:b.txt:20:30:101:\a1:c.txt:30:40:2:");
    const QStringList segments = ProtocolUtil::fileAttachSegments(secondOffer);
    CHECK(segments.size() == 3);
    CHECK(!ProtocolUtil::parseFileAttachment(segments.at(0)).parsed);
    const FileAttachParsed secondFile = ProtocolUtil::parseFileAttachment(
                segments.at(1));
    CHECK(secondFile.parsed);
    CHECK(secondFile.name == QLatin1String("b.txt"));
    CHECK(secondFile.fileId == 0);
    CHECK(secondFile.attrWord == 0x101u);
    CHECK(secondFile.type == IPMSG_FILE_REGULAR);
    const FileAttachParsed secondDir = ProtocolUtil::parseFileAttachment(
                segments.at(2));
    CHECK(secondDir.parsed);
    CHECK(secondDir.fileId == 1);
    CHECK(secondDir.type == IPMSG_FILE_DIR);

    const FileAttachParsed decimalId = ProtocolUtil::parseFileAttachment(
                QStringLiteral("10:a.txt:10:20:1:"));
    CHECK(decimalId.parsed);
    CHECK(decimalId.fileId == 10);

    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("qipmsgP1Check ok\n");
    return 0;
}
