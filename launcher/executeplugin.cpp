/*
  * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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
#include <interfaces/iruncontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/environmentgrouplist.h>

#include "nativeappconfig.h"
#include "debug.h"
#include <project/projectmodel.h>
#include <project/builderjob.h>
#include <util/kdevstringhandler.h>

QString ExecutePlugin2::_nativeAppConfigTypeId = QStringLiteral("Native Application2");
QString ExecutePlugin2::workingDirEntry = QStringLiteral("Working Directory");
QString ExecutePlugin2::executableEntry = QStringLiteral("Executable");
QString ExecutePlugin2::argumentsEntry = QStringLiteral("Arguments");
QString ExecutePlugin2::isExecutableEntry = QStringLiteral("isExecutable");
QString ExecutePlugin2::dependencyEntry = QStringLiteral("Dependencies");
QString ExecutePlugin2::environmentGroupEntry = QStringLiteral("EnvironmentGroup");
QString ExecutePlugin2::useTerminalEntry = QStringLiteral("Use External Terminal");
QString ExecutePlugin2::terminalEntry = QStringLiteral("External Terminal");
QString ExecutePlugin2::userIdToRunEntry = QStringLiteral("User Id to Run");
QString ExecutePlugin2::dependencyActionEntry = QStringLiteral("Dependency Action");
QString ExecutePlugin2::projectTargetEntry = QStringLiteral("Project Target");

using namespace KDevelop;

Q_LOGGING_CATEGORY(PLUGIN_EXECUTE, "kdevplatform.plugins.execute2")
K_PLUGIN_FACTORY_WITH_JSON(KDevExecuteFactory, "kdevexecute2.json", registerPlugin<ExecutePlugin2>();)

ExecutePlugin2::ExecutePlugin2(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevexecute2"), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IExecutePlugin2)
    m_configType = new NativeAppConfigType();
    m_configType->addLauncher( new NativeAppLauncher() );
    qCDebug(PLUGIN_EXECUTE) << "adding native app launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecutePlugin2::~ExecutePlugin2()
{
}

void ExecutePlugin2::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = 0;
}

QStringList ExecutePlugin2::arguments( KDevelop::ILaunchConfiguration* cfg, QString& err_ ) const
{

    if( !cfg )
    {
        return QStringList();
    }

    KShell::Errors err;
    QStringList args = KShell::splitArgs( cfg->config().readEntry( ExecutePlugin2::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
    if( err != KShell::NoError )
    {

        if( err == KShell::BadQuoting )
        {
            err_ = i18n("There is a quoting error in the arguments for "
            "the launch configuration '%1'. Aborting start.", cfg->name() );
        } else
        {
            err_ = i18n("A shell meta character was included in the "
            "arguments for the launch configuration '%1', "
            "this is not supported currently. Aborting start.", cfg->name() );
        }
        args = QStringList();
        qWarning() << "Launch Configuration:" << cfg->name() << "arguments have meta characters";
    }
    return args;
}


KJob* ExecutePlugin2::dependencyJob( KDevelop::ILaunchConfiguration* cfg ) const
{
    QVariantList deps = KDevelop::stringToQVariant( cfg->config().readEntry( dependencyEntry, QString() ) ).toList();
    QString depAction = cfg->config().readEntry( dependencyActionEntry, "Nothing" );
    if( depAction != QLatin1String("Nothing") && !deps.isEmpty() )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QList<KDevelop::ProjectBaseItem*> items;
        foreach( const QVariant& dep, deps )
        {
            KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex( dep.toStringList() ) );
            if( item )
            {
                items << item;
            }
            else
            {
                KMessageBox::error(core()->uiController()->activeMainWindow(),
                                   i18n("Couldn't resolve the dependency: %1", dep.toString()));
            }
        }
        KDevelop::BuilderJob* job = new KDevelop::BuilderJob();
        if( depAction == QLatin1String("Build") )
        {
            job->addItems( KDevelop::BuilderJob::Build, items );
        } else if( depAction == QLatin1String("Install") )
        {
            job->addItems( KDevelop::BuilderJob::Install, items );
        }
        job->updateJobName();
        return job;
    }
    return 0;
}


QString ExecutePlugin2::environmentGroup( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QLatin1String("");
    }

    return cfg->config().readEntry( ExecutePlugin2::environmentGroupEntry, "" );
}


QUrl ExecutePlugin2::executable( KDevelop::ILaunchConfiguration* cfg, QString& err ) const
{
    QUrl executable;
    if( !cfg )
    {
        return executable;
    }
    KConfigGroup grp = cfg->config();
    if( grp.readEntry(ExecutePlugin2::isExecutableEntry, false ) )
    {
        executable = grp.readEntry( ExecutePlugin2::executableEntry, QUrl() );
    } else
    {
        QStringList prjitem = grp.readEntry( ExecutePlugin2::projectTargetEntry, QStringList() );
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        KDevelop::ProjectBaseItem* item = model->itemFromIndex( model->pathToIndex(prjitem) );
        if( item && item->executable() )
        {
            // TODO: Need an option in the gui to choose between installed and builddir url here, currently cmake only supports builddir url
            executable = item->executable()->builtUrl();
        }
    }
    if( executable.isEmpty() )
    {
        err = i18n("No valid executable specified");
        qWarning() << "Launch Configuration:" << cfg->name() << "no valid executable set";
    } else
    {
        KShell::Errors err_;
        if( KShell::splitArgs( executable.toLocalFile(), KShell::TildeExpand | KShell::AbortOnMeta, &err_ ).isEmpty() || err_ != KShell::NoError )
        {
            executable = QUrl();
            if( err_ == KShell::BadQuoting )
            {
                err = i18n("There is a quoting error in the executable "
                "for the launch configuration '%1'. "
                "Aborting start.", cfg->name() );
            } else
            {
                err = i18n("A shell meta character was included in the "
                "executable for the launch configuration '%1', "
                "this is not supported currently. Aborting start.", cfg->name() );
            }
            qWarning() << "Launch Configuration:" << cfg->name() << "executable has meta characters";
        }
    }
    return executable;
}


bool ExecutePlugin2::useTerminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return false;
    }

    return cfg->config().readEntry( ExecutePlugin2::useTerminalEntry, false );
}


QString ExecutePlugin2::terminal( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QString();
    }

    return cfg->config().readEntry( ExecutePlugin2::terminalEntry, QString() );
}


QUrl ExecutePlugin2::workingDirectory( KDevelop::ILaunchConfiguration* cfg ) const
{
    if( !cfg )
    {
        return QUrl();
    }

    return cfg->config().readEntry( ExecutePlugin2::workingDirEntry, QUrl() );
}


QString ExecutePlugin2::nativeAppConfigTypeId() const
{
    return _nativeAppConfigTypeId;
}


#include "executeplugin.moc"
