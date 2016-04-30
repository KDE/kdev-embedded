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
  mDownloadManager(new QNetworkAccessManager),
  downloadFinished(false),
  installFinished(false)
{
  setupUi(this);

  downloadStatusLabel->clear();
  installStatusLabel->clear();
  urlLabel->setText(urlLabel->text().arg(i18n("mailto:%1").arg("patrickelectric@gmail.com")));
  projectLabel->setText(projectLabel->text().arg(i18n("Embedded plugin")).arg("Patrick J. Pereira"));

  //TODO update to ARDUINO_SDK_MIN_VERSION_NAME
  existingInstallButton->setText(existingInstallButton->text().arg(ARDUINO_SDK_VERSION_NAME));
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
}

bool FirstTimeWizard::validateCurrentPage()
{
  switch (currentId())
  {
    case 0:
      if(existingInstallButton->isChecked() && !Toolkit::isValidArduinoPath(arduinoPathEdit->text()))
        return false;
      break;

    case 1:
    {
      if(downloadFinished && installFinished)
        return true;
      else
        download();

      return false;
      break;
    }

    case 2:
    {
      KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
      settings.writeEntry("arduinoFolder",arduinoPathEdit->text());
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
  downloadProgressBar->setValue(0);
  // TODO update to generic links, create to linux and mac
  QNetworkRequest request(QUrl("https://downloads.arduino.cc/arduino-1.6.8-linux64.tar.xz"));
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
  reply = mDownloadManager->get(request);
  connect(reply, &QNetworkReply::downloadProgress, this, &FirstTimeWizard::onDownloadProgress);
  connect(reply, &QNetworkReply::finished, this, &FirstTimeWizard::install);
  downloadStatusLabel->setText(i18n("Downloading..."));
}

void FirstTimeWizard::install()
{
  downloadFinished = true;
  downloadStatusLabel->setText(i18n("Downloaded !"));
  // Extract the archive
  QTemporaryFile archive;
  bool extractSuccess = archive.open();
  QString destinationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  QDir destinationDir(destinationPath);

  if (!extractSuccess)
    qCDebug(FtwIo) << "Cant open file" << archive.fileName();

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
      qint64 readBytes = reply->read(buffer.data(), bufferSize);
      while (readBytes > 0)
      {
          archive.write(buffer.data(), readBytes);
          readBytes = reply->read(buffer.data(), bufferSize);
      }
      installStatusLabel->setText(i18n("Extracting... ( %1KB )").arg(((int)(readBytes >> 10))));
      archive.seek(0);

      // Call tar to extract
      KTar extract(archive.fileName(), "application/x-xz");
      extract.open(QIODevice::ReadOnly);
      extractSuccess = extract.directory()->copyTo(destinationPath, true);
      qCDebug(FtwIo) << "Downloaded file extracted with success ? :" << extractSuccess;
      qCDebug(FtwIo) << archive.fileName() << "extracted in" << destinationPath;

      installStatusLabel->setText(i18n("Extracted !"));
      arduinoPathEdit->setText(destinationPath+"/arduino-"+ ARDUINO_SDK_VERSION_NAME);
      installFinished = true;
  }
}

int FirstTimeWizard::nextId() const
{
  if (currentId() == 0 && existingInstallButton->isChecked())
    return 2;
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

  foreach(const auto& path, defaultArduinoPaths)
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

  if(sketchbookPath.exists())
    sketchbookPathEdit->setText(sketchbookPath.absolutePath());

  return QString();
}

QString FirstTimeWizard::downloadAndInstallArduino()
{
  return QString();
}

bool FirstTimeWizard::finish()
{
  return true;
}

void FirstTimeWizard::chooseArduinoPath()
{
  QString path;
  path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
  if (!path.isEmpty())
    arduinoPathEdit->setText(path);

}

void FirstTimeWizard::chooseSketchbookPath()
{
  QString path;
  path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
  if (!path.isEmpty())
    sketchbookPathEdit->setText(path);
}

void FirstTimeWizard::onDownloadProgress(qint64 received, qint64 total)
{
    int percent = 0;
    if(total)
       percent = 100 * received / total;

    qCDebug(FtwIo) << "Download in Progress" << percent << "%";
    qCDebug(FtwIo) << "Download in Progress" << received << "/" << total;

    downloadStatusLabel->setText(i18n("Downloading... ( %1KB / %2KB )").arg((int)(received >> 10)).arg((int)(total >> 10)));
    downloadProgressBar->setValue(percent);
}

FirstTimeWizard::~FirstTimeWizard()
{
    delete mDownloadManager;
}
