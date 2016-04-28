#include "embedded.h"

#include <QAction>
#include <QVariantList>
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KActionCollection>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <QApplication>
#include <QStandardPaths>
#include <KParts/MainWindow>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include "arduinowindow.h"
#include "firsttimewizard.h"

Q_LOGGING_CATEGORY(PLUGIN_EMBEDDED, "kdevplatform.plugins.embedded");

using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(EmbeddedFactory, "kdevembedded.json", registerPlugin<Embedded>(); )

Embedded::Embedded (QObject* parent, const QVariantList&)
    : IPlugin (QStringLiteral("kdevembedded"), parent)
{
    setXMLFile(QStringLiteral("kdevembedded.rc"));

    QAction* actionProject = actionCollection()->addAction(QStringLiteral("action_project"));
    actionProject->setText(i18n("Arduino Setup"));
    actionProject->setToolTip(i18n("Configure Arduino Toolkit."));
    actionProject->setWhatsThis(i18n("Toolkit manager for Arduino programs,"));
    actionProject->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    connect(actionProject, &QAction::triggered, this, &Embedded::documentDeclaration);

    QAction* actionConfigureBoard = actionCollection()->addAction(QStringLiteral("action_board"));
    actionConfigureBoard->setText(i18n("Board settings"));
    actionCollection()->setDefaultShortcut(actionConfigureBoard, i18n("Alt+Shift+b"));
    actionConfigureBoard->setToolTip(i18n("Configure board and interface configurations for embedded systems."));
    actionConfigureBoard->setWhatsThis(i18n("Project and upload manager for embedded systems."));
    actionConfigureBoard->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    connect(actionConfigureBoard, &QAction::triggered, this, &Embedded::documentDeclaration2);
}

void Embedded::documentDeclaration()
{
  embeddedWindow = new FirstTimeWizard(ICore::self()->uiController()->activeMainWindow());
  embeddedWindow->show();
}

void Embedded::documentDeclaration2()
{
  arduinoBoard = new ArduinoWindow(ICore::self()->uiController()->activeMainWindow());
  arduinoBoard->show();
}

Embedded::~Embedded()
{
    delete embeddedWindow;
    delete arduinoBoard;
}

#include "embedded.moc"
