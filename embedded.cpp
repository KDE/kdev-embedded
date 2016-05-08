#include "embedded.h"

#include <QAction>
#include <QVariantList>
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <KParts/MainWindow>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
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
    connect(actionProject, &QAction::triggered, this, &Embedded::firstTimeWizardEvent);

    QAction* actionConfigureBoard = actionCollection()->addAction(QStringLiteral("action_board"));
    actionConfigureBoard->setText(i18n("Board settings"));
    actionCollection()->setDefaultShortcut(actionConfigureBoard, i18n("Alt+Shift+b"));
    actionConfigureBoard->setToolTip(i18n("Configure board and interface configurations for embedded systems."));
    actionConfigureBoard->setWhatsThis(i18n("Project and upload manager for embedded systems."));
    actionConfigureBoard->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    connect(actionConfigureBoard, &QAction::triggered, this, &Embedded::boardSettingsEvent);
}

void Embedded::firstTimeWizardEvent()
{
    m_embeddedWindow = new FirstTimeWizard(ICore::self()->uiController()->activeMainWindow());
    m_embeddedWindow->show();
}

void Embedded::boardSettingsEvent()
{
    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    if(!settings.readEntry("arduinoFolder","").isEmpty())
    {
        m_arduinoBoard = new ArduinoWindow(ICore::self()->uiController()->activeMainWindow());
        m_arduinoBoard->show();
    }
    else
    {
        QMessageBox::warning(0, i18n("kdev-embedded"), i18n("Please, run the first time wizard."));
        firstTimeWizardEvent();
    }

}

Embedded::~Embedded()
{
    delete m_embeddedWindow;
    delete m_arduinoBoard;
}

#include "embedded.moc"
