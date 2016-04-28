#include "arduinowindow.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QCoreApplication>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include <solid/device.h>
#include <solid/devicenotifier.h>

#include "board.h"
#include "toolkit.h"

using namespace KDevelop;

using namespace Solid;

//TODO: create document to add board ID, description and image
ArduinoWindow::ArduinoWindow(QWidget *parent) :
    QDialog(parent),
    model (new arduinoWindowModel),
    devices (new Solid::DeviceNotifier)
{
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
    model->populate(Board::instance().boardList, Board::instance().boardNameList);

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
    bitext->setText(text);

    // TODO: select image from board selection
    QString imageLocal = boardImgsDir.absolutePath()+"/NetduinoPlus2.svg";
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
            qDebug() << "INTERFACE ############ INTERFACE";
            qDebug() << "Description\t:" << device.description();
            qDebug() << "Parent Udi\t:" << device.parentUdi();
            qDebug() << "Product\t:" << device.product();
            qDebug() << "Udi\t:" << device.udi();
            qDebug() << "Vendor\t:" <<device.vendor();
        }
    }
}

ArduinoWindow::~ArduinoWindow()
{
}
