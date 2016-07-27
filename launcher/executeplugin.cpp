/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Based onde the work of:
 *  Hamish Rodda <rodda@kde.org>
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

#include "executeplugin.h"

#include <QDebug>

#include <KConfigGroup>
#include <KJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KShell>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/environmentgrouplist.h>

#include "debug.h"
#include "toolkit.h"
#include "embeddedlauncher.h"
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <util/kdevstringhandler.h>

QString ExecutePlugin::_nativeAppConfigTypeId = i18n("Embedded Application");
QString ExecutePlugin::workingDirEntry = i18n("Working Directory");
QString ExecutePlugin::executableEntry = i18n("Executable");
QString ExecutePlugin::argumentsEntry = i18n("Arguments");
QString ExecutePlugin::isExecutableEntry = i18n("isExecutable");
QString ExecutePlugin::environmentGroupEntry = i18n("EnvironmentGroup");
QString ExecutePlugin::useTerminalEntry = i18n("Use External Terminal");
QString ExecutePlugin::commandEntry = i18n("Command");
QString ExecutePlugin::boardEntry = i18n("Board Index");
QString ExecutePlugin::mcuEntry = i18n("mcu Index");
QString ExecutePlugin::userIdToRunEntry = i18n("User Id to Run");
QString ExecutePlugin::dependencyActionEntry = i18n("Dependency Action");
QString ExecutePlugin::projectTargetEntry = i18n("Project Target");
QString ExecutePlugin::arduinoEntry = i18n("Arduino Entry");
QString ExecutePlugin::launcherIndexEntry = i18n("Embedded Launcher Index");
QString ExecutePlugin::openocdArgEntry = i18n("Openocd Arg Entry");
QString ExecutePlugin::openocdWorkEntry = i18n("Openocd Work Entry");
QString ExecutePlugin::openocdCommEntry = i18n("Openocd Command Entry");

using namespace KDevelop;

Q_LOGGING_CATEGORY(EpMsg, "Kdev.embedded.ep.msg")
K_PLUGIN_FACTORY_WITH_JSON(KDevExecuteFactory, "kdevembedded-launcher.json", registerPlugin<ExecutePlugin>();)

ExecutePlugin::ExecutePlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevembedded-launcher"), parent)
{
    KDEV_USE_EXTENSION_INTERFACE(IExecutePlugin)
    m_configType = new NativeAppConfigType();
    m_configType->addLauncher(new EmbeddedLauncher());
    qCDebug(EpMsg) << "adding native app launch config";
    core()->runController()->addConfigurationType(m_configType);
}

ExecutePlugin::~ExecutePlugin()
{
    delete m_configType;
}

void ExecutePlugin::unload()
{
    core()->runController()->removeConfigurationType(m_configType);
    delete m_configType;
    m_configType = 0;
}

QStringList ExecutePlugin::arguments(KDevelop::ILaunchConfiguration* cfg, QString& err_) const
{
    qCDebug(EpMsg) << "ExecutePlugin::arguments";
    qCDebug(EpMsg) << "name" << cfg->name();
    qCDebug(EpMsg) << "entryMap" << cfg->config().entryMap();
    qCDebug(EpMsg) << "groupList" << cfg->config().groupList();
    qCDebug(EpMsg) << "keyList" << cfg->config().keyList();

    if (!cfg)
    {
        qCDebug(EpMsg) << "ExecutePlugin::arguments" << "!cfg";
        return QStringList();
    }

    KShell::Errors err;
    uint launcherIndex = cfg->config().readEntry(ExecutePlugin::launcherIndexEntry, 0);

    QStringList args;
    switch (launcherIndex)
    {
        case index::arduino:
            args = KShell::splitArgs(cfg->config().readEntry(ExecutePlugin::argumentsEntry, ""), KShell::TildeExpand | KShell::AbortOnMeta, &err);
        break;

        case index::openocd:
            args = KShell::splitArgs(cfg->config().readEntry(ExecutePlugin::openocdArgEntry, ""), KShell::TildeExpand | KShell::AbortOnMeta, &err);
        break;
    }

    if (err != KShell::NoError)
    {

        if (err == KShell::BadQuoting)
        {
            err_ = i18n("There is a quoting error in the arguments for "
                        "the launch configuration '%1'. Aborting start.", cfg->name());
        }
        else
        {
            err_ = i18n("A shell meta character was included in the "
                        "arguments for the launch configuration '%1', "
                        "this is not supported currently. Aborting start.", cfg->name());
        }
        args = QStringList();
        qWarning() << "Launch Configuration:" << cfg->name() << "arguments have meta characters";
    }

    switch (launcherIndex)
    {
        case index::arduino:
        {
            QStringList arduinoConfig = cfg->config().readEntry(ExecutePlugin::arduinoEntry, QStringList());

            for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
            {
                qCDebug(EpMsg) << *it;
                if (!arduinoConfig.empty())
                {
                    it->replace(QLatin1String("%mcu"), KShell::quoteArg(arduinoConfig[1]));
                    it->replace(QLatin1String("%baud"), KShell::quoteArg(arduinoConfig[2]));
                    it->replace(QLatin1String("%interface"), KShell::quoteArg(arduinoConfig[3]));
                    it->replace(QLatin1String("%hex"), KShell::quoteArg(arduinoConfig[4]));
                    it->replace(QLatin1String("%avrdudeconf"), KShell::quoteArg(arduinoConfig[5]));
                }
            }
        }

        case index::openocd:
        {
            QString binary = cfg->config().readEntry(ExecutePlugin::executableEntry, QString());

            for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
            {
                qCDebug(EpMsg) << *it;
                if (!binary.isEmpty())
                {
                    it->replace(QLatin1String("%hex"), binary);
                }
            }
        }

    }

    qCDebug(EpMsg) << "ExecutePlugin::arguments" << args;
    return args;
}


KJob* ExecutePlugin::dependencyJob(KDevelop::ILaunchConfiguration* cfg) const
{
    Q_UNUSED(cfg)
    return nullptr;
}


QString ExecutePlugin::environmentGroup(KDevelop::ILaunchConfiguration* cfg) const
{
    if (!cfg)
    {
        return QString();
    }

    return cfg->config().readEntry(ExecutePlugin::environmentGroupEntry, "");
}


QUrl ExecutePlugin::executable(KDevelop::ILaunchConfiguration* cfg, QString& err) const
{
    Q_UNUSED(err)
    if (!cfg)
    {
        qCDebug(EpMsg) << "ExecutePlugin::executable" << "!cfg";
        return QUrl();
    }

    QString exe;
    uint launcherIndex = cfg->config().readEntry(ExecutePlugin::launcherIndexEntry, 0);
    qCDebug(EpMsg) << "ExecutePlugin::executable" << "launcherIndex" << launcherIndex;
    switch (launcherIndex)
    {
        case index::arduino:
            exe = cfg->config().readEntry(ExecutePlugin::commandEntry, QString());
        break;
        case index::openocd:
            exe = cfg->config().readEntry(ExecutePlugin::openocdCommEntry, QString());
        break;
    }
    qCDebug(EpMsg) << "ExecutePlugin::executable" << "exe" << exe;
    if (!exe.isEmpty())
    {
        if (exe.contains(QLatin1String("%avrdude")))
        {
            exe.replace(QLatin1String("%avrdude"), Toolkit::instance().getAvrdudeFile());
        }

        if  (exe.contains(QLatin1String("%openocd")))
        {
            exe.replace(QLatin1String("%openocd"), Toolkit::instance().getOpenocdFile());
        }

    }

    return QUrl::fromLocalFile(exe);
}


bool ExecutePlugin::useTerminal(KDevelop::ILaunchConfiguration* cfg) const
{
    if (!cfg)
    {
        return false;
    }

    return cfg->config().readEntry(ExecutePlugin::useTerminalEntry, false);
}


QString ExecutePlugin::terminal(KDevelop::ILaunchConfiguration* cfg) const
{
    if (!cfg)
    {
        return QString();
    }

    return cfg->config().readEntry(ExecutePlugin::commandEntry, QString());
}


QUrl ExecutePlugin::workingDirectory(KDevelop::ILaunchConfiguration* cfg) const
{
    if (!cfg)
    {
        return QUrl();
    }

    uint launcherIndex = cfg->config().readEntry(ExecutePlugin::launcherIndexEntry, 0);

    switch (launcherIndex)
    {
        case index::arduino:
            return cfg->config().readEntry(ExecutePlugin::openocdWorkEntry, QUrl());
        break;

        case index::openocd:
            return cfg->config().readEntry(ExecutePlugin::workingDirEntry, QUrl());
        break;
    }

    return QUrl();
}


QString ExecutePlugin::nativeAppConfigTypeId() const
{
    return _nativeAppConfigTypeId;
}


#include "executeplugin.moc"
