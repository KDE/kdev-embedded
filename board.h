#ifndef BOARD_H
#define BOARD_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QMap>

class BoardInfo
{
public:
    void printData();

    // Board

    QStringList name;
    QStringList vid;
    QStringList pid;

    // Upload

    QStringList upTool;
    QStringList upProtocol;
    QStringList upMaxSize;
    QStringList upMaxDataSize;
    QStringList upSpeed;
    QStringList upDisableFlush;
    QStringList upUse1k2bpsTouch;
    QStringList upWaitForUploadPort;

    // Boot loader

    QStringList blTool;
    QStringList blLowFuses;
    QStringList blHighFuses;
    QStringList blExtendedFuses;
    QStringList blFile;
    QStringList blNoblink;
    QStringList blUnlockBits;
    QStringList blLockBits;

    // Build

    QStringList bMcu;
    QStringList bFcpu;
    QStringList bVid;
    QStringList bPid;
    QStringList bUsbProduct;
    QStringList bBoard;
    QStringList bCore;
    QStringList bVariant;
    QStringList bExtraFlags;

    // Not an Option
    QStringList NaO;

};

/**
 * @brief A class to help deal with boards.txt
 *
 */
class Board
{
public:
    static  Board& instance();

    QMap<QString, BoardInfo> boards;

    QStringList boardList;
    QStringList boardNameList;
    void update();
    QString getIdFromName(QString _name);

private:
    Board& operator=(Board& other)=delete;
    Board (const Board& other)=delete;
    Board();

    void load();
    bool listed;
};

#endif // BOARD_H
