/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Based onde the work of:
 *  Aleix Pol Gonzalez <aleixpol@kde.org>
 *  Andreas Pakulat <apaku@gmx.de>
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

#include "embeddedlauncher.h"

#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "launcherjob.h"
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/builderjob.h>
#include <interfaces/iuicontroller.h>
#include <util/executecompositejob.h>

#include <interfaces/iplugincontroller.h>

#include "executeplugin.h"
#include "toolkit.h"
#include "debug.h"
#include "board.h"

#include <util/kdevstringhandler.h>
#include <util/environmentgrouplist.h>
#include <project/projectitemlineedit.h>
#include "projecttargetscombobox.h"

#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>

#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KShell>

#include <solid/device.h>
#include <solid/devicenotifier.h>

 #include "firsttimewizard.h"

using namespace KDevelop;
using namespace Solid;

Q_LOGGING_CATEGORY(ElMsg, "Kdev.embedded.el.msg");

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

QIcon EmbeddedLauncherConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("system-run"));
}

static KDevelop::ProjectBaseItem* itemForPath(const QStringList& path, KDevelop::ProjectModel* model)
{
    return model->itemFromIndex(model->pathToIndex(path));
}

void EmbeddedLauncherConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project)
{
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::loadFromConfiguration" << cfg.groupList() << cfg.keyList() << cfg.entryMap();

    bool b = blockSignals(true);
    projectTarget->setBaseItem(project ? project->projectItem() : 0, true);
    projectTarget->setCurrentItemPath(cfg.readEntry(ExecutePlugin::projectTargetEntry, QStringList()));

    QUrl exe = cfg.readEntry(ExecutePlugin::executableEntry, QUrl());
    if (!exe.isEmpty() || project)
    {
        executablePath->setUrl(!exe.isEmpty() ? exe : project->path().toUrl());
    }
    else
    {
        KDevelop::IProjectController* pc = KDevelop::ICore::self()->projectController();
        if (pc)
        {
            executablePath->setUrl(pc->projects().isEmpty() ? QUrl() : pc->projects().at(0)->path().toUrl());
        }
    }

    executableRadio->setChecked(true);
    if (!cfg.readEntry(ExecutePlugin::isExecutableEntry, false) && projectTarget->count())
    {
        projectTargetRadio->setChecked(true);
    }

    QString arg = cfg.readEntry(ExecutePlugin::argumentsEntry, QString());
    if (argumentsCombo->findText(arg) == -1 && !arg.isEmpty())
    {
        argumentsCombo->addItem(arg);
    }

    workingDirectory->setUrl(cfg.readEntry(ExecutePlugin::workingDirEntry, QUrl()));
    commandCombo->setEditText(cfg.readEntry(ExecutePlugin::commandEntry, commandCombo->itemText(0)));

    const int boardIndex = cfg.readEntry(ExecutePlugin::boardEntry, 0);
    const int mcuIndex = cfg.readEntry(ExecutePlugin::mcuEntry, 0);

    qCDebug(ElMsg) << "Board index from cfg" << QString(cfg.readEntry(ExecutePlugin::boardEntry, 0)).toInt() << cfg.readEntry(ExecutePlugin::boardEntry, 0);
    qCDebug(ElMsg) << "BoardCombo size" << boardCombo->count();

    if (boardIndex < boardCombo->count())
    {
        boardCombo->setCurrentIndex(boardIndex);
    }
    if (mcuIndex < mcuCombo->count())
    {
        mcuCombo->setCurrentIndex(mcuIndex);
    }

    QStringList strDeps;
    blockSignals(b);
}

EmbeddedLauncherConfigPage::EmbeddedLauncherConfigPage(QWidget* parent)
    : LaunchConfigurationPage(parent),
      m_model(new ArduinoWindowModel(parent))
{
    setupUi(this);

    //Set workingdirectory widget to ask for directories rather than files
    workingDirectory->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    Solid::DeviceNotifier *devices = Solid::DeviceNotifier::instance();

    Board::instance().update();

    // Populate model
    QVector<ArduinoWindowModelStruct> data;
    Q_ASSERT(Board::instance().m_boardList.size() == Board::instance().m_boardNameList.size());
    for (int i = 0; i < Board::instance().m_boardNameList.size(); i++)
        data.push_back(ArduinoWindowModelStruct{Board::instance().m_boardList[i], Board::instance().m_boardNameList[i]});
    m_model->populate(data);

    // We don't gave presets yet
    presetsCombo->setEnabled(false);

    // Start ComboBoxes
    boardCombo->clear();
    boardCombo->setModel(m_model);
    boardComboChanged(boardCombo->currentText());
    devicesChanged(QString());

    mcuCombo->setToolTip(mcuTooltip());
    interfaceCombo->setToolTip(interfaceTooltip());
    baudCombo->setToolTip(baudTooltip());
    argumentsCombo->setToolTip(argumentsTooltip());
    commandCombo->setToolTip(commandTooltip());

    //connect signals to changed signal
    connect(projectTarget, static_cast<void(ProjectTargetsComboBox::*)(const QString&)>(&ProjectTargetsComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::changed);
    connect(projectTargetRadio, &QRadioButton::toggled, this, &EmbeddedLauncherConfigPage::changed);
    connect(executableRadio, &QRadioButton::toggled, this, &EmbeddedLauncherConfigPage::changed);
    connect(executablePath->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(executablePath, &KUrlRequester::urlSelected, this, &EmbeddedLauncherConfigPage::changed);
    connect(argumentsCombo->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(workingDirectory, &KUrlRequester::urlSelected, this, &EmbeddedLauncherConfigPage::changed);
    connect(workingDirectory->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(boardCombo, &QComboBox::currentTextChanged, this, &EmbeddedLauncherConfigPage::changed);
    connect(commandCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::changed);

    connect(devices, &Solid::DeviceNotifier::deviceAdded, this, &EmbeddedLauncherConfigPage::devicesChanged);
    connect(devices, &Solid::DeviceNotifier::deviceRemoved, this, &EmbeddedLauncherConfigPage::devicesChanged);
    connect(boardCombo, &QComboBox::currentTextChanged, this,  &EmbeddedLauncherConfigPage::boardComboChanged);
    connect(mcuCombo->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(mcuCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::mcuComboChanged);
    connect(baudCombo->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    if (settings.readEntry("arduinoFolder", "").isEmpty())
    {
        QMessageBox::warning(0, i18n("kdev-embedded"), i18n("Please, run the first time wizard."));
        FirstTimeWizard *embeddedWindow = new FirstTimeWizard(parent);
        embeddedWindow->setAttribute(Qt::WA_DeleteOnClose);
        embeddedWindow->show();
    }
}

QString EmbeddedLauncherConfigPage::mcuTooltip()
{
    QString tooltip;
    tooltip += i18n("Please choose one microcontroller:\n")
        += QStringLiteral("AT32UC3A0512, AT90CAN128, AT90CAN32, AT90CAN64, AT90PWM2, AT90PWM216, AT90PWM2B, AT90PWM3\n\
AT90PWM316, AT90PWM3B, AT90S1200, AT90S2313, AT90S2333, AT90S2343, AT90S4414\n\
AT90S4433, AT90S4434, AT90S8515, AT90S8535, AT90USB1286, AT90USB1287, AT90USB162\n\
AT90USB646, AT90USB647, AT90USB82, ATmega103, ATmega128, ATmega1280, ATmega1281\n\
ATmega1284, ATmega1284P, ATmega1284RFR2, ATmega128RFA1, ATmega128RFR2, ATmega16, ATmega161\n\
ATmega162, ATmega163, ATmega164P, ATmega168, ATmega168P, ATmega168PB, ATmega169\n\
ATmega16U2, ATmega2560, ATmega2561, ATmega2564RFR2, ATmega256RFR2, ATmega32, ATmega324P\n\
ATmega324PA, ATmega325, ATmega3250, ATmega328, ATmega328P, ATmega329, ATmega3290\n\
ATmega3290P, ATmega329P, ATmega32M1, ATmega32U2, ATmega32U4, ATMEGA406, ATmega48\n\
ATmega48P, ATmega48PB, ATmega64, ATmega640, ATmega644, ATmega644P, ATmega644RFR2\n\
ATmega645, ATmega6450, ATmega649, ATmega6490, ATmega64RFR2, ATmega8, ATmega8515\n\
ATmega8535, ATmega88, ATmega88P, ATmega88PB, ATmega8U2, ATtiny10, ATtiny11\n\
ATtiny12, ATtiny13, ATtiny15, ATtiny1634, ATtiny20, ATtiny2313, ATtiny24\n\
ATtiny25, ATtiny26, ATtiny261, ATtiny28, ATtiny4, ATtiny40, ATtiny4313\n\
ATtiny43u, ATtiny44, ATtiny45, ATtiny461, ATtiny5, ATtiny84, ATtiny85\n\
ATtiny861, ATtiny88, ATtiny9, ATxmega128A1, ATxmega128A1revD, ATxmega128A1U, ATxmega128A3\n\
ATxmega128A3U, ATxmega128A4, ATxmega128A4U, ATxmega128B1, ATxmega128B3, ATxmega128C3, ATxmega128D3\n\
ATxmega128D4, ATxmega16A4, ATxmega16A4U, ATxmega16C4, ATxmega16D4, ATxmega16E5, ATxmega192A1\n\
ATxmega192A3, ATxmega192A3U, ATxmega192C3, ATxmega192D3, ATxmega256A1, ATxmega256A3, ATxmega256A3B\n\
ATxmega256A3BU, ATxmega256A3U, ATxmega256C3, ATxmega256D3, ATxmega32A4, ATxmega32A4U, ATxmega32C4\n\
ATxmega32D4, ATxmega32E5, ATxmega384C3, ATxmega384D3, ATxmega64A1, ATxmega64A1U, ATxmega64A3\n\
ATxmega64A3U, ATxmega64A4, ATxmega64A4U, ATxmega64B1, ATxmega64B3, ATxmega64C3, ATxmega64D3\n\
ATxmega64D4, ATxmega8E5, ...");
        return tooltip;
}

QString EmbeddedLauncherConfigPage::interfaceTooltip()
{
    QString tooltip;
    tooltip += i18n("Please, connect or select an interface like:\n")
        += QStringLiteral("/dev/ttyUSBx, /dev/ttyACMx, COMx, ...");
    return tooltip;
}

QString EmbeddedLauncherConfigPage::baudTooltip()
{
    QString tooltip;
    tooltip += i18n("Please choose or select a baudrate:\n")
        += QStringLiteral("19200, 57600, 115200, ...");
    return tooltip;
}

QString EmbeddedLauncherConfigPage::argumentsTooltip()
{
    QString tooltip;
    tooltip += i18n("Variables to programmer:\n")
        += i18n("%avrdudeconf\t- Specify location of configuration file.\n")
        += i18n("%mcu\t- Required. Specify AVR device.\n")
        += i18n("%interface\t- Specify connection port.\n")
        += i18n("%baud\t- Override RS-232 baud rate.\n")
        += i18n("%hex\t- Firmware.");
    return tooltip;
}

QString EmbeddedLauncherConfigPage::commandTooltip()
{
    QString tooltip;
    tooltip += i18n("%avrdude - Avrdude is a program for downloading code and data to Atmel AVR microcontrollers.");
    return tooltip;
}

void EmbeddedLauncherConfigPage::checkActions(const QItemSelection& selected, const QItemSelection& unselected)
{
    Q_UNUSED(unselected);
    qCDebug(ElMsg) << "checkActions";
    if (!selected.indexes().isEmpty())
    {
        qCDebug(ElMsg) << "have selection";
        Q_ASSERT(selected.indexes().count() == 1);
        QModelIndex idx = selected.indexes().at(0);
        qCDebug(ElMsg) << "index" << idx;
    }
}

void EmbeddedLauncherConfigPage::selectItemDialog()
{
}

void EmbeddedLauncherConfigPage::saveToConfiguration(KConfigGroup cfg, KDevelop::IProject* project) const
{
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::saveToConfiguration" << cfg.config()->groupList();
    Q_UNUSED(project);
    cfg.writeEntry(ExecutePlugin::isExecutableEntry, executableRadio->isChecked());
    cfg.writeEntry(ExecutePlugin::executableEntry, executablePath->url());
    cfg.writeEntry(ExecutePlugin::projectTargetEntry, projectTarget->currentItemPath());
    cfg.writeEntry(ExecutePlugin::argumentsEntry, argumentsCombo->currentText());
    cfg.writeEntry(ExecutePlugin::commandEntry, commandCombo->currentText());
    cfg.writeEntry(ExecutePlugin::workingDirEntry, workingDirectory->url());
    cfg.writeEntry(ExecutePlugin::boardEntry, boardCombo->currentIndex());
    cfg.writeEntry(ExecutePlugin::mcuEntry, mcuCombo->currentText());

    // Arduino configuration
    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    QString arduinoPath = settings.readEntry("arduinoFolder", "");
    QString avrdudeConf = Toolkit::instance().avrConfigFile();

    QStringList arduinoConf;
    arduinoConf
        << m_model->getData(boardCombo->currentIndex()).m_id
        << mcuCombo->currentText()
        << baudCombo->currentText()
        << interfaceCombo->currentText()
        << executablePath->text()
        << avrdudeConf;
    cfg.writeEntry(ExecutePlugin::arduinoEntry, arduinoConf);

    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::saveToConfiguration" << cfg.groupList() << cfg.keyList() << cfg.entryMap();
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::saveToConfiguration" << "arduinoConf" << arduinoConf << m_mcu << m_baud << m_interface;
}

QString EmbeddedLauncherConfigPage::title() const
{
    return i18n("Configure Embedded Application");
}

EmbeddedLauncherConfigPage::~EmbeddedLauncherConfigPage()
{
}

QList< KDevelop::LaunchConfigurationPageFactory* > EmbeddedLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString EmbeddedLauncher::description() const
{
    return i18n("Upload applications to embedded platforms");
}

QString EmbeddedLauncher::id()
{
    return QStringLiteral("EmbeddedLauncher");
}

QString EmbeddedLauncher::name() const
{
    return i18n("Embedded");
}

EmbeddedLauncher::EmbeddedLauncher()
{
}

KJob* EmbeddedLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    qCDebug(ElMsg) << "EmbeddedLauncher::start" << launchMode << cfg->name() << cfg->config().groupList();
    Q_ASSERT(cfg);
    if (!cfg)
    {
        return 0;
    }
    if (launchMode == QLatin1String("execute"))
    {
        IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevembedded-launcher"))->extension<IExecutePlugin>();
        Q_ASSERT(iface);

        KJob* depjob = iface->dependencyJob(cfg);
        QList<KJob*> l;
        if (depjob)
        {
            l << depjob;
        }
        l << new LauncherJob(KDevelop::ICore::self()->runController(), cfg);
        return new KDevelop::ExecuteCompositeJob(KDevelop::ICore::self()->runController(), l);

    }
    qWarning() << "Unknown launch mode " << launchMode << "for config:" << cfg->name();
    return 0;
}

QStringList EmbeddedLauncher::supportedModes() const
{
    return QStringList() << QStringLiteral("execute");
}

KDevelop::LaunchConfigurationPage* NativeAppPageFactory::createWidget(QWidget* parent)
{
    return new EmbeddedLauncherConfigPage(parent);
}

NativeAppPageFactory::NativeAppPageFactory()
{
}

NativeAppConfigType::NativeAppConfigType()
{
    factoryList.append(new NativeAppPageFactory());
}

NativeAppConfigType::~NativeAppConfigType()
{
    qDeleteAll(factoryList);
    factoryList.clear();
}

QString NativeAppConfigType::name() const
{
    return i18n("Embedded");
}


QList<KDevelop::LaunchConfigurationPageFactory*> NativeAppConfigType::configPages() const
{
    return factoryList;
}

QString NativeAppConfigType::id() const
{
    return ExecutePlugin::_nativeAppConfigTypeId;
}

QIcon NativeAppConfigType::icon() const
{
    return QIcon::fromTheme(QStringLiteral("application-x-executable"));
}

bool NativeAppConfigType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    if (item->target() && item->target()->executable())
    {
        return canLaunch(item->target()->executable()->builtUrl());
    }
    return false;
}

bool NativeAppConfigType::canLaunch(const QUrl& file) const
{
    return (file.isLocalFile() && QFileInfo(file.toLocalFile()).isExecutable());
}

void NativeAppConfigType::configureLaunchFromItem(KConfigGroup cfg, KDevelop::ProjectBaseItem* item) const
{
    qCDebug(ElMsg) << "EmbeddedLauncher::configureLaunchFromItem" << cfg.config()->groupList();
    cfg.writeEntry(ExecutePlugin::isExecutableEntry, false);
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    cfg.writeEntry(ExecutePlugin::projectTargetEntry, model->pathFromIndex(model->indexFromItem(item)));
    cfg.writeEntry(ExecutePlugin::workingDirEntry, item->executable()->builtUrl().adjusted(QUrl::RemoveFilename));
    cfg.sync();
}

void NativeAppConfigType::configureLaunchFromCmdLineArguments(KConfigGroup cfg, const QStringList& args) const
{
    qCDebug(ElMsg) << "EmbeddedLauncher::configureLaunchFromCmdLineArguments" << cfg.config()->groupList();
    cfg.writeEntry(ExecutePlugin::isExecutableEntry, true);
    Q_ASSERT(QFile::exists(args.first()));

    cfg.writeEntry(ExecutePlugin::executableEntry, QUrl::fromLocalFile(args.first()));
    QStringList a(args);
    a.removeFirst();
    cfg.writeEntry(ExecutePlugin::argumentsEntry, KShell::joinArgs(a));
    cfg.sync();
}

QList<KDevelop::ProjectTargetItem*> targetsInFolder(KDevelop::ProjectFolderItem* folder)
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach (KDevelop::ProjectFolderItem* f, folder->folderList())
        ret += targetsInFolder(f);

    ret += folder->targetList();
    return ret;
}

bool actionLess(QAction* a, QAction* b)
{
    return a->text() < b->text();
}

bool menuLess(QMenu* a, QMenu* b)
{
    return a->title() < b->title();
}


QMenu* NativeAppConfigType::launcherSuggestions()
{

    QMenu* ret = new QMenu(i18n("Embedded Binary"));

    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    QList<KDevelop::IProject*> projects = KDevelop::ICore::self()->projectController()->projects();

    foreach (KDevelop::IProject* project, projects)
    {
        if (project->projectFileManager()->features() & KDevelop::IProjectFileManager::Targets)
        {
            QList<KDevelop::ProjectTargetItem*> targets=targetsInFolder(project->projectItem());
            QHash<KDevelop::ProjectBaseItem*, QList<QAction*> > targetsContainer;
            QMenu* projectMenu = ret->addMenu(QIcon::fromTheme(QStringLiteral("project-development")), project->name());
            foreach (KDevelop::ProjectTargetItem* target, targets)
            {
                if (target->executable())
                {
                    QStringList path = model->pathFromIndex(target->index());
                    if (!path.isEmpty())
                    {
                        QAction* act = new QAction(projectMenu);
                        act->setData(KDevelop::joinWithEscaping(path, QChar::fromLatin1('/'),QChar::fromLatin1('\\')));
                        act->setProperty("name", target->text());
                        path.removeFirst();
                        act->setText(path.join(QChar::fromLatin1('/')));
                        act->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
                        connect(act, &QAction::triggered, this, &NativeAppConfigType::suggestionTriggered);
                        targetsContainer[target->parent()].append(act);
                    }
                }
            }

            QList<QAction*> separateActions;
            QList<QMenu*> submenus;
            foreach (KDevelop::ProjectBaseItem* folder, targetsContainer.keys())
            {
                QList<QAction*> actions = targetsContainer.value(folder);
                if (actions.size()==1 || !folder->parent())
                {
                    separateActions += actions.first();
                }
                else
                {
                    foreach (QAction* a, actions)
                    {
                        a->setText(a->property("name").toString());
                    }
                    QStringList path = model->pathFromIndex(folder->index());
                    path.removeFirst();
                    QMenu* submenu = new QMenu(path.join(QStringLiteral("/")));
                    std::sort(actions.begin(), actions.end(), actionLess);
                    submenu->addActions(actions);
                    submenus += submenu;
                }
            }
            std::sort(separateActions.begin(), separateActions.end(), actionLess);
            std::sort(submenus.begin(), submenus.end(), menuLess);
            foreach (QMenu* m, submenus)
                projectMenu->addMenu(m);
            projectMenu->addActions(separateActions);

            projectMenu->setEnabled(!projectMenu->isEmpty());
        }
    }

    return ret;
}

void NativeAppConfigType::suggestionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    KDevelop::ProjectTargetItem* pitem =
        dynamic_cast<KDevelop::ProjectTargetItem*>
        (itemForPath(KDevelop::splitWithEscaping(action->data().toString(),QChar::fromLatin1('/'),QChar::fromLatin1('\\')), model));
    if (pitem)
    {
        QPair<QString,QString> launcher = qMakePair(launchers().at(0)->supportedModes().at(0), launchers().at(0)->id());
        KDevelop::IProject* p = pitem->project();

        KDevelop::ILaunchConfiguration* config = KDevelop::ICore::self()->runController()->createLaunchConfiguration(this, launcher, p, pitem->text());
        KConfigGroup cfg = config->config();
        qCDebug(ElMsg) << "NativeAppConfigType::suggestionTriggered" << cfg.groupList();
        QStringList splitPath = model->pathFromIndex(pitem->index());
        cfg.writeEntry(ExecutePlugin::projectTargetEntry, splitPath);
        cfg.writeEntry(ExecutePlugin::dependencyActionEntry, "Build");
        cfg.sync();

        emit signalAddLaunchConfiguration(config);
    }
}

void EmbeddedLauncherConfigPage::devicesChanged(const QString& udi)
{
    Q_UNUSED(udi);
    interfaceCombo->clear();
    // It's necessary to implement a better way to check the interface
    auto devices = Solid::Device::allDevices();
    //auto devices = Solid::Device::listFromType(Solid::DeviceInterface::Type::Block);

    qCDebug(ElMsg) << "devicesChanged";
    bool interfaceExist = false;
    foreach (const auto& device, devices)
    {
        if (device.product() != QStringLiteral("") and device.udi().contains(QStringLiteral("tty")))
        {
            interfaceExist = true;
            qCDebug(ElMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(ElMsg) << "Description\t:" << device.description();
            qCDebug(ElMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(ElMsg) << "Product\t:" << device.product();
            qCDebug(ElMsg) << "Udi\t:" << device.udi();
            qCDebug(ElMsg) << "Vendor\t:" << device.vendor();
            qCDebug(ElMsg) << "Icon\t:" << device.icon();
            qCDebug(ElMsg) << "Emblems\t:" << device.emblems();
            qCDebug(ElMsg) << "Interface\t:" << device.udi().split(QStringLiteral("/")).takeLast();
            m_interface << QString(QStringLiteral("/dev/") + device.udi().split(QStringLiteral("/")).takeLast());
        }
    }
    m_interface.removeDuplicates();
    interfaceCombo->addItems(m_interface);

    if (interfaceExist == false)
    {
        interfaceLabel->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
    }
    else
    {
        interfaceLabel->setText(i18n("Interface:"));
        interfaceLabel->setStyleSheet(QStringLiteral("color: rgb(0, 0, 0);"));
    }
}

void EmbeddedLauncherConfigPage::boardComboChanged(const QString& text)
{
    Q_UNUSED(text);
    QString id = m_model->getData(boardCombo->currentIndex()).m_id;
    Board::instance().m_boards[id].printData();

    m_mcu = Board::instance().m_boards[id].m_bMcu;
    m_baud = Board::instance().m_boards[id].m_upSpeed;
    m_mcu.removeDuplicates();
    m_baud.removeDuplicates();

    mcuCombo->clear();
    mcuCombo->addItems(m_mcu);
    baudCombo->clear();
    baudCombo->addItems(m_baud);

    mcuComboChanged(0);
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged" << "mcus"  << m_mcu;
}

void EmbeddedLauncherConfigPage::mcuComboChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    baudCombo->setCurrentIndex(index);

    qCDebug(ElMsg) << "mcuComboBox Index: " << index;
    qCDebug(ElMsg) << "mcuComboBox Count: " << mcuCombo->count();
}
