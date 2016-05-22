#ifndef EMBEDDED_FIRSTTIMEWIZARD_H
#define EMBEDDED_FIRSTTIMEWIZARD_H

// first time wizard configuration

#include <QWizard>
#include <QLoggingCategory>

#include "ui_firsttimewizard.h"

#define ARDUINO_SDK_VERSION "168"
#define ARDUINO_SDK_VERSION_NAME "1.6.8"
#define ARDUINO_SDK_MIN_VERSION_NAME "1.6.8"

Q_DECLARE_LOGGING_CATEGORY(FtwIo);
Q_DECLARE_LOGGING_CATEGORY(FtwMsg);

class QNetworkAccessManager;
class QNetworkReply;

class FirstTimeWizard : public QWizard, Ui::FirstTimeWizard
{
    Q_OBJECT

public:
    explicit FirstTimeWizard(QWidget *parent = NULL);
    ~FirstTimeWizard();

    QNetworkAccessManager *m_mDownloadManager;
    QNetworkReply *m_reply;

    bool m_downloadFinished;
    bool m_installFinished;

    QString getArduinoPath();
    QString getSketchbookPath();
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

};

#endif // EMBEDDED_FIRSTTIMEWIZARD_H
