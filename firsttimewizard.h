#ifndef EMBEDDED_FIRSTTIMEWIZARD_H
#define EMBEDDED_FIRSTTIMEWIZARD_H

// first time wizard configuration

#include <QWizard>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(FtwIo);
Q_DECLARE_LOGGING_CATEGORY(FtwMsg);

#include "ui_firsttimewizard.h"

class QNetworkAccessManager;
class QNetworkReply;

class FirstTimeWizard : public QWizard, Ui::FirstTimeWizard
{
  Q_OBJECT

public:
  explicit FirstTimeWizard(QWidget *parent = NULL);
  ~FirstTimeWizard();

  QNetworkAccessManager *mDownloadManager;
  QNetworkReply *reply;

  bool downloadFinished;
  bool installFinished;

  QString getArduinoPath();
  QString getSketchbookPath();
  QString downloadAndInstallArduino();
  bool validateCurrentPage();
  int  nextId() const;
  bool finish();

  void onDownloadProgress(qint64 received, qint64 total);
  void chooseArduinoPath();
  void chooseSketchbookPath();
  void download();
  void install();

};

#endif // EMBEDDED_FIRSTTIMEWIZARD_H
