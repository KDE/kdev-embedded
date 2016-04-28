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

    qCDebug(BoMsg) << "name" << name;
    qCDebug(BoMsg) << "vid"  << vid;
    qCDebug(BoMsg) << "pid"  << pid;

    // Upload

    qCDebug(BoMsg) << "upTool"              << upTool;
    qCDebug(BoMsg) << "upProtocol"          << upProtocol;
    qCDebug(BoMsg) << "upMaxSize"           << upMaxSize;
    qCDebug(BoMsg) << "upMaxDataSize"       << upMaxDataSize;
    qCDebug(BoMsg) << "upSpeed"             << upSpeed;
    qCDebug(BoMsg) << "upDisableFlush"      << upDisableFlush;
    qCDebug(BoMsg) << "upUse1k2bpsTouch"    << upUse1k2bpsTouch;
    qCDebug(BoMsg) << "upWaitForUploadPort" << upWaitForUploadPort;

    // Boot loader

    qCDebug(BoMsg) << "blTool"          << blTool;
    qCDebug(BoMsg) << "blLowFuses"      << blLowFuses;
    qCDebug(BoMsg) << "blHighFuses"     << blHighFuses;
    qCDebug(BoMsg) << "blExtendedFuses" << blExtendedFuses;
    qCDebug(BoMsg) << "blFile"          << blFile;
    qCDebug(BoMsg) << "blNoblink"       << blNoblink;
    qCDebug(BoMsg) << "blUnlockBits"    << blUnlockBits;
    qCDebug(BoMsg) << "blLockBits"      << blLockBits;

    // Build

    qCDebug(BoMsg) << "bMcu"        << bMcu;
    qCDebug(BoMsg) << "bFcpu"       << bFcpu;
    qCDebug(BoMsg) << "bVid"        << bVid;
    qCDebug(BoMsg) << "bPid"        << bPid;
    qCDebug(BoMsg) << "bUsbProduct" << bUsbProduct;
    qCDebug(BoMsg) << "bBoard"      << bBoard;
    qCDebug(BoMsg) << "bCore"       << bCore;
    qCDebug(BoMsg) << "bVariant"    << bVariant;
    qCDebug(BoMsg) << "bExtraFlags" << bExtraFlags;

    // Not an Option
    qCDebug(BoMsg) << "NaO" << NaO;
}

QString Board::getIdFromName(QString _name)
{
    update();
    foreach(const auto& boardId, boardList)
        if(boards[boardId].name[0] == _name)
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
  listed=false;
}

void Board::update()
{
  if(!listed)
      load();
  listed=true;
}

void Board::load()
{

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    QFile boardsFile(Toolkit::getBoardFile(settings.readEntry("arduinoFolder","")));
    boardsFile.open(QFile::ReadOnly);
    QTextStream boardsFileUTF8(&boardsFile);
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
        if(lineType<4)
        {
            // Board
            if(attrName.contains("name"))
            {
                boardList << productId;
                boardNameList << attrValue;
                boards[productId].name << attrValue;
            }

            else if(attrName.contains("vid"))
                boards[productId].vid << attrValue;

            else if(attrName.contains("pid"))
                boards[productId].pid << attrValue;

            // Upload type
            else if(attrName.contains("upload.tool"))
                boards[productId].upTool << attrValue;

            else if(attrName.contains("upload.protocol"))
                boards[productId].upProtocol << attrValue;

            else if(attrName.contains("upload.maximum_size"))
                boards[productId].upMaxSize << attrValue;

            else if(attrName.contains("upload.maximum_data_size"))
                boards[productId].upMaxDataSize << attrValue;

            else if(attrName.contains("upload.speed"))
                boards[productId].upSpeed << attrValue;

            else if(attrName.contains("upload.disable_flushing"))
                boards[productId].upDisableFlush << attrValue;

            else if(attrName.contains("upload.use_1200bps_touch"))
                boards[productId].upUse1k2bpsTouch << attrValue;

            else if(attrName.contains("upload.wait_for_upload_port"))
                boards[productId].upWaitForUploadPort << attrValue;

            // Boodloader
            else if(attrName.contains("bootloader.tool"))
                boards[productId].blTool << attrValue;

            else if(attrName.contains("bootloader.low_fuses"))
                boards[productId].blLowFuses << attrValue;

            else if(attrName.contains("bootloader.high_fuses"))
                boards[productId].blHighFuses << attrValue;

            else if(attrName.contains("bootloader.extended_fuses"))
                boards[productId].blExtendedFuses << attrValue;

            else if(attrName.contains("bootloader.file"))
                boards[productId].blFile << attrValue;

            else if(attrName.contains("bootloader.noblink"))
                boards[productId].blNoblink << attrValue;

            else if(attrName.contains("bootloader.unlock_bits"))
                boards[productId].blUnlockBits << attrValue;

            else if(attrName.contains("bootloader.lock_bits"))
                boards[productId].blLockBits << attrValue;

            // Build
            else if(attrName.contains("build.mcu"))
                boards[productId].bMcu << attrValue;

            else if(attrName.contains("build.f_cpu"))
                boards[productId].bFcpu << attrValue;

            else if(attrName.contains("build.vid"))
                boards[productId].bVid << attrValue;

            else if(attrName.contains("build.pid"))
                boards[productId].bPid << attrValue;

            else if(attrName.contains("build.usb_product"))
                boards[productId].bUsbProduct << attrValue;

            else if(attrName.contains("build.board"))
                boards[productId].bUsbProduct << attrValue;

            else if(attrName.contains("build.core"))
                boards[productId].bCore << attrValue;

            else if(attrName.contains("build.variant"))
                boards[productId].bVariant << attrValue;

            else if(attrName.contains("build.extra_flags"))
                boards[productId].bExtraFlags << attrValue;

            else boards[productId].NaO << attrValue;
        }
    }
    boardsFile.close();
}
