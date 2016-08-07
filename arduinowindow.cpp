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

#include "arduinowindow.h"

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLoggingCategory>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include <solid/device.h>
#include <solid/devicenotifier.h>

#include <QAbstractTableModel>
#include <QAbstractButton>

#include <KConfigGroup>
#include <KColorScheme>

#include "board.h"
#include "toolkit.h"
#include "arduinowindowmodel.h"

Q_LOGGING_CATEGORY(AwMsg, "Kdev.embedded.aw.msg")

using namespace KDevelop;
using namespace Solid;

//TODO: create document to add board ID, description and image
ArduinoWindow::ArduinoWindow(QWidget *parent) :
    QDialog(parent),
    m_model(new ArduinoWindowModel(parent)),
    m_avrdudeProcess(new QProcess(parent))
{
    qCDebug(AwMsg) << "AW opened";
    setupUi(this);

    m_boardImgsDir = QDir(QStandardPaths::locate(
                              QStandardPaths::GenericDataLocation,
                              QLatin1String("kdevembedded/boardsimg"),
                              QStandardPaths::LocateDirectory
                          ) + QChar::fromLatin1('/'));

    // Just select the options
    boardCombo->setEditable(false);
    interfaceCombo->setEditable(false);
    mcuFreqCombo->setEditable(false);

    // Update variables
    Board::instance().update();

    // Populate model
    QVector<ArduinoWindowModelStruct> data;
    Q_ASSERT(Board::instance().m_boardList.size() == Board::instance().m_boardNameList.size());
    for (int i = 0; i < Board::instance().m_boardNameList.size(); i++)
        data.push_back(ArduinoWindowModelStruct{Board::instance().m_boardList[i], Board::instance().m_boardNameList[i]});
    m_model->populate(data);

    // Start ComboBoxes
    boardCombo->setModel(m_model);
    boardComboChanged(boardCombo->currentText());
    devicesChanged(QString());

    projectCheck->setEnabled(false);
    projectBox->setEnabled(false);
    hexPathEdit->setEnabled(false);
    hexPathButton->setEnabled(false);
    verboseCheck->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Upload"));

    // Start output box configuration
    setOutpotStatus(Normal);
    output->append(i18n("Welcome,\n\nKDev-Embedded is still in alpha,\nplease be careful and report any problems you find.\n\nHave fun!"));

    Solid::DeviceNotifier *devices = Solid::DeviceNotifier::instance();

    connect(hexPathButton, &QToolButton::clicked, this, &ArduinoWindow::chooseHexPath);
    m_avrdudeProcess->connect(m_avrdudeProcess, &QProcess::readyReadStandardOutput, this, &ArduinoWindow::avrdudeStdout);
    m_avrdudeProcess->connect(m_avrdudeProcess, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished, this, &ArduinoWindow::avrdudeStderr);

    connect(uploadCheck, &QCheckBox::stateChanged, this, &ArduinoWindow::uploadCheckChanged);

    connect(devices, &Solid::DeviceNotifier::deviceAdded, this, &ArduinoWindow::devicesChanged);
    connect(devices, &Solid::DeviceNotifier::deviceRemoved, this, &ArduinoWindow::devicesChanged);
    connect(boardCombo, &QComboBox::currentTextChanged, this,  &ArduinoWindow::boardComboChanged);
    connect(mcuFreqCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,  &ArduinoWindow::mcuFreqComboChanged);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ArduinoWindow::buttonBoxOk);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ArduinoWindow::buttonBoxCancel);
}

void ArduinoWindow::chooseHexPath()
{
    const QString path = QFileDialog::getOpenFileName(this, i18n("Choose .hex file"), QDir::currentPath());
    if (!path.isEmpty())
    {
        hexPathEdit->setText(path);
    }
}

void ArduinoWindow::mcuFreqComboChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    qCDebug(AwMsg) << "mcuFreqComboBox Index: " << index;
    qCDebug(AwMsg) << "mcuFreqComboBox Count: " << mcuFreqCombo->count();

    if (mcuFreqCombo->count() <= 1)
    {
        mcuFreqCombo->setEnabled(false);
    }
    else
    {
        mcuFreqCombo->setEnabled(true);
    }

    bitext->setText(richTextDescription());
}

void ArduinoWindow::boardComboChanged(const QString& text)
{
    Q_UNUSED(text);
    mcuFreqCombo->clear();
    const QString id = m_model->getData(boardCombo->currentIndex()).m_id;

    QStringList mcus = Board::instance().m_boards[id].m_bMcu;
    QStringList freqs = Board::instance().m_boards[id].m_freqHz;

    QString freq;
    int index = 0;
    foreach (const auto& mcu, mcus)
    {
        if (mcus.size() == freqs.size())
        {
            freq = freqs[index];
        }
        else
        {
            freq = freqs[0];
        }

        mcuFreqCombo->addItem(i18nc("<MCU name>, <MCU frequency>", "%1, %2", mcu, freq));
        index += 1;
    }
    Board::instance().m_boards[id].printData();

    // TODO: select image from board selection
    QPixmap pix(QStringLiteral("%1/%2.svg").arg(m_boardImgsDir.absolutePath(), id));
    if (pix.isNull())
    {
        pix = QPixmap(m_boardImgsDir.absolutePath() + QStringLiteral("/arduino.svg"));
    }

    if (pix.width() > image->width() || pix.height() > image->height())
    {
        pix = pix.scaled(image->width(), image->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_pixBuffer = QPixmap(image->width(), image->height());
    QPainter painter(&m_pixBuffer);
    painter.fillRect(QRect(0, 0, image->width(), image->height()), palette().background());
    painter.drawPixmap(m_pixBuffer.width() / 2 - pix.width() / 2, m_pixBuffer.height() / 2 - pix.height() / 2, pix);
    painter.end();

    qCDebug(AwMsg) << "Baord image path" << id << pix;
    image->setPixmap(m_pixBuffer);

    mcuFreqComboChanged(0);
}

void ArduinoWindow::devicesChanged(const QString& udi)
{
    Q_UNUSED(udi);
    interfaceCombo->clear();
    auto devices = Solid::Device::allDevices();

    foreach (const auto& device, devices)
    {
        if (!device.product().isEmpty() and device.udi().contains(QStringLiteral("tty")))
        {
            interfaceCombo->addItem(device.product());
            qCDebug(AwMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(AwMsg) << "Description\t:" << device.description();
            qCDebug(AwMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(AwMsg) << "Product\t:" << device.product();
            qCDebug(AwMsg) << "Udi\t:" << device.udi();
            qCDebug(AwMsg) << "Vendor\t:" << device.vendor();
            qCDebug(AwMsg) << "Icon\t:" << device.icon();
            qCDebug(AwMsg) << "Emblems\t:" << device.emblems();
            qCDebug(AwMsg) << "Interface\t:" << device.udi().split(QChar::fromLatin1('/')).takeLast();
            m_interface = QString(device.udi().split(QChar::fromLatin1('/')).takeLast());
        }
    }

    if (interfaceCombo->count())
    {
        interfaceCombo->setEnabled(true);
    }
    else
    {
        interfaceCombo->addItem(i18n("Could not find interface"));
        interfaceCombo->setEnabled(false);
    }
}

void ArduinoWindow::buttonBoxOk()
{
    qCDebug(AwMsg) << "Button clicked" << "Ok";
    int index = mcuFreqCombo->currentIndex();
    const QString id = m_model->getData(boardCombo->currentIndex()).m_id;
    Q_ASSERT(Board::instance().m_boards[id].m_bMcu.size() >= index);

    const QString mcu = Board::instance().m_boards[id].m_bMcu[index];
    QString freq;
    if (Board::instance().m_boards[id].m_bFcpu.size() == Board::instance().m_boards[id].m_bMcu.size())
    {
        freq = Board::instance().m_boards[id].m_bFcpu[index];
    }
    else
    {
        freq = Board::instance().m_boards[id].m_bFcpu[0];
    }

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    settings.writeEntry("buildId", id);
    settings.writeEntry("buildMcu", mcu);
    settings.writeEntry("buildFreq", freq);

    qCDebug(AwMsg) << "buildId " << id;
    qCDebug(AwMsg) << "buildMcu " << mcu;
    qCDebug(AwMsg) << "buildFreq " << freq;

    QStringList flags;
    if (verboseCheck->checkState() == Qt::Checked)
    {
        flags << QStringLiteral("-v") << QStringLiteral("-v") << QStringLiteral("-v") << QStringLiteral("-v");
    }
    else
    {
        flags << QStringLiteral("-q") << QStringLiteral("-q");
    }

    flags << QStringLiteral("-C")
          << Toolkit::instance().avrConfigFile()
          << QStringLiteral("-p%0").arg(mcu)
          << QStringLiteral("-c") << Board::instance().m_boards[id].m_upProtocol[0]
          << QStringLiteral("-P") << QStringLiteral("/dev/%0").arg(m_interface)
          << QStringLiteral("-b") << Board::instance().m_boards[id].m_upSpeed
          << QStringLiteral("-D")
          << QStringLiteral("-Uflash:w:%0:i").arg(hexPathEdit->text());

    if (m_avrdudeProcess->state() != QProcess::NotRunning)
    {
        m_avrdudeProcess->close();
    }

    output->clear();
    output->append(i18n("Running...\n"));
    qCDebug(AwMsg) << Toolkit::instance().getAvrdudeFile() << flags;
    // Check if file exist to not create a zombie
    if (QFileInfo(Toolkit::instance().getAvrdudeFile()).exists())
    {
        m_avrdudeProcess->start(Toolkit::instance().getAvrdudeFile(), flags);
    }
}

void ArduinoWindow::avrdudeStderr(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString perr;
    perr.fromLocal8Bit(m_avrdudeProcess->readAllStandardError());
    if (exitCode != 0)
    {
        qCDebug(AwMsg) << QStringLiteral("Error during upload.\n") << perr << exitCode << exitStatus;
        setOutpotStatus(Bad);
        output->append(i18n("Error during upload.\nCode: %1\n%2", exitCode, perr));
    }
    else
    {
        qCDebug(AwMsg) << QStringLiteral("Upload complete") << perr << exitCode << exitStatus;
        setOutpotStatus(Good);
        output->append(i18n("Upload complete.\n"));
        output->append(perr);
    }
}

void ArduinoWindow::avrdudeStdout()
{
    m_avrdudeProcess->setReadChannel(QProcess::StandardOutput);
    QTextStream stream(m_avrdudeProcess);
    while (!stream.atEnd())
    {
        output->append(stream.readLine());
        qCDebug(AwMsg) << "avrdudeStdout" << stream.readLine();
    }
}

void ArduinoWindow::uploadCheckChanged(int state)
{
    bool enable = (state == Qt::Checked);

    hexPathEdit->setEnabled(enable);
    hexPathButton->setEnabled(enable);
    verboseCheck->setEnabled(enable);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
}

QString ArduinoWindow::richTextDescription()
{
    const QString id = m_model->getData(boardCombo->currentIndex()).m_id;
    const QStringList mcus = Board::instance().m_boards[id].m_bMcu;
    const QStringList freqs = Board::instance().m_boards[id].m_freqHz;
    const QStringList flashs = Board::instance().m_boards[id].m_upMaxSizeKb;
    const QStringList srams = Board::instance().m_boards[id].m_upMaxDataSizeKb;

    const int index = mcuFreqCombo->currentIndex();
    QString mcu = getRedRichTextSelected(mcus, index);
    QString freq = getRedRichTextSelected(freqs, index);
    QString flash = getRedRichTextSelected(flashs, index);
    QString sram = getRedRichTextSelected(srams, index);

    // TODO: add a better board description
    return i18n("<p>Processor:</p>"
                "<ul>"
                "<li>%1</li>"
                "</ul>"
                "<p>Frequency:</p>"
                "<ul>"
                "<li>%2</li>"
                "</ul>"
                "<p>Memory:</p>"
                "<ul>"
                "<li>Flash (kB): %3</li>"
                "<li>SRAM (kB): %4</li>"
                "</ul>",
                mcu, freq, flash, sram);
}

QString ArduinoWindow::getRedRichTextSelected(QStringList list, int index)
{
    const QStringList ulist = list.toSet().toList();

    QString item;
    QString temp;

    qCDebug(AwMsg) << "List size" << list << list.size() << "Index" << index;

    if (list.isEmpty())
    {
        return QString();
    }

    if (list.size() <= 1)
    {
        item = QStringLiteral("<font color='red'>%0</font>").arg(list[0]);
    }
    else
    {
        foreach (auto const& oitem, ulist)
        {
            temp = oitem;
            if (oitem == list[index])
            {
                temp = QStringLiteral("<font color='red'>%0</font>").arg(oitem);
            }
            if (item.isEmpty())
            {
                item = temp;
            }
            else
            {
                item = QStringLiteral("%0,%1").arg(item).arg(temp);
            }
        }
    }

    return item;
}

void ArduinoWindow::buttonBoxCancel()
{
    qCDebug(AwMsg) << "Button clicked" << "Cancel";
    close();
}

void ArduinoWindow::setOutpotStatus(status st)
{
    QPalette p = output->palette();
    switch (st)
    {
        case Normal:
            KColorScheme::adjustForeground(p, KColorScheme::ActiveText);
            KColorScheme::adjustBackground(p, KColorScheme::ActiveBackground);
        break;
        case Good:
            KColorScheme::adjustForeground(p, KColorScheme::PositiveText);
            KColorScheme::adjustBackground(p, KColorScheme::PositiveBackground);
        break;
        case Bad:
            KColorScheme::adjustForeground(p, KColorScheme::NegativeText);
            KColorScheme::adjustBackground(p, KColorScheme::NegativeBackground);
        break;
    }
    output->setPalette(p);
}

ArduinoWindow::~ArduinoWindow()
{
}
