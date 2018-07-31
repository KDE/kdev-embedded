#pragma once

#include <QString>
#include <QModelIndex>
#include <QAbstractTableModel>

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

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    ArduinoWindowModelStruct getData(int index);

    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
};
