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

#include <interfaces/launchconfigurationtype.h>
#include <interfaces/launchconfigurationpage.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>

#include "ui_embeddedlauncher.h"

namespace Solid
{
class DeviceNotifier;
}

class Board;

struct ArduinoWindowModelStruct
{
    QString m_id;
    QString m_name;
};

class ArduinoWindowModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QVector<ArduinoWindowModelStruct> m_db;

public:
    ArduinoWindowModel(QObject *parent);
    enum {NAME, ID, COLUMNS};
    void populate(const QVector<ArduinoWindowModelStruct> &tdb);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    ArduinoWindowModelStruct getData(int index);

    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return COLUMNS;
    }
    int rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return m_db.count();
    }
};

class EmbeddedLauncherConfigPage : public KDevelop::LaunchConfigurationPage, Ui::NativeAppPage
{
    Q_OBJECT
public:
    explicit EmbeddedLauncherConfigPage(QWidget* parent);
    virtual ~EmbeddedLauncherConfigPage();
    void loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project = nullptr) override;
    void saveToConfiguration(KConfigGroup cfg, KDevelop::IProject* project = nullptr) const override;
    QString title() const override;
    QIcon icon() const override;
private:
    void mcuFreqComboChanged(int index);
    void checkActions(const QItemSelection& , const QItemSelection&);
    void boardComboChanged(const QString& text);
    void devicesChanged(const QString& udi);
    void selectItemDialog();

    Board *m_board;
    QString m_interface;
    QScopedPointer<Solid::DeviceNotifier> *m_devices;
    ArduinoWindowModel *m_model;
};

class EmbeddedLauncher : public KDevelop::ILauncher
{
public:
    EmbeddedLauncher();
    QList< KDevelop::LaunchConfigurationPageFactory* > configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;
    QStringList supportedModes() const override;
};

class NativeAppPageFactory : public KDevelop::LaunchConfigurationPageFactory
{
public:
    NativeAppPageFactory();
    KDevelop::LaunchConfigurationPage* createWidget(QWidget* parent) override;
};

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configured informations, as well as its type and a name
 */
class NativeAppConfigType : public KDevelop::LaunchConfigurationType
{
    Q_OBJECT
public:
    NativeAppConfigType();
    ~NativeAppConfigType() override;

    QString id() const override;
    QString name() const override;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;
    QIcon icon() const override;
    bool canLaunch(KDevelop::ProjectBaseItem* item) const override;
    bool canLaunch(const QUrl& file) const override;
    void configureLaunchFromItem(KConfigGroup cfg,
                                 KDevelop::ProjectBaseItem* item) const override;
    void configureLaunchFromCmdLineArguments(KConfigGroup cfg,
            const QStringList& args) const override;
    QMenu* launcherSuggestions() override;
private:
    QList<KDevelop::LaunchConfigurationPageFactory*> factoryList;

public slots:
    void suggestionTriggered();
};

