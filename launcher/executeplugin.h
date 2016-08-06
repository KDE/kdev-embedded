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

#pragma once

#include <interfaces/iplugin.h>
#include <QtCore/QVariant>
#include <QtCore/QProcess>
#include "iexecuteplugin.h"

class QUrl;
class KJob;

class NativeAppConfigType;

class ExecutePlugin : public KDevelop::IPlugin, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES(IExecutePlugin)

public:
    explicit ExecutePlugin(QObject *parent, const QVariantList & = QVariantList());
    ~ExecutePlugin() override;

    static QString _nativeAppConfigTypeId;
    static QString workingDirEntry;
    static QString executableEntry;
    static QString argumentsEntry;
    static QString isExecutableEntry;
    static QString dependencyEntry;
    static QString environmentGroupEntry;
    static QString useTerminalEntry;
    static QString terminalEntry;
    static QString userIdToRunEntry;
    static QString dependencyActionEntry;
    static QString projectTargetEntry;

    void unload() override;

    QUrl executable(KDevelop::ILaunchConfiguration*, QString& err) const override;
    QStringList arguments(KDevelop::ILaunchConfiguration*, QString& err) const override;
    QUrl workingDirectory(KDevelop::ILaunchConfiguration*) const override;
    KJob* dependencyJob(KDevelop::ILaunchConfiguration*) const override;
    QString environmentGroup(KDevelop::ILaunchConfiguration*) const override;
    bool useTerminal(KDevelop::ILaunchConfiguration*) const override;
    QString terminal(KDevelop::ILaunchConfiguration*) const override;
    QString nativeAppConfigTypeId() const override;

    NativeAppConfigType* m_configType;
};

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
