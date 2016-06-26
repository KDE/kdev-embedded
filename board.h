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

#pragma once

#include <QString>
#include <QStringList>
#include <QHash>
#include <QFile>
#include <QMap>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BoMsg)

//class QFile;

class BoardInfo
{
public:
    void printData();

    // Board

    QStringList m_name;
    QStringList m_vid;
    QStringList m_pid;

    // Upload

    QStringList m_upTool;
    QStringList m_upProtocol;
    QStringList m_upMaxSize;
    QStringList m_upMaxDataSize;
    QStringList m_upSpeed;
    QStringList m_upDisableFlush;
    QStringList m_upUse1k2bpsTouch;
    QStringList m_upWaitForUploadPort;

    // Boot loader

    QStringList m_blTool;
    QStringList m_blLowFuses;
    QStringList m_blHighFuses;
    QStringList m_blExtendedFuses;
    QStringList m_blFile;
    QStringList m_blNoblink;
    QStringList m_blUnlockBits;
    QStringList m_blLockBits;

    // Build

    QStringList m_bMcu;
    QStringList m_bFcpu;
    QStringList m_bVid;
    QStringList m_bPid;
    QStringList m_bUsbProduct;
    QStringList m_bBoard;
    QStringList m_bCore;
    QStringList m_bVariant;
    QStringList m_bExtraFlags;

    QStringList m_upMaxSizeKb;
    QStringList m_upMaxDataSizeKb;
    QStringList m_freqHz;

    // Not an Option
    QStringList m_NaO;

};

/**
 * @brief A class to help deal with boards.txt
 *
 */
class Board
{
public:
    static  Board& instance();

    QMap<QString, BoardInfo> m_boards;

    QStringList m_boardList;
    QStringList m_boardNameList;
    void update();
    QString getIdFromName(QString _name);

private:
    Board& operator = (Board& other) = delete;
    Board(const Board& other) = delete;
    Board();
    ~Board();

    void load();
    QString Freq2FreqHz(QString freq);

    bool m_arduinoFolderFail;
    bool m_listed;
    QFile* m_arduinoFolder;
};
