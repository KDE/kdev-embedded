#ifndef FIRSTTIMEWIZARD_H
#define FIRSTTIMEWIZARD_H

// first time wizard configuration

#include <QNetworkAccessManager>
#include <QWizard>
#include <QSettings>

#include "ui_firsttimewizard.h"

class firstTimeWizard : public QWizard, Ui::firstTimeWizard
{
  Q_OBJECT

public:
  explicit firstTimeWizard(QWidget *parent = NULL);
  ~firstTimeWizard();

  QNetworkAccessManager *mDownloadManager;
  QNetworkReply *reply;
  QSettings *settings;

  bool downloadFinished;
  bool installFinished;

  QString getArduinoPath();
  QString getSketchbookPath();
  QString downloadAndInstallArduino();
  bool validateCurrentPage();
  int nextId() const;
  bool finish();
private slots:
  void onDownloadProgress(qint64 received, qint64 total);
  void chooseArduinoPath();
  void chooseSketchbookPath();
  void download();
  void install();

};

#endif // FIRSTTIMEWIZARD_H
