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

    QAction* action = actionCollection()->addAction( QStringLiteral("action_project") );
    action->setText( i18n( "Arduino Setup" ) );
    actionCollection()->setDefaultShortcut(action, i18n( "Alt+Shift+a" ));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action->setToolTip( i18n( "1 Line Tip" ) );
    // i18n: translate title same as the action name
    action->setWhatsThis( i18n( "Long Tip 3 lines") );
    action->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );
}

void Embedded::documentDeclaration()
{
  embeddedWindow = new firstTimeWizard(ICore::self()->uiController()->activeMainWindow());
  embeddedWindow->show();
}

Embedded::~Embedded()
{
}

#include "embedded.moc"
