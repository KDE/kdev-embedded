/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/executecompositejob.h>

#include <interfaces/iplugincontroller.h>

#include "executeplugin.h"
#include "toolkit.h"
#include "debug.h"
#include "board.h"

#include <util/kdevstringhandler.h>
#include <project/projectitemlineedit.h>
#include "projecttargetscombobox.h"

#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QItemSelection>

#include <QStandardPaths>

#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KShell>

#include <solid/device.h>
#include <solid/devicenotifier.h>

#include "firsttimewizard.h"

using namespace KDevelop;
using namespace Solid;

Q_LOGGING_CATEGORY(ElMsg, "Kdev.embedded.el.msg")

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

    const QString arg = cfg.readEntry(ExecutePlugin::argumentsEntry, QString());
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

    const int launcherIndex = cfg.readEntry(ExecutePlugin::launcherIndexEntry, 0);
    stackedWidget->setCurrentIndex(launcherIndex);
    presetsComboPage1->setCurrentIndex(launcherIndex);
    presetsComboPage2->setCurrentIndex(launcherIndex);

    const QString openocdArg = cfg.readEntry(ExecutePlugin::openocdArgEntry, QString());
    const QUrl openocdWork = cfg.readEntry(ExecutePlugin::openocdWorkEntry, QUrl());
    const QString openocdComm = cfg.readEntry(ExecutePlugin::openocdCommEntry, QString());

    if (!openocdArg.isEmpty())
    {
        openocdArgumentsCombo->setCurrentText(openocdArg);
    }
    if (!openocdWork.isEmpty())
    {
        openocdWorkingDirectory->setUrl(openocdWork);
    }
    if (!openocdComm.isEmpty())
    {
        openocdCommand->setCurrentText(openocdComm);
    }

    if (boardIndex < boardCombo->count())
    {
        boardCombo->setCurrentIndex(boardIndex);
    }
    if (mcuIndex < mcuCombo->count())
    {
        mcuCombo->setCurrentIndex(mcuIndex);
    }

    blockSignals(b);
}

EmbeddedLauncherConfigPage::EmbeddedLauncherConfigPage(QWidget* parent)
    : LaunchConfigurationPage(parent),
      m_model(new ArduinoWindowModel(parent))
{
    setupUi(this);

    m_boardImgsDir = QDir(QStandardPaths::locate(
                              QStandardPaths::GenericDataLocation,
                              QLatin1String("kdevembedded/boardsimg"),
                              QStandardPaths::LocateDirectory
                          ) + QChar::fromLatin1('/'));

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

    // Start in the first widget and in the first preset
    stackedWidget->setCurrentIndex(0);
    presetsComboPage1->setCurrentIndex(0);
    presetsComboPage2->setCurrentIndex(0);


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

    connect(presetsComboPage1, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::presetsChanged);
    connect(presetsComboPage2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::presetsChanged);

    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    if (settings.readEntry("arduinoFolder", "").isEmpty())
    {
        QMessageBox::warning(0, i18n("kdev-embedded"), i18n("Please run the first time wizard."));
        FirstTimeWizard *embeddedWindow = new FirstTimeWizard(parent);
        embeddedWindow->setAttribute(Qt::WA_DeleteOnClose);
        embeddedWindow->show();
    }
}

const QString EmbeddedLauncherConfigPage::mcuTooltip()
{
    return i18n("Please add a microcontroller");
}

const QStringList EmbeddedLauncherConfigPage::mcuList()
{
    return Toolkit::instance().avrdudeMcuList();
}
const QString EmbeddedLauncherConfigPage::interfaceTooltip()
{
    QString tooltip;
    tooltip += i18n("Please connect or select an interface, for example:\n"
                    "/dev/ttyUSBx, /dev/ttyACMx, COMx, etc");
    return tooltip;
}

const QString EmbeddedLauncherConfigPage::baudTooltip()
{
    QString tooltip;
    tooltip += i18n("Please choose or select a baudrate:\n"
                    "19200, 57600, 115200, etc");
    return tooltip;
}

const QString EmbeddedLauncherConfigPage::argumentsTooltip()
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

const QString EmbeddedLauncherConfigPage::commandTooltip()
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
    Q_UNUSED(project);

    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::saveToConfiguration" << cfg.config()->groupList();
    cfg.writeEntry(ExecutePlugin::isExecutableEntry, executableRadio->isChecked());
    cfg.writeEntry(ExecutePlugin::executableEntry, executablePath->text());
    cfg.writeEntry(ExecutePlugin::projectTargetEntry, projectTarget->currentItemPath());

    cfg.writeEntry(ExecutePlugin::launcherIndexEntry, stackedWidget->currentIndex());

    cfg.writeEntry(ExecutePlugin::boardEntry, boardCombo->currentIndex());
    cfg.writeEntry(ExecutePlugin::mcuEntry, mcuCombo->currentText());
    // interface
    // interface baud rate
    cfg.writeEntry(ExecutePlugin::argumentsEntry, argumentsCombo->currentText());
    cfg.writeEntry(ExecutePlugin::workingDirEntry, workingDirectory->url());
    cfg.writeEntry(ExecutePlugin::commandEntry, commandCombo->currentText());

    cfg.writeEntry(ExecutePlugin::openocdArgEntry, openocdArgumentsCombo->currentText());
    cfg.writeEntry(ExecutePlugin::openocdWorkEntry, openocdWorkingDirectory->url());
    cfg.writeEntry(ExecutePlugin::openocdCommEntry, openocdCommand->currentText());

    // Arduino configuration
    const KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    const QString avrdudeConf = Toolkit::instance().avrConfigFile();

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
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::saveToConfiguration" << "arduinoConf" << arduinoConf << m_mcu << m_baud << interfaceCombo->currentText();
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
        return nullptr;
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
    return nullptr;
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
    QStringList interface;
    // It's necessary to implement a better way to check the interface
    auto devices = Solid::Device::allDevices();
    //auto devices = Solid::Device::listFromType(Solid::DeviceInterface::Type::Block);

    qCDebug(ElMsg) << "devicesChanged";
    foreach (const auto& device, devices)
    {
        if (device.product() != QStringLiteral("") && device.udi().contains(QStringLiteral("tty")))
        {
            qCDebug(ElMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(ElMsg) << "Description\t:" << device.description();
            qCDebug(ElMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(ElMsg) << "Product\t:" << device.product();
            qCDebug(ElMsg) << "Udi\t:" << device.udi();
            qCDebug(ElMsg) << "Vendor\t:" << device.vendor();
            qCDebug(ElMsg) << "Icon\t:" << device.icon();
            qCDebug(ElMsg) << "Emblems\t:" << device.emblems();
            qCDebug(ElMsg) << "Interface\t:" << device.udi().split(QStringLiteral("/")).takeLast();
            interface << QString(QStringLiteral("/dev/") + device.udi().split(QStringLiteral("/")).takeLast());
        }
    }

    interface.removeDuplicates();
    interfaceCombo->addItems(interface);

    if (interfaceCombo->count() == 0)
    {
        interfaceCombo->clear();
        interfaceCombo->lineEdit()->setPlaceholderText(i18n("Could not find interface"));
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
    mcuCombo->insertSeparator(mcuCombo->count());
    mcuCombo->addItems(mcuList());
    baudCombo->clear();
    baudCombo->addItems(m_baud);
    qCDebug(ElMsg) << "SIZEOF MCUCOMBO" << sizeof(mcuCombo);
    qCDebug(ElMsg) << "SIZEOF BAUDCOMBO" << sizeof(baudCombo);


    // TODO: select image from board selection
    QPixmap pix(QStringLiteral("%1/%2.svg").arg(m_boardImgsDir.absolutePath(), id));
    if (pix.isNull())
    {
        pix = QPixmap(m_boardImgsDir.absolutePath() + QStringLiteral("/arduino.svg"));
    }

    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged Pixmap" << pix;

    if (pix.width() > pix.height())
    {
        qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged Start rotation" << id << pix;
        QTransform rotate_disc;
        pix = pix.transformed(QTransform().rotate(90));
        qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged End rotation" << id << pix;
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

    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged Baord image path" << id << pix;
    image->setPixmap(m_pixBuffer);

    mcuComboChanged(0);
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged" << "mcus"  << m_mcu;
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::boardComboChanged" << image->width() << image->height();
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

void EmbeddedLauncherConfigPage::presetsChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    qCDebug(ElMsg) << "presetsCombo Index: " << index;

    stackedWidget->setCurrentIndex(index);
    presetsComboPage1->setCurrentIndex(index);
    presetsComboPage2->setCurrentIndex(index);
}
