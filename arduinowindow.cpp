#include "arduinowindow.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QLoggingCategory>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include <solid/device.h>
#include <solid/devicenotifier.h>

#include <QAbstractTableModel>

#include "board.h"
#include "toolkit.h"

Q_LOGGING_CATEGORY(AwMsg, "Kdev.embedded.aw.msg");

using namespace KDevelop;
using namespace Solid;

ArduinoWindowModel::ArduinoWindowModel(QObject *parent)
    :QAbstractTableModel(parent)
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
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        if(index.column() == ID)
            return m_db.at(index.row()).m_id;
        else if(index.column() == NAME)
            return m_db.at(index.row()).m_name;
    }

    return QVariant();
}

ArduinoWindowModelStruct ArduinoWindowModel::getData(int index)
{
    if(index>-1)
        return m_db.at(index);
    return ArduinoWindowModelStruct{QString(""), QString("")};
}

//TODO: create document to add board ID, description and image
ArduinoWindow::ArduinoWindow(QWidget *parent) :
    QDialog(parent),
    m_model (new ArduinoWindowModel(parent)),
    devices (new Solid::DeviceNotifier)
{
    qCDebug(AwMsg) << "AW opened";
    setupUi(this);

    m_boardImgsDir = QDir(QStandardPaths::locate(
                        QStandardPaths::GenericDataLocation,
                        QLatin1String("kdevembedded/boardsimg"),
                        QStandardPaths::LocateDirectory
                        ) + '/');

    // Just select the options
    boardCombo->setEditable(false);
    interfaceCombo->setEditable(false);
    baudCombo->setEditable(false);

    // Update variables
    Board::instance().update();

    // Populate model
    QVector<ArduinoWindowModelStruct> data;
    Q_ASSERT(Board::instance().m_boardList.size() == Board::instance().m_boardNameList.size());
    for(int i = 0; i<Board::instance().m_boardNameList.size();i++)
        data.push_back(ArduinoWindowModelStruct{Board::instance().m_boardList[i], Board::instance().m_boardNameList[i]});
    m_model->populate(data);

    // Start ComboBoxes
    boardCombo->setModel(m_model);
    boardComboChanged(boardCombo->currentText());
    devicesChanged(QString());

    devices = Solid::DeviceNotifier::instance();
    connect(devices, &Solid::DeviceNotifier::deviceAdded, this, &ArduinoWindow::devicesChanged);
    connect(devices, &Solid::DeviceNotifier::deviceRemoved, this, &ArduinoWindow::devicesChanged);
    connect(boardCombo, &QComboBox::currentTextChanged, this,  &ArduinoWindow::boardComboChanged);
}

void ArduinoWindow::boardComboChanged(const QString& text)
{
    baudCombo->clear();
    QString id = m_model->getData(boardCombo->currentIndex()).m_id;
    QStringList baud = Board::instance().m_boards[id].m_upSpeed;

    baudCombo->addItems(baud);
    // TODO: add boards description
    qCDebug(AwMsg) << "Baord selected" << text;
    bitext->setText(text);

    // TODO: select image from board selection
    QString imageLocal = m_boardImgsDir.absolutePath()+"/arduino_uno(rev3)-icsp_breadboard.svg";
    image->setPixmap(QPixmap::fromImage(QImage(imageLocal)));
}

void ArduinoWindow::devicesChanged(const QString& udi)
{
    Q_UNUSED(udi);

    interfaceCombo->clear();
    auto devices = Solid::Device::allDevices();
    foreach(const auto& device, devices)
    {
        if(device.product() != "" and device.udi().contains("tty"))
        {
            interfaceCombo->addItem(device.product());
            qCDebug(AwMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(AwMsg) << "Description\t:" << device.description();
            qCDebug(AwMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(AwMsg) << "Product\t:" << device.product();
            qCDebug(AwMsg) << "Udi\t:" << device.udi();
            qCDebug(AwMsg) << "Vendor\t:" <<device.vendor();
        }
    }
}

ArduinoWindow::~ArduinoWindow()
{
}
