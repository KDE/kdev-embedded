#include "embedded.h"

#include <QAction>
#include <QVariantList>

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

Q_LOGGING_CATEGORY(PLUGIN_EMBEDDED, "kdevplatform.plugins.embedded")


using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(EmbeddedFactory, "kdevembedded.json", registerPlugin<Embedded>(); )

Embedded::Embedded ( QObject* parent, const QVariantList& )
    : IPlugin ( QStringLiteral("kdevembedded"), parent )
{
    setXMLFile( QStringLiteral("kdevembedded.rc") );

    QAction* actionProject = actionCollection()->addAction( QStringLiteral("action_project") );
    actionProject->setText( i18n( "Arduino Setup" ) );
    actionCollection()->setDefaultShortcut(actionProject, i18n( "Alt+Shift+a" ));
    connect( actionProject, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    actionProject->setToolTip( i18n( "1 Line Tip" ) );
    actionProject->setWhatsThis( i18n( "Long Tip 3 lines") );
    actionProject->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );
  
    QAction* actionConfigureBoard = actionCollection()->addAction( QStringLiteral("action_board") );
    actionConfigureBoard->setText( i18n( "Board settings" ) );
    actionCollection()->setDefaultShortcut(actionConfigureBoard, i18n( "Alt+Shift+b" ));
    connect( actionConfigureBoard, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration2()) );
    actionConfigureBoard->setToolTip( i18n( "1 Line Tip" ) );
    actionConfigureBoard->setWhatsThis( i18n( "Long Tip 3 lines") );
    actionConfigureBoard->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );
}

void Embedded::documentDeclaration()
{
  embeddedWindow = new firstTimeWizard(ICore::self()->uiController()->activeMainWindow());
  embeddedWindow->show();
}

void Embedded::documentDeclaration2()
{
  arduinoBoard = new arduinoWindow(ICore::self()->uiController()->activeMainWindow());
  arduinoBoard->show();
}

Embedded::~Embedded()
{
}

#include "embedded.moc"
