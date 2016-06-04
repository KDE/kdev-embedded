#pragma once

#include <interfaces/iplugin.h>

#include <QtCore/QVariant>
#include <QLoggingCategory>

class ArduinoWindow;
class FirstTimeWizard;

Q_DECLARE_LOGGING_CATEGORY(PLUGIN_EMBEDDED);

class Embedded : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Embedded(QObject *parent, const QVariantList & = QVariantList());
    ~Embedded() override;

    FirstTimeWizard* m_embeddedWindow = nullptr;
    ArduinoWindow* m_arduinoBoard = nullptr;

    void firstTimeWizardEvent();
    void boardSettingsEvent();
};
