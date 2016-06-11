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
#include "debug.h"
#include "board.h"

#include <util/kdevstringhandler.h>
#include <util/environmentgrouplist.h>
#include <project/projectitemlineedit.h>
#include "projecttargetscombobox.h"

#include <QDebug>
#include <QIcon>
#include <QMenu>

#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KShell>

#include <solid/device.h>
#include <solid/devicenotifier.h>

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

//TODO: Make sure to auto-add the executable target to the dependencies when its used.

void EmbeddedLauncherConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject* project)
{
    qCDebug(ElMsg) << "EmbeddedLauncherConfigPage::loadFromConfiguration" << cfg.groupList();
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

    //executablePath->setFilter("application/x-executable");

    executableRadio->setChecked(true);
    if (!cfg.readEntry(ExecutePlugin::isExecutableEntry, false) && projectTarget->count())
    {
        projectTargetRadio->setChecked(true);
    }

    arguments->setClearButtonEnabled(true);
    arguments->setText(cfg.readEntry(ExecutePlugin::argumentsEntry, ""));
    workingDirectory->setUrl(cfg.readEntry(ExecutePlugin::workingDirEntry, QUrl()));
    //terminal->setEditText( cfg.readEntry( ExecutePlugin::terminalEntry, terminal->itemText(0) ) );
    QStringList strDeps;
    blockSignals(b);
}

EmbeddedLauncherConfigPage::EmbeddedLauncherConfigPage(QWidget* parent)
    : LaunchConfigurationPage(parent),
      m_model(new ArduinoWindowModel(parent))
{
    setupUi(this);
    //Setup data info for combobox

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

    presetsCombo->setEnabled(false);

    //connect signals to changed signal
    connect(projectTarget, static_cast<void(ProjectTargetsComboBox::*)(const QString&)>(&ProjectTargetsComboBox::currentIndexChanged), this, &EmbeddedLauncherConfigPage::changed);
    connect(projectTargetRadio, &QRadioButton::toggled, this, &EmbeddedLauncherConfigPage::changed);
    connect(executableRadio, &QRadioButton::toggled, this, &EmbeddedLauncherConfigPage::changed);
    connect(executablePath->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(executablePath, &KUrlRequester::urlSelected, this, &EmbeddedLauncherConfigPage::changed);
    connect(arguments, &QLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(workingDirectory, &KUrlRequester::urlSelected, this, &EmbeddedLauncherConfigPage::changed);
    connect(workingDirectory->lineEdit(), &KLineEdit::textEdited, this, &EmbeddedLauncherConfigPage::changed);
    connect(devices, &Solid::DeviceNotifier::deviceAdded, this, &EmbeddedLauncherConfigPage::devicesChanged);
    connect(devices, &Solid::DeviceNotifier::deviceRemoved, this, &EmbeddedLauncherConfigPage::devicesChanged);
    connect(boardCombo, &QComboBox::currentTextChanged, this,  &EmbeddedLauncherConfigPage::boardComboChanged);
    connect(mcuFreqCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,  &EmbeddedLauncherConfigPage::mcuFreqComboChanged);
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
    else
    {
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
    cfg.writeEntry(ExecutePlugin::argumentsEntry, arguments->text());
    cfg.writeEntry(ExecutePlugin::workingDirEntry, workingDirectory->url());
    //cfg.writeEntry( ExecutePlugin::terminalEntry, terminal->currentText() );
    QVariantList deps;
    cfg.writeEntry(ExecutePlugin::dependencyEntry, KDevelop::qvariantToString(QVariant(deps)));
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
//  TODO: we probably want to flexibilize, but at least we won't be accepting wrong values anymore
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
//         QString path = KDevelop::joinWithEscaping(splitPath,'/','\\');
        cfg.writeEntry(ExecutePlugin::projectTargetEntry, splitPath);
        cfg.writeEntry(ExecutePlugin::dependencyEntry, KDevelop::qvariantToString(QVariantList() << splitPath));
        cfg.writeEntry(ExecutePlugin::dependencyActionEntry, "Build");
        cfg.sync();

        emit signalAddLaunchConfiguration(config);
    }
}

void EmbeddedLauncherConfigPage::devicesChanged(const QString& udi)
{
    Q_UNUSED(udi);
    interfaceCombo->clear();
    auto devices = Solid::Device::allDevices();

    qCDebug(ElMsg) << "devicesChanged";
    bool interfaceExist = false;
    foreach (const auto& device, devices)
    {
        if (device.product() != QStringLiteral("") and device.udi().contains(QStringLiteral("tty")))
        {
            interfaceExist = true;
            interfaceCombo->addItem(device.product());
            qCDebug(ElMsg) << "INTERFACE ############ INTERFACE";
            qCDebug(ElMsg) << "Description\t:" << device.description();
            qCDebug(ElMsg) << "Parent Udi\t:" << device.parentUdi();
            qCDebug(ElMsg) << "Product\t:" << device.product();
            qCDebug(ElMsg) << "Udi\t:" << device.udi();
            qCDebug(ElMsg) << "Vendor\t:" << device.vendor();
            qCDebug(ElMsg) << "Icon\t:" << device.icon();
            qCDebug(ElMsg) << "Emblems\t:" << device.emblems();
            qCDebug(ElMsg) << "Interface\t:" << device.udi().split(QStringLiteral("/")).takeLast();
            m_interface = QString(device.udi().split(QStringLiteral("/")).takeLast());
        }
    }

    if (interfaceExist == false)
    {
        interfaceCombo->setEnabled(false);
        interfaceCombo->addItem(i18n("Please connect one !"));
        interfaceLabel->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
    }
    else
    {
        interfaceCombo->setEnabled(true);
        interfaceLabel->setText(i18n("Interface:"));
        interfaceLabel->setStyleSheet(QStringLiteral("color: rgb(0, 0, 0);"));
    }
}

void EmbeddedLauncherConfigPage::boardComboChanged(const QString& text)
{
    Q_UNUSED(text);
    mcuFreqCombo->clear();
    QString id = m_model->getData(boardCombo->currentIndex()).m_id;

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
        mcuFreqCombo->addItem(QStringLiteral("%0, %1").arg(mcu).arg(freq));
        index += 1;
    }
    Board::instance().m_boards[id].printData();
    mcuFreqComboChanged(0);
}

void EmbeddedLauncherConfigPage::mcuFreqComboChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    qCDebug(ElMsg) << "mcuFreqComboBox Index: " << index;
    qCDebug(ElMsg) << "mcuFreqComboBox Count: " << mcuFreqCombo->count();

    if (mcuFreqCombo->count() <= 1)
    {
        mcuFreqCombo->setEnabled(false);
    }
    else
    {
        mcuFreqCombo->setEnabled(true);
    }
}
