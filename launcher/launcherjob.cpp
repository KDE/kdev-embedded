/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
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

#include "launcherjob.h"
#include "executeplugin.h"
#include "toolkit.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include <KLocalizedString>
#include <KShell>
#include <KSharedConfig>
#include <KConfigGroup>

#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iruncontroller.h>
#include <outputview/outputmodel.h>
#include <util/environmentgrouplist.h>

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iplugincontroller.h>
#include <project/projectmodel.h>

#include <execute/iexecuteplugin.h>

#include "debug.h"

using namespace KDevelop;

Q_LOGGING_CATEGORY(LaMsg, "Kdev.embedded.la.msg")

LauncherJob::LauncherJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg)
    : KDevelop::OutputExecuteJob(parent)
    , m_cfgname(cfg->name())
{
    qCDebug(LaMsg) << "LauncherJob::LauncherJob";
    qCDebug(LaMsg) << "name" << cfg->name();
    qCDebug(LaMsg) << "entryMap" << cfg->config().entryMap();
    qCDebug(LaMsg) << "groupList" << cfg->config().groupList();
    qCDebug(LaMsg) << "keyList" << cfg->config().keyList();

    setCapabilities(Killable);

    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevembedded-launcher"))->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    KDevelop::EnvironmentGroupList l(KSharedConfig::openConfig());
    QString envgrp = iface->environmentGroup(cfg);
    qCDebug(LaMsg) << "LauncherJob::LauncherJob envgrp" << envgrp;

    QString err;
    QUrl executable = iface->executable(cfg, err);
    qCDebug(LaMsg) << "LauncherJob::LauncherJob executable" << executable;

    if (!err.isEmpty())
    {
        setError(-1);
        setErrorText(err);
        return;
    }

    if (envgrp.isEmpty())
    {
        qWarning() << "Launch Configuration:" << cfg->name() << i18n("No environment group specified, looks like a broken "
                   "configuration, please check run configuration '%1'. "
                   "Using default environment group.", cfg->name());
        envgrp = l.defaultGroup();
    }
    setEnvironmentProfile(envgrp);

    QStringList arguments = iface->arguments(cfg, err);
    qCDebug(LaMsg) << "LauncherJob::LauncherJob arguments" << arguments;
    if (!err.isEmpty())
    {
        setError(-2);
        setErrorText(err);
    }

    if (error() != 0)
    {
        qWarning() << "Launch Configuration:" << cfg->name() << "oops, problem" << errorText();
        return;
    }

    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setFilteringStrategy(OutputModel::NativeAppErrorFilter);
    setProperties(DisplayStdout | DisplayStderr);

    // Now setup the process parameters

    QUrl wc = iface->workingDirectory(cfg);
    if (!wc.isValid() || wc.isEmpty())
    {
        wc = QUrl::fromLocalFile(QFileInfo(executable.toLocalFile()).absolutePath());
    }
    setWorkingDirectory(wc);

    qCDebug(EpMsg) << "setting app:" << executable << arguments;

    if (iface->useTerminal(cfg))
    {
        QStringList args = KShell::splitArgs(iface->terminal(cfg));
        for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
        {
            if (*it == QLatin1String("%exe"))
            {
                *it = KShell::quoteArg(executable.toLocalFile());
            }
            else if (*it == QLatin1String("%workdir"))
            {
                *it = KShell::quoteArg(wc.toLocalFile());
            }
        }
        args.append(arguments);
        *this << args;
    }
    else
    {
        *this << executable.toLocalFile();
        *this << arguments;
    }

    setJobName(cfg->name());
}

LauncherJob* findNativeJob(KJob* j)
{
    LauncherJob* job = qobject_cast<LauncherJob*>(j);
    if (!job)
    {
        const QList<LauncherJob*> jobs = j->findChildren<LauncherJob*>();
        if (!jobs.isEmpty())
            job = jobs.first();
    }
    return job;
}

void LauncherJob::start()
{
    // We kill any execution of the configuration
    foreach (KJob* j, ICore::self()->runController()->currentJobs())
    {
        LauncherJob* job = findNativeJob(j);
        if (job && job != this && job->m_cfgname == m_cfgname)
        {
            QMessageBox::StandardButton button = QMessageBox::question(nullptr, i18n("Job already running"), i18n("'%1' is already being executed. Should we kill the previous instance?", m_cfgname));
            if (button != QMessageBox::No)
                j->kill();
        }
    }

    OutputExecuteJob::start();
}

void LauncherJob::output(const QStringList& l)
{
    if (KDevelop::OutputModel* m = model())
    {
        m->appendLines(l);
    }
}

KDevelop::OutputModel* LauncherJob::model()
{
    return dynamic_cast<KDevelop::OutputModel*>(KDevelop::OutputJob::model());
}
