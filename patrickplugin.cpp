/*
 * This file is part of KDevelop
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "patrickplugin.h"

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

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/interfaces/ilanguagesupport.h>

Q_LOGGING_CATEGORY(PLUGIN_patrick, "kdevplatform.plugins.patrick")


using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(patrickPluginFactory, "kdevpatrick.json", registerPlugin<patrickPlugin>(); )

patrickPlugin::patrickPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( QStringLiteral("kdevpatrick"), parent )
{
    setXMLFile( QStringLiteral("kdevpatrick.rc") );

    QAction* action = actionCollection()->addAction( QStringLiteral("action_project") );
    action->setText( i18n( "Project" ) );
    actionCollection()->setDefaultShortcut(action, i18n( "Alt+Shift+a" ));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action->setToolTip( i18n( "1 Line Tip" ) );
    // i18n: translate title same as the action name
    action->setWhatsThis( i18n( "Long Tip 3 lines") );
    action->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );


    /*
    QAction* action2 = actionCollection()->addAction( QStringLiteral("action_upload") );
    action2->setText( i18n( "Upload" ) );
    actionCollection()->setDefaultShortcut(action2, i18n( "Alt+Shift+u" ));
    connect( action2, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action2->setToolTip( i18n( "1 Line Tip" ) );
    // i18n: translate title same as the action name
    action2->setWhatsThis( i18n( "Long Tip 3 lines") );
    action2->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );
    */
    QMenu *menu1 = new QMenu;
    menu1->setTitle("potato3");

    QAction* actionn;
    actionn->setData("HAHAHA");
    actionn->setCheckable(true);
    QMenu *menu2 = new QMenu;
    menu2->setTitle("potato2");
    menu2->addAction(actionn);
    menu1->addMenu(menu2);

    menu1->addAction(actionCollection()->addAction(QStringLiteral("action_upload")));


    QAction* action3 = actionCollection()->addAction( QStringLiteral("action_config") );
    action3->setText( i18n( "Settings" ) );
    actionCollection()->setDefaultShortcut(action3, i18n( "Alt+Shift+c" ));
    connect( action3, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action3->setToolTip( i18n( "1 Line Tip" ) );
    // i18n: translate title same as the action name
    action3->setWhatsThis( i18n( "Long Tip 3 lines") );
    action3->setIcon( QIcon::fromTheme( QStringLiteral("project-development-new-template") ) );
}

void patrickPlugin::documentDeclaration()
{
  patrickwindow = new arduinowindow();
  qDebug() << "potato8";;
  patrickwindow->show();
  qDebug() << "potato";
}

patrickPlugin::~patrickPlugin()
{
}

#include "patrickplugin.moc"
