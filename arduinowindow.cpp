#include "arduinowindow.h"

#include <QDebug>
#include <QPixmap>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include <solid/device.h>

#include "toolkit.h"

using namespace KDevelop;

// TODO create a abstracttablemodel to update img and text
arduinoWindow::arduinoWindow(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    model = new arduinoWindowModel;

    // Just select the options
    boardCombo->setEditable(false);
    interfaceCombo->setEditable(false);
    baudCombo->setEditable(false);

    Board::instance().update();
    QStringList boardsId = Board::instance().boardList;
    QStringList boardsName;
    foreach(const QString &boardId, boardsId)
        boardsName << Board::instance().boards[boardId].name;
    model->populate(boardsId, boardsName);

    boardCombo->setModel(model);
    boardComboChanged(boardCombo->currentText());

    // TODO filter devices
    auto devices = Solid::Device::allDevices();
    foreach(const Solid::Device device, devices)
        if(device.product() != "")
            interfaceCombo->addItem(device.product());
        //qDebug() << device.description() << device.parentUdi() << device.product() << device.udi() << device.vendor();

    connect(boardCombo, &QComboBox::currentTextChanged, this,  &arduinoWindow::boardComboChanged);
}

void arduinoWindow::boardComboChanged(QString text)
{
    baudCombo->clear();
    QString id = model->getData(boardCombo->currentIndex()).id;
    QStringList baud = Board::instance().boards[id].upSpeed;

    baudCombo->addItems(baud);
    // TODO add boards description
    bitext->setText(text);
    // TODO get path to image
    //image->setPixmap(QPixmap::fromImage(image));
}

arduinoWindow::~arduinoWindow()
{
    delete ui;
}
