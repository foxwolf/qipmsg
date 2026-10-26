// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "recv_file.h"
#include "constants.h"
#include "helper.h"
#include "protocolutil.h"

#include <QStringList>

RecvFile::RecvFile(QString ip, QString packetNoString, QString info,
                   quint16 peerPort)
    : m_ip(ip), m_connectPort(0), m_peerPort(peerPort),
    m_packetNoString(packetNoString),
    m_fileId(0), m_size(0), m_type(-1), m_parsed(false), m_attrWord(0),
    m_offset(0), m_bytesReaded(0), m_regularFileCount(0), m_state(NotRecv)
{
    const FileAttachParsed parsed = ProtocolUtil::parseFileAttachment(info);
    m_parsed = parsed.parsed;
    m_rawSegment = parsed.segment;
    m_rawAttr = parsed.rawAttr;
    m_attrWord = parsed.attrWord;
    if (!parsed.parsed) {
        return;
    }
    m_fileId = parsed.fileId;
    m_name = parsed.name;
    m_size = parsed.size;
    m_type = parsed.type;
    m_attrMap = parsed.extend;
}

RecvFile::~RecvFile()
{
}

QString RecvFile::sizeString() const
{
    if (m_type == IPMSG_FILE_DIR) {
        return QObject::tr("Folder");
    }

    return QString("%1").arg(Helper::sizeStringUnit(m_size, " "));
}

QString RecvFile::transferStatsInfo() const
{
    if (m_type == IPMSG_FILE_REGULAR) {
        QString fileSizeString = Helper::sizeStringUnit(m_bytesReaded) + "/"
            + Helper::sizeStringUnit(m_size);

        QString transferRateString
            = Helper::sizeStringUnit(transferRateAvg()) + "/s";

        QString transferPercentString = QString("(%1%2)")
            .arg(percent(), 0, 'f', 0)
            .arg("%");

        return (fileSizeString + " " + transferRateString
                + " " + transferPercentString);
    } else if (m_type == IPMSG_FILE_DIR) {
            return (QObject::tr("Folder") + " "
                    + Helper::sizeStringUnit((double)m_bytesReaded) + " / "
                    + Helper::fileCountString(m_regularFileCount) + " ("
                    + Helper::sizeStringUnit(transferRateAvg()) + "/s)");
    }

    return QString();
}

double RecvFile::transferRateAvg() const
{
    return m_bytesReaded / (double)elapse();
}

double RecvFile::percent() const
{
    // If a empty file
    if (m_size == 0) {
        return 100;
    }

    return ((double)m_bytesReaded / m_size) * 100;
}

void RecvFile::resetStats()
{
    m_bytesReaded = 0;
    m_regularFileCount = 0;
}

