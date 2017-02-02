/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
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

#include "firsttimewizard.h"

Q_LOGGING_CATEGORY(PLUGIN_EMBEDDED, "kdevplatform.plugins.embedded")

using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(EmbeddedFactory, "kdevembedded.json", registerPlugin<Embedded>();)

Embedded::Embedded(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("kdevembedded"), parent)
{
    setXMLFile(QStringLiteral("kdevembedded.rc"));

    QAction* actionProject = actionCollection()->addAction(QStringLiteral("action_project"));
    actionProject->setText(i18n("Arduino Setup"));
    actionProject->setToolTip(i18n("Configure Arduino Toolkit."));
    actionProject->setWhatsThis(i18n("Toolkit manager for Arduino programs."));
    actionProject->setIcon(QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    connect(actionProject, &QAction::triggered, this, &Embedded::firstTimeWizardEvent);
}

void Embedded::firstTimeWizardEvent()
{
    FirstTimeWizard *embeddedWindow = new FirstTimeWizard(ICore::self()->uiController()->activeMainWindow());
    embeddedWindow->setAttribute(Qt::WA_DeleteOnClose);
    embeddedWindow->show();
}

Embedded::~Embedded()
{
}

#include "embedded.moc"
