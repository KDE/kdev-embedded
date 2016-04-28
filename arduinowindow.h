#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

// Configure Arduino board and interface

#include <Qt>
#include <QDir>
#include <QDialog>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(AwMsg);

#include "ui_arduinowindow.h"

class Board;

namespace Solid
{
    class DeviceNotifier;
};

class arduinoWindowModel : public QAbstractTableModel {
    Q_OBJECT
private:
    struct coluns
    {
        QString id;
        QString name;
    };

    QVector<coluns> db;

public:
    enum {NAME, ID, COLUMNS};
    void populate(QStringList ids, QStringList names)
    {
        Q_ASSERT(ids.size() == names.size());
        for(int i=0; i<ids.size();i++)
            db.push_back(coluns{ids[i], names[i]});
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
        if(!index.isValid())
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            if(index.column() == ID)
                return db.at(index.row()).id;
            else if(index.column() == NAME)
                return db.at(index.row()).name;
        }

        return QVariant();
    };

    coluns getData(int index)
    {
        if(index>-1)
            return db.at(index);
        return coluns{QString(""), QString("")};
    }

    int columnCount(const QModelIndex &parent) const { Q_UNUSED(parent) return COLUMNS; }
    int rowCount(const QModelIndex &parent) const { Q_UNUSED(parent) return db.count(); }
};

class ArduinoWindow : public QDialog, Ui::ArduinoWindow
{
    Q_OBJECT

public:
    explicit ArduinoWindow(QWidget *parent = 0);
    ~ArduinoWindow();

private:
    arduinoWindowModel *model;
    void boardComboChanged(const QString& text);
    void devicesChanged(const QString& udi);

    Board *board;

    QDir boardImgsDir;
    Solid::DeviceNotifier *devices;
};

#endif // ARDUINOWINDOW_H
