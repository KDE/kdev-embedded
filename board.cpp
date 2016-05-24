#include "board.h"
#include "toolkit.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#include <QMap>

#include <interfaces/isession.h>
#include <interfaces/icore.h>
#include <KConfigGroup>

Q_LOGGING_CATEGORY(BoMsg, "Kdev.embedded.bo.msg");

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

QString Board::getIdFromName(QString _name)
{
    update();
    foreach(const auto& boardId, m_boardList)
        if(m_boards[boardId].m_name[0] == _name)
            return boardId;
    return QString();
}

Board& Board::instance()
{
    static Board self;
    return self;
}

Board::Board()
{
    m_listed = false;
}

void Board::update()
{
    if(!m_listed)
        load();
    m_listed = true;
}

QString Board::Freq2FreqHz(QString freq)
{
        return QString::number(freq.left(freq.lastIndexOf("0")+1).toInt()/1e6)+"MHz";
}

void Board::load()
{

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    QFile m_boardsFile(Toolkit::getBoardFile(settings.readEntry("arduinoFolder","")));
    bool fileOpened = m_boardsFile.open(QFile::ReadOnly);
    qCDebug(BoMsg) << "Board file opened" << fileOpened;
    Q_ASSERT(fileOpened);
    QTextStream boardsFileUTF8(&m_boardsFile);
    boardsFileUTF8.setCodec("UTF-8");

    while (!boardsFileUTF8.atEnd())
    {
        QString line = boardsFileUTF8.readLine().trimmed();

        if (line.isEmpty() || line[0] == '#')
            continue;

        QString attrName = line.section('=', 0, 0);
        QString attrValue = line.section('=', 1);

        // attrName = <product>.<attrName>
        QString productId = attrName.section('.', 0, 0);
        attrName = attrName.section('.', 1);

        if(productId.contains("menu"))
            continue;

        int lineType = attrName.split(".").size();

        // Normal type
        if(lineType!=0)
        {
            // Board
            if(attrName.contains("name"))
            {
                m_boardList << productId;
                m_boardNameList << attrValue;
                m_boards[productId].m_name << attrValue;
            }

            else if(attrName.contains("vid"))
                m_boards[productId].m_vid << attrValue;

            else if(attrName.contains("pid"))
                m_boards[productId].m_pid << attrValue;

            // Upload type
            else if(attrName.contains("upload.tool"))
                m_boards[productId].m_upTool << attrValue;

            else if(attrName.contains("upload.protocol"))
                m_boards[productId].m_upProtocol << attrValue;

            else if(attrName.contains("upload.maximum_size"))
            {
                m_boards[productId].m_upMaxSize << attrValue;
                m_boards[productId].m_upMaxSizeKb << QString::number((attrValue).toInt()/1024);
            }

            else if(attrName.contains("upload.maximum_data_size"))
            {
                m_boards[productId].m_upMaxDataSize << attrValue;
                m_boards[productId].m_upMaxDataSizeKb << QString::number((attrValue).toInt()/1024);
            }

            else if(attrName.contains("upload.speed"))
                m_boards[productId].m_upSpeed << attrValue;

            else if(attrName.contains("upload.disable_flushing"))
                m_boards[productId].m_upDisableFlush << attrValue;

            else if(attrName.contains("upload.use_1200bps_touch"))
                m_boards[productId].m_upUse1k2bpsTouch << attrValue;

            else if(attrName.contains("upload.wait_for_upload_port"))
                m_boards[productId].m_upWaitForUploadPort << attrValue;

            // Boodloader
            else if(attrName.contains("bootloader.tool"))
                m_boards[productId].m_blTool << attrValue;

            else if(attrName.contains("bootloader.low_fuses"))
                m_boards[productId].m_blLowFuses << attrValue;

            else if(attrName.contains("bootloader.high_fuses"))
                m_boards[productId].m_blHighFuses << attrValue;

            else if(attrName.contains("bootloader.extended_fuses"))
                m_boards[productId].m_blExtendedFuses << attrValue;

            else if(attrName.contains("bootloader.file"))
                m_boards[productId].m_blFile << attrValue;

            else if(attrName.contains("bootloader.noblink"))
                m_boards[productId].m_blNoblink << attrValue;

            else if(attrName.contains("bootloader.unlock_bits"))
                m_boards[productId].m_blUnlockBits << attrValue;

            else if(attrName.contains("bootloader.lock_bits"))
                m_boards[productId].m_blLockBits << attrValue;

            // Build
            else if(attrName.contains("build.mcu") && !attrValue.contains("atmegang"))
                m_boards[productId].m_bMcu << attrValue;

            else if(attrName.contains("build.f_cpu"))
            {
                m_boards[productId].m_bFcpu << attrValue;
                m_boards[productId].m_freqHz << Freq2FreqHz(attrValue);
            }

            else if(attrName.contains("build.vid"))
                m_boards[productId].m_bVid << attrValue;

            else if(attrName.contains("build.pid"))
                m_boards[productId].m_bPid << attrValue;

            else if(attrName.contains("build.usb_product"))
                m_boards[productId].m_bUsbProduct << attrValue;

            else if(attrName.contains("build.board"))
                m_boards[productId].m_bUsbProduct << attrValue;

            else if(attrName.contains("build.core"))
                m_boards[productId].m_bCore << attrValue;

            else if(attrName.contains("build.variant"))
                m_boards[productId].m_bVariant << attrValue;

            else if(attrName.contains("build.extra_flags"))
                m_boards[productId].m_bExtraFlags << attrValue;

            else m_boards[productId].m_NaO << QString("(%1,%2,%3)").arg(productId).arg(attrName).arg(attrValue);
        }
    }
    m_boardsFile.close();
}
