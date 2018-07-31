/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Based onde the work Arduide Project of:
 *  Denis Martinez
 *  Martin Peres
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

// first time wizard configuration

#include <QWizard>
#include <QProcess>
#include <QLoggingCategory>

#include "ui_firsttimewizard.h"

#include "arduinoversion.h"

Q_DECLARE_LOGGING_CATEGORY(FtwIo)
Q_DECLARE_LOGGING_CATEGORY(FtwMsg)

class QNetworkAccessManager;
class QNetworkReply;
class KFormat;

class FirstTimeWizard : public QWizard, Ui::FirstTimeWizard
{
    Q_OBJECT

public:
    explicit FirstTimeWizard(QWidget *parent = nullptr);
    ~FirstTimeWizard();

    QNetworkAccessManager *m_mDownloadManager;
    QNetworkReply *m_reply;

    static QString arduinoDownloadUrl;
    static QString downloadExtensionUrl;
    static QString downloadArchUrl;
    static QString downloadOsUrl;

    bool m_downloadRunning;
    bool m_downloadFinished;
    bool m_installFinished;

    //get mcu list from avrdude
    QProcess *m_avrdudeProcess;
    KFormat  *m_format;

    /**
     * @brief Populate `Arduino Path` field with valid path to a Arduino install.
     *
     * If a setting exists, it is used, otherwise standard paths are verified
     * and if a valid one is found, it is used.
     */
    void fetchArduinoPath();
    /**
     * @brief Populate `Sketchbook Path` field with valid projects path.
     *
     * If a setting exists, it is used, otherwise standard paths are verified
     * and if a valid one is found, it is used.
     */
    void fetchSketchbookPath();
    QString downloadAndInstallArduino();
    bool validateCurrentPage();
    int  nextId() const;
    bool finish();

    void onDownloadProgress(qint64 received, qint64 total);
    void chooseArduinoPath();
    void validateCurrentId(int id);
    void cancelButtonClicked(bool state);
    void chooseSketchbookPath();
    void download();
    void install();

    void avrdudeStdout();
    void avrdudeStderr(int exitCode, QProcess::ExitStatus exitStatus);

};
