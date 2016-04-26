#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

// Configure Arduino board and interface

#include <Qt>
#include <QDir>
#include <QDialog>

#include "board.h"

#include "ui_arduinowindow.h"

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
    bool populate(QStringList ids, QStringList names)
    {
        if(ids.size() == names.size())
        {
            for(int i=0; i<ids.size();i++)
                db.push_back(coluns{ids[i], names[i]});
            return true;
        }
        return false;
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

class arduinoWindow : public QDialog, Ui::arduinowindow
{
    Q_OBJECT

public:
    explicit arduinoWindow(QWidget *parent = 0);
    ~arduinoWindow();

private:
    arduinoWindowModel *model;
    void boardComboChanged(const QString& text);
    void devicesChanged(const QString& udi);

    Board *board;
    Ui::arduinowindow *ui;

    QDir boardImgsDir;
    Solid::DeviceNotifier *devices;
};

#endif // ARDUINOWINDOW_H
