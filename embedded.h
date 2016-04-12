#ifndef KDEVPLATFORM_PLUGIN_EMBEDDED_H
#define KDEVPLATFORM_PLUGIN_EMBEDDED_H

// main file

#include <interfaces/iplugin.h>
#include <interfaces/contextmenuextension.h>

#include <QtCore/QVariant>
#include <QLoggingCategory>
#include <QMenu>

#include "arduinowindow.h"
#include "firsttimewizard.h"

Q_DECLARE_LOGGING_CATEGORY(PLUGIN_EMBEDDED)

class Embedded : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Embedded( QObject *parent, const QVariantList & = QVariantList() );
    ~Embedded() override;

    firstTimeWizard* embeddedWindow;

private slots:
    void documentDeclaration();
};


#endif // KDEVPLATFORM_PLUGIN_EMBEDDED_PLUGIN_H
