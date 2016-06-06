/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Based onde the work of:
 *  Andreas Pakulat <apaku@gmx.de>
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

#pragma once

#include <outputview/outputexecutejob.h>

#include <QtCore/QProcess>

namespace KDevelop
{
class ILaunchConfiguration;
}

class KProcess;

class LauncherJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    LauncherJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg);

    void start() override;

private:
    QString m_cfgname;

    void output(const QStringList& l);
    KDevelop::OutputModel* model();
};
