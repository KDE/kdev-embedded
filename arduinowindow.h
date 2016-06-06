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

#pragma once

// Configure Arduino board and interface

#include <QDir>
#include <QDialog>
#include <QLoggingCategory>
// need to create a better way without this include
#include <QProcess>

Q_DECLARE_LOGGING_CATEGORY(AwMsg);

#include "ui_arduinowindow.h"

class Board;
class QPixmap;
class QAbstractTableModel;

namespace Solid
{
class DeviceNotifier;
};

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

class ArduinoWindow : public QDialog, Ui::ArduinoWindow
{
    Q_OBJECT

public:
    explicit ArduinoWindow(QWidget *parent = 0);
    ~ArduinoWindow();

private:
    ArduinoWindowModel *m_model;
    QString getRedRichTextSelected(QStringList list, int index);
    QString richTextDescription();
    void chooseHexPath();
    void boardComboChanged(const QString& text);
    void mcuFreqComboChanged(int index);
    void devicesChanged(const QString& udi);
    void buttonBoxOk();
    void buttonBoxCancel();
    void avrdudeStderr(int exitCode, QProcess::ExitStatus exitStatus);
    void avrdudeStdout();
    void uploadCheckChanged(int state);

    QString m_interface;
    QProcess *m_avrdudeProcess;
    Board *m_board;

    QPixmap m_pixBuffer;
    QDir m_boardImgsDir;
    Solid::DeviceNotifier *devices;
};
