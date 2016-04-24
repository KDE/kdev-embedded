#include "firsttimewizard.h"

#include <QStandardPaths>

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

//#include <QtCore/QSet>
#include <interfaces/isession.h>
#include <interfaces/icore.h>
#include <KConfigGroup>

#include "toolkit.h"

using namespace KDevelop;

firstTimeWizard::firstTimeWizard(QWidget *parent) :
  QWizard(parent)
{
  downloadFinished=installFinished=false;
  mDownloadManager = new QNetworkAccessManager;

  setupUi(this);

  downloadStatusLabel->setText("");
  installStatusLabel->setText("");
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

  connect(arduinoPathButton, &QToolButton::clicked, this, &firstTimeWizard::chooseArduinoPath);
  connect(sketchbookPathButton, &QToolButton::clicked, this, &firstTimeWizard::chooseSketchbookPath);
}

bool firstTimeWizard::validateCurrentPage()
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

void firstTimeWizard::download()
{
  downloadProgressBar->setValue(0);
  // TODO update to generic links, create to linux and mac
  QNetworkRequest request(QUrl("https://downloads.arduino.cc/arduino-1.6.8-linux64.tar.xz"));
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
  reply = mDownloadManager->get(request);
  connect(reply, &QNetworkReply::downloadProgress, this, &firstTimeWizard::onDownloadProgress);
  connect(reply, &QNetworkReply::finished, this, &firstTimeWizard::install);
  downloadStatusLabel->setText(i18n("Downloading..."));
}

void firstTimeWizard::install()
{
  downloadFinished = true;
  downloadStatusLabel->setText(i18n("Downloaded !"));
  // extract the archive
  QTemporaryFile archive("arduino");
  bool extractSuccess = archive.open();
  QString destinationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  QDir destinationDir(destinationPath);

  if (!destinationDir.exists())
    extractSuccess = extractSuccess && destinationDir.mkpath(".");

  if (extractSuccess)
  {
    installStatusLabel->setText(i18n("Extracting..."));
      // write the reply to the temporary file
      QByteArray buffer;
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

      // call tar to extract
      QString extractCommand;
      QStringList extractArgs = QStringList();
      extractCommand = "tar";
      if (QString(ARDUINO_SDK_VERSION_NAME) >= ARDUINO_SDK_MIN_VERSION_NAME)
          extractArgs = QStringList()
              << "-x" << "-J" << "-f" << archive.fileName()
              << "-C" << destinationPath;

      QFutureWatcher<int> extractWatcher;
      QFuture<int> extractFuture = QtConcurrent::run(&QProcess::execute, extractCommand, extractArgs);
      extractWatcher.setFuture(extractFuture);
      extractSuccess = extractFuture.result() == 0;
      installStatusLabel->setText(i18n("Extracted !"));
      arduinoPathEdit->setText(destinationPath+"/arduino-"+ ARDUINO_SDK_VERSION_NAME);
      installFinished = true;
  }
}

int firstTimeWizard::nextId() const
{
  if (currentId() == 0 && existingInstallButton->isChecked())
    return 2;
  return QWizard::nextId();
}

QString firstTimeWizard::getArduinoPath()
{
  // paths to search for an existing installation
  QStringList defaultArduinoPaths;

  // Find Arduino path
#ifdef Q_OS_DARWIN
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#else
  QString applicationPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

  defaultArduinoPaths
    << QDir(applicationPath).filePath(QString("arduino-") + QString(ARDUINO_SDK_VERSION_NAME))
    << QDir(applicationPath).filePath("arduino")
    << QString("/usr/local/share/arduino-") + ARDUINO_SDK_VERSION_NAME
    << QString("/usr/local/share/arduino")
    << QString("/usr/share/arduino-") + ARDUINO_SDK_VERSION_NAME
    << QString("/usr/share/arduino");
#endif

  //change to ->caontins ?
  foreach(const QString &path, defaultArduinoPaths)
  {
    if (Toolkit::isValidArduinoPath(path))
    {
      arduinoPathEdit->setText(path);
      existingInstallButton->setChecked(true);
      return path;
    }
  }
  return "";
}

QString firstTimeWizard::getSketchbookPath()
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

  return "";
}

QString firstTimeWizard::downloadAndInstallArduino()
{
  return "";
}

bool firstTimeWizard::finish()
{
  return true;
}

void firstTimeWizard::chooseArduinoPath()
{
  QString path;
#ifdef Q_OS_DARWIN
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#else
  path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
#endif

    if (!path.isEmpty())
      arduinoPathEdit->setText(path);

}

void firstTimeWizard::chooseSketchbookPath()
{
  QString path;
#ifdef Q_OS_DARWIN
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#else
  path = QFileDialog::getExistingDirectory(this, i18n("Find Files"), QDir::currentPath());
#endif

  if (!path.isEmpty())
    sketchbookPathEdit->setText(path);
}

void firstTimeWizard::onDownloadProgress(qint64 received, qint64 total)
{
    int percent = 0;
    if(total)
       percent = 100 * received / total;

    downloadStatusLabel->setText(i18n("Downloading... ( %1KB / %2KB )").arg((int)(received >> 10)).arg((int)(total >> 10)));
    downloadProgressBar->setValue(percent);
}

firstTimeWizard::~firstTimeWizard()
{
}
