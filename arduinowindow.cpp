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
    db = tdb;
    endResetModel();
}

QVariant ArduinoWindowModel::data(const QModelIndex& index, int role) const
{
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
}

ArduinoWindowModelStruct ArduinoWindowModel::getData(int index)
{
    if(index>-1)
        return db.at(index);
    return ArduinoWindowModelStruct{QString(""), QString("")};
}

//TODO: create document to add board ID, description and image
ArduinoWindow::ArduinoWindow(QWidget *parent) :
    QDialog(parent),
    model (new ArduinoWindowModel(parent)),
    devices (new Solid::DeviceNotifier)
{
    QLoggingCategory potato("Kdev.embedded.aw.msg");
    qDebug() << "potato" << potato.isDebugEnabled();
    qCDebug(AwMsg) << "AW opened";
    setupUi(this);

    boardImgsDir = QDir(QStandardPaths::locate(
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
    Q_ASSERT(Board::instance().boardList.size() == Board::instance().boardNameList.size());
    for(int i=0; i<Board::instance().boardNameList.size();i++)
        data.push_back(ArduinoWindowModelStruct{Board::instance().boardList[i], Board::instance().boardNameList[i]});
    model->populate(data);

    // Start ComboBoxes
    boardCombo->setModel(model);
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
    QString id = model->getData(boardCombo->currentIndex()).id;
    QStringList baud = Board::instance().boards[id].upSpeed;

    baudCombo->addItems(baud);
    // TODO: add boards description
    qCDebug(AwMsg) << "Baord selected" << text;
    bitext->setText(text);

    // TODO: select image from board selection
    QString imageLocal = boardImgsDir.absolutePath()+"/arduino_uno(rev3)-icsp_breadboard.svg";
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
