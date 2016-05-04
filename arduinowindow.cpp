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

#include <KConfigGroup>

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
    mcuFreqCombo->setEditable(false);

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
    connect(mcuFreqCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,  &ArduinoWindow::mcuFreqComboChanged);
}

void ArduinoWindow::mcuFreqComboChanged(int index)
{
    if(index < 0)
        return;

    qCDebug(AwMsg) << "mcuFreqComboBox Index: " << index;

    QString id = m_model->getData(boardCombo->currentIndex()).m_id;
    Q_ASSERT(Board::instance().m_boards[id].m_bMcu.size() >= index);

    QString mcu = Board::instance().m_boards[id].m_bMcu[index];
    QString freq;
    if(Board::instance().m_boards[id].m_bFcpu.size() == Board::instance().m_boards[id].m_bMcu.size())
        freq = Board::instance().m_boards[id].m_bFcpu[index];
    else
        freq = Board::instance().m_boards[id].m_bFcpu[0];

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");

    settings.writeEntry("buildId", id);
    settings.writeEntry("buildMcu", mcu);
    settings.writeEntry("buildFreq", freq);

    qCDebug(AwMsg) << "mcuFreqComboBox Count: " << mcuFreqCombo->count();

    if(mcuFreqCombo->count() <= 1)
        mcuFreqCombo->setEnabled(false);
    else
        mcuFreqCombo->setEnabled(true);
}

void ArduinoWindow::boardComboChanged(const QString& text)
{
    mcuFreqCombo->clear();
    QString id = m_model->getData(boardCombo->currentIndex()).m_id;

    QStringList mcus = Board::instance().m_boards[id].m_bMcu;
    QStringList freqs = Board::instance().m_boards[id].m_bFcpu;

    QString freq;
    uint index = 0;
    foreach(const auto& mcu, mcus)
    {
      if(mcus.size() == freqs.size())
        freq = freqs[index];
      else
        freq = freqs[0];

      QString freqMHz = QString::number(freq.left(freq.lastIndexOf("0")+1).toInt()/1e6)+"MHz";
      mcuFreqCombo->addItem(mcu+", "+freqMHz);
      index += 1;
    }
    Board::instance().m_boards[id].printData();


    // TODO: add boards description
    qCDebug(AwMsg) << "Baord selected" << text;
    bitext->setText(text);

    // TODO: select image from board selection
    QString imageLocal = m_boardImgsDir.absolutePath()+"/arduino_uno(rev3)-icsp_breadboard.svg";
    image->setPixmap(QPixmap::fromImage(QImage(imageLocal)));

    mcuFreqComboChanged(0);
}

void ArduinoWindow::devicesChanged(const QString& udi)
{
    Q_UNUSED(udi);
    interfaceCombo->clear();
    auto devices = Solid::Device::allDevices();

    bool interfaceExist = false;
    foreach(const auto& device, devices)
    {
        if(device.product() != "" and device.udi().contains("tty"))
        {
            interfaceExist = true;
            interfaceCombo->addItem(device.product());
            qCDebug(AwMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(AwMsg) << "Description\t:" << device.description();
            qCDebug(AwMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(AwMsg) << "Product\t:" << device.product();
            qCDebug(AwMsg) << "Udi\t:" << device.udi();
            qCDebug(AwMsg) << "Vendor\t:" <<device.vendor();
        }
    }

    if(interfaceExist == false)
    {
        interfaceCombo->setEnabled(false);
        interfacelabel->setText("Interface (please connect one):");
        interfacelabel->setStyleSheet("color: rgb(255, 0, 0);");
    }
    else
    {
        interfaceCombo->setEnabled(true);
        interfacelabel->setText("Interface:");
        interfacelabel->setStyleSheet("color: rgb(0, 0, 0);");
    }
}

ArduinoWindow::~ArduinoWindow()
{
}
