#ifndef FIRSTTIMEWIZARD_H
#define FIRSTTIMEWIZARD_H

// first time wizard configuration

#include <QWizard>

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

#endif // FIRSTTIMEWIZARD_H
