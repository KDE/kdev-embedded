/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "board.h"
#include "toolkit.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#include <QMap>

#include <interfaces/isession.h>
#include <interfaces/icore.h>
#include <KConfigGroup>

Q_LOGGING_CATEGORY(BoMsg, "Kdev.embedded.bo.msg")

using namespace KDevelop;

void BoardInfo::printData()
{
    // Board

    qCDebug(BoMsg) << "name" << m_name;
    qCDebug(BoMsg) << "vid"  << m_vid;
    qCDebug(BoMsg) << "pid"  << m_pid;

    // Upload

    qCDebug(BoMsg) << "upTool"              << m_upTool;
    qCDebug(BoMsg) << "upProtocol"          << m_upProtocol;
    qCDebug(BoMsg) << "upMaxSize"           << m_upMaxSize;
    qCDebug(BoMsg) << "upMaxDataSize"       << m_upMaxDataSize;
    qCDebug(BoMsg) << "upSpeed"             << m_upSpeed;
    qCDebug(BoMsg) << "upDisableFlush"      << m_upDisableFlush;
    qCDebug(BoMsg) << "upUse1k2bpsTouch"    << m_upUse1k2bpsTouch;
    qCDebug(BoMsg) << "upWaitForUploadPort" << m_upWaitForUploadPort;

    // Boot loader

    qCDebug(BoMsg) << "blTool"          << m_blTool;
    qCDebug(BoMsg) << "blLowFuses"      << m_blLowFuses;
    qCDebug(BoMsg) << "blHighFuses"     << m_blHighFuses;
    qCDebug(BoMsg) << "blExtendedFuses" << m_blExtendedFuses;
    qCDebug(BoMsg) << "blFile"          << m_blFile;
    qCDebug(BoMsg) << "blNoblink"       << m_blNoblink;
    qCDebug(BoMsg) << "blUnlockBits"    << m_blUnlockBits;
    qCDebug(BoMsg) << "blLockBits"      << m_blLockBits;

    // Build

    qCDebug(BoMsg) << "bMcu"        << m_bMcu;
    qCDebug(BoMsg) << "bFcpu"       << m_bFcpu;
    qCDebug(BoMsg) << "bVid"        << m_bVid;
    qCDebug(BoMsg) << "bPid"        << m_bPid;
    qCDebug(BoMsg) << "bUsbProduct" << m_bUsbProduct;
    qCDebug(BoMsg) << "bBoard"      << m_bBoard;
    qCDebug(BoMsg) << "bCore"       << m_bCore;
    qCDebug(BoMsg) << "bVariant"    << m_bVariant;
    qCDebug(BoMsg) << "bExtraFlags" << m_bExtraFlags;

    // Not an Option
    qCDebug(BoMsg) << "NaO" << m_NaO;
}

Board::Board()
{
    m_listed = false;
    m_arduinoFolder = new QFile(Toolkit::instance().arduinoPath());
    m_arduinoFolderFail = !m_arduinoFolder->exists();
    qCDebug(BoMsg) << "m_arduinoFolderFail" << m_arduinoFolderFail << m_arduinoFolder->fileName();
}

Board::~Board()
{
    delete m_arduinoFolder;
}

QString Board::getIdFromName(QString _name)
{
    update();
    foreach (const auto& boardId, m_boardList)
    {
        if (m_boards[boardId].m_name[0] == _name)
        {
            return boardId;
        }
    }
    return QString();
}

Board& Board::instance()
{
    static Board self;
    return self;
}

void Board::update()
{
    if (!m_listed)
    {
        load();
    }
    m_listed = true;
}

QString Board::Freq2FreqHz(QString freq)
{
    return QStringLiteral("%0 %1").arg(QString::number(freq.left(freq.lastIndexOf(QChar::fromLatin1('L'))).toInt() /  1e6)).arg(QStringLiteral("MHz"));
}

void Board::load()
{
    if (m_arduinoFolderFail)
    {
        qCDebug(BoMsg) << "Board::load" << "error in arduino folder";
        return;
    }

    qCDebug(BoMsg) << "Board::load m_boardsFile m_arduinoFolder->fileName()" << m_arduinoFolder->fileName();
    QFile m_boardsFile(Toolkit::instance().boardFile());
    qCDebug(BoMsg) << "Board::load m_boardsFile local" << m_boardsFile.fileName();
    bool fileOpened = m_boardsFile.open(QFile::ReadOnly);
    qCDebug(BoMsg) << "Board::load fileOpened" << fileOpened;

    // if no boards.txt, nothing to do
    if (!fileOpened)
    {
        return;
    }

    QTextStream boardsFileUTF8(&m_boardsFile);
    boardsFileUTF8.setCodec("UTF-8");

    while (!boardsFileUTF8.atEnd())
    {
        const QString line = boardsFileUTF8.readLine().trimmed();

        if (line.isEmpty() || line[0] == QChar::fromLatin1('#'))
        {
            continue;
        }

        QString attrName = line.section(QChar::fromLatin1('='), 0, 0);
        const QString attrValue = line.section(QChar::fromLatin1('='), 1);

        // attrName = <product>.<attrName>
        const QString productId = attrName.section(QChar::fromLatin1('.'), 0, 0);
        attrName = attrName.section(QChar::fromLatin1('.'), 1);

        if (productId.contains(QStringLiteral("menu")))
        {
            continue;
        }

        int lineType = attrName.split(QStringLiteral(".")).size();

        // Normal type
        if (lineType != 0)
        {
            // Board
            if (attrName.contains(QStringLiteral("name")))
            {
                m_boardList << productId;
                m_boardNameList << attrValue;
                m_boards[productId].m_name << attrValue;
            }

            else if (attrName.contains(QStringLiteral("vid")))
            {
                m_boards[productId].m_vid << attrValue;
            }

            else if (attrName.contains(QStringLiteral("pid")))
            {
                m_boards[productId].m_pid << attrValue;
            }

            // Upload type
            else if (attrName.contains(QStringLiteral("upload.tool")))
            {
                m_boards[productId].m_upTool << attrValue;
            }

            else if (attrName.contains(QStringLiteral("upload.protocol")))
            {
                m_boards[productId].m_upProtocol << attrValue;
            }

            else if (attrName.contains(QStringLiteral("upload.maximum_size")))
            {
                m_boards[productId].m_upMaxSize << attrValue;
                m_boards[productId].m_upMaxSizeKb << QString::number(attrValue.toInt() / 1024);
            }

            else if (attrName.contains(QStringLiteral("upload.maximum_data_size")))
            {
                m_boards[productId].m_upMaxDataSize << attrValue;
                m_boards[productId].m_upMaxDataSizeKb << QString::number(attrValue.toInt() / 1024);
            }

            else if (attrName.contains(QStringLiteral("upload.speed")))
            {
                m_boards[productId].m_upSpeed << attrValue;
            }

            else if (attrName.contains(QStringLiteral("upload.disable_flushing")))
            {
                m_boards[productId].m_upDisableFlush << attrValue;
            }

            else if (attrName.contains(QStringLiteral("upload.use_1200bps_touch")))
            {
                m_boards[productId].m_upUse1k2bpsTouch << attrValue;
            }

            else if (attrName.contains(QStringLiteral("upload.wait_for_upload_port")))
            {
                m_boards[productId].m_upWaitForUploadPort << attrValue;
            }

            // Boodloader
            else if (attrName.contains(QStringLiteral("bootloader.tool")))
            {
                m_boards[productId].m_blTool << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.low_fuses")))
            {
                m_boards[productId].m_blLowFuses << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.high_fuses")))
            {
                m_boards[productId].m_blHighFuses << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.extended_fuses")))
            {
                m_boards[productId].m_blExtendedFuses << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.file")))
            {
                m_boards[productId].m_blFile << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.noblink")))
            {
                m_boards[productId].m_blNoblink << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.unlock_bits")))
            {
                m_boards[productId].m_blUnlockBits << attrValue;
            }

            else if (attrName.contains(QStringLiteral("bootloader.lock_bits")))
            {
                m_boards[productId].m_blLockBits << attrValue;
            }

            // Build
            else if (attrName.contains(QStringLiteral("build.mcu")) && !attrValue.contains(QStringLiteral("atmegang")))
            {
                m_boards[productId].m_bMcu << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.f_cpu")))
            {
                m_boards[productId].m_bFcpu << attrValue;
                m_boards[productId].m_freqHz << Freq2FreqHz(attrValue);
            }

            else if (attrName.contains(QStringLiteral("build.vid")))
            {
                m_boards[productId].m_bVid << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.pid")))
            {
                m_boards[productId].m_bPid << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.usb_product")))
            {
                m_boards[productId].m_bUsbProduct << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.board")))
            {
                m_boards[productId].m_bUsbProduct << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.core")))
            {
                m_boards[productId].m_bCore << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.variant")))
            {
                m_boards[productId].m_bVariant << attrValue;
            }

            else if (attrName.contains(QStringLiteral("build.extra_flags")))
            {
                m_boards[productId].m_bExtraFlags << attrValue;
            }

            else
            {
                m_boards[productId].m_NaO << QStringLiteral("(%1,%2,%3)").arg(productId).arg(attrName).arg(attrValue);
            }
        }
    }
    m_boardsFile.close();
}
