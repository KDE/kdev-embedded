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

#include "firsttimewizard.h"

#include <QStandardPaths>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QProcess>
#include <QMessageBox>

#include <QPushButton>
#include <QThread>

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QStringList>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include <KConfigGroup>
#include <KTar>

#include "toolkit.h"

Q_LOGGING_CATEGORY(FtwIo, "Kdev.embedded.ftw.io");
Q_LOGGING_CATEGORY(FtwMsg, "kdev.embedded.ftw.msg");

using namespace KDevelop;

FirstTimeWizard::FirstTimeWizard(QWidget *parent) :
    QWizard(parent),
    m_mDownloadManager(new QNetworkAccessManager),
    m_downloadFinished(false),
    m_installFinished(false)
{
    setupUi(this);

    downloadStatusLabel->clear();
    installStatusLabel->clear();
    urlLabel->setText(urlLabel->text().arg(QString("mailto:%1").arg("patrickelectric@gmail.com")));
    projectLabel->setText(projectLabel->text().arg(i18n("Embedded plugin")).arg("Patrick J. Pereira"));

    existingInstallButton->setText(existingInstallButton->text().arg(ARDUINO_SDK_MIN_VERSION_NAME));
    automaticInstallButton->setText(automaticInstallButton->text().arg(ARDUINO_SDK_VERSION_NAME));


    // Download mode is default
    automaticInstallButton->setChecked(true);
    // Arduino path
    getArduinoPath();
    // Sketchbook path
    getSketchbookPath();

    //TODO support others OS
    QString mDownloadOs = "Linux";
    downloadLabel->setText(downloadLabel->text().arg(ARDUINO_SDK_VERSION_NAME).arg(mDownloadOs));

    connect(arduinoPathButton, &QToolButton::clicked, this, &FirstTimeWizard::chooseArduinoPath);
    connect(sketchbookPathButton, &QToolButton::clicked, this, &FirstTimeWizard::chooseSketchbookPath);
    connect(this, &QWizard::currentIdChanged, this, &FirstTimeWizard::validateCurrentId);
    connect(button(QWizard::CancelButton), &QAbstractButton::clicked, this, &FirstTimeWizard::cancelButtonClicked);
}

bool FirstTimeWizard::validateCurrentPage()
{
    switch (currentId())
    {
    case 0:
        if (existingInstallButton->isChecked() && !Toolkit::isValidArduinoPath(arduinoPathEdit->text()))
        {
            return false;
        }
        break;

    case 1:
    {
        if (m_downloadFinished && m_installFinished)
        {
            return true;
        }
        else
        {
            download();
        }

        return false;
        break;
    }

    case 2:
    {
        KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
        settings.writeEntry("arduinoFolder", arduinoPathEdit->text());
        settings.writeEntry("sketchbookFolder", sketchbookPathEdit->text());
    }
    break;

    default:
        break;
    }
    return true;
}

void FirstTimeWizard::download()
{
    button(QWizard::NextButton)->setEnabled(false);
    downloadProgressBar->setValue(0);
    // TODO update to generic links, create to linux and mac
    QNetworkRequest request(QUrl(QString("https://downloads.arduino.cc/arduino-%0-linux64.tar.xz").arg(ARDUINO_SDK_VERSION_NAME)));
    qCDebug(FtwIo) << "Download : " << QString("https://downloads.arduino.cc/arduino-%0-linux64.tar.xz").arg(ARDUINO_SDK_VERSION_NAME);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    m_reply = m_mDownloadManager->get(request);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &FirstTimeWizard::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &FirstTimeWizard::install);
    downloadStatusLabel->setText(i18n("Downloading..."));
}

void FirstTimeWizard::install()
{
    m_downloadFinished = m_reply->isOpen();
    qCDebug(FtwIo) << "at install m_downloadFinished" << m_downloadFinished;

    if (m_downloadFinished)
    {
        downloadStatusLabel->setText(i18n("Downloaded"));
    }
    else
    {
        downloadStatusLabel->setText(i18n("Download cancelled"));
        return;
    }

    // Extract the archive
    QTemporaryFile archive;
    bool extractSuccess = archive.open();
    QString destinationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir destinationDir(destinationPath);

    if (!extractSuccess)
    {
        qCDebug(FtwIo) << "Cant open file" << archive.fileName();
    }

    if (!destinationDir.exists())
    {
        qCDebug(FtwIo) << "Destination directory already exists at" << destinationPath;
        extractSuccess = extractSuccess && destinationDir.mkpath(".");
    }

    if (extractSuccess)
    {
        installStatusLabel->setText(i18n("Extracting..."));
        // Write the reply to the temporary file
        QByteArray buffer;
        // Create a buffer of 8kB
        static const int bufferSize = 8192;
        buffer.resize(bufferSize);
        qint64 readBytes = m_reply->read(buffer.data(), bufferSize);
        while (readBytes > 0)
        {
            archive.write(buffer.data(), readBytes);
            readBytes = m_reply->read(buffer.data(), bufferSize);
        }
        // FIXME use KFormat::formatByteSize here instead of hardcoding KB
        installStatusLabel->setText(i18n("Extracting... (%1 KB)", ((int)(readBytes >> 10))));
        archive.seek(0);

        // Call tar to extract
        KTar extract(archive.fileName(), "application/x-xz");
        extract.open(QIODevice::ReadOnly);
        extractSuccess = extract.directory()->copyTo(destinationPath, true);
        qCDebug(FtwIo) << "Downloaded file extracted with success ? :" << extractSuccess;
        qCDebug(FtwIo) << archive.fileName() << "extracted in" << destinationPath;

        installStatusLabel->setText(i18n("Extracted"));
        arduinoPathEdit->setText(destinationPath + "/arduino-" + ARDUINO_SDK_VERSION_NAME);
        m_installFinished = true;
    }
    this->button(QWizard::NextButton)->setEnabled(true);
}

void FirstTimeWizard::cancelButtonClicked(bool state)
{
    Q_UNUSED(state);
    qCDebug(FtwIo) << "CancelButton clicked";
    m_reply->abort();
}

void FirstTimeWizard::validateCurrentId(int id)
{
    if (id == 1 && !existingInstallButton->isChecked())
    {
        download();
    }
}

int FirstTimeWizard::nextId() const
{
    if (currentId() == 0 && existingInstallButton->isChecked())
    {
        return 2;
    }

    return QWizard::nextId();
}

QString FirstTimeWizard::getArduinoPath()
{
    // Find Arduino path
#ifdef Q_OS_DARWIN
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#else
    QString applicationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    // Paths to search for an existing installation
    static QStringList defaultArduinoPaths = QStringList()
            << QDir(applicationPath).filePath(QString("arduino-") + QString(ARDUINO_SDK_VERSION_NAME))
            << QDir(applicationPath).filePath("arduino")
            << QString("/usr/local/share/arduino-") + ARDUINO_SDK_VERSION_NAME
            << QString("/usr/local/share/arduino")
            << QString("/usr/share/arduino-") + ARDUINO_SDK_VERSION_NAME
            << QString("/usr/share/arduino");
#endif

    foreach (const auto& path, defaultArduinoPaths)
    {
        if (Toolkit::isValidArduinoPath(path))
        {
            qCDebug(FtwIo) << "Valid Arduino path at" << path;
            arduinoPathEdit->setText(path);
            existingInstallButton->setChecked(true);
            return path;
        }
    }
    qCDebug(FtwIo) << "No valid Arduino path";
    return QString();
}

QString FirstTimeWizard::getSketchbookPath()
{
    // Find Sketchbook path
    QDir sketchbookPath;
#ifdef Q_OS_DARWIN
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#else
    sketchbookPath = QDir(QDir::homePath()).filePath("sketchbook");
#endif

    if (sketchbookPath.exists())
    {
        sketchbookPathEdit->setText(sketchbookPath.absolutePath());
    }

    return QString();
}

void FirstTimeWizard::chooseArduinoPath()
{
    QString path;
    path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
    if (!path.isEmpty())
    {
        arduinoPathEdit->setText(path);
    }

}

void FirstTimeWizard::chooseSketchbookPath()
{
    QString path;
    path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
    if (!path.isEmpty())
    {
        sketchbookPathEdit->setText(path);
    }
}

void FirstTimeWizard::onDownloadProgress(qint64 received, qint64 total)
{
    int percent = 0;
    if (total)
    {
        percent = 100 * received / total;
    }

    qCDebug(FtwIo) << "Download in Progress" << percent << "%";
    qCDebug(FtwIo) << "Download in Progress" << received << "/" << total;

    // FIXME use KFormat::formatByteSize here instead of hardcoding KB
    downloadStatusLabel->setText(i18n("Downloading... (%1 KB / %2 KB)", ((int)(received >> 10)), ((int)(total >> 10))));
    downloadProgressBar->setValue(percent);
}

FirstTimeWizard::~FirstTimeWizard()
{
    delete m_mDownloadManager;
}
