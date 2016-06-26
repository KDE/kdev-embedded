#include "arduinowindowmodel.h"
#include "board.h"

ArduinoWindowModel::ArduinoWindowModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ArduinoWindowModel::populate(const QVector<ArduinoWindowModelStruct> &tdb)
{
    beginResetModel();
    m_db = tdb;
    endResetModel();
}

QVariant ArduinoWindowModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        if (index.column() == ID)
        {
            return m_db.at(index.row()).m_id;
        }
        else if (index.column() == NAME)
        {
            return m_db.at(index.row()).m_name;
        }
    }

    return QVariant();
}

ArduinoWindowModelStruct ArduinoWindowModel::getData(int index)
{
    if (index > -1)
    {
        return m_db.at(index);
    }
    return ArduinoWindowModelStruct{QString(), QString()};
}

int ArduinoWindowModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return COLUMNS;
}

int ArduinoWindowModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_db.count();
}
