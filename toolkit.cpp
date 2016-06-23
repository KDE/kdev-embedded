/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Copyright 2010 Denis Martinez
 * Copyright 2010 Martin Peres
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

#include "toolkit.h"

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>

#include <interfaces/isession.h>
#include <interfaces/icore.h>
#include <KConfigGroup>

using namespace KDevelop;

Toolkit::Toolkit()
{
    KConfigGroup settings = ICore::self()->activeSession()->config()->group("Embedded");
    m_arduinoFolder = new QFile(settings.readEntry("arduinoFolder", QString()));
    m_arduinoPath = getPath(m_arduinoFolder->fileName());
}

Toolkit::~Toolkit()
{
    delete m_arduinoFolder;
}

Toolkit& Toolkit::instance()
{
    static Toolkit self;
    return self;
}

QString Toolkit::arduinoPath()
{
    return m_arduinoPath;
}

QString Toolkit::boardFile(QString path)
{
    return getPath(path + boardFilePath());
}

QString Toolkit::boardFile()
{
    return boardFile(arduinoPath());
}

QString Toolkit::toolkitVersion(QString path)
{
    QFile file(QDir(path).filePath(QStringLiteral("revisions.txt")));
    if (!file.open(QFile::ReadOnly))
    {
        return QString();
    }

    QTextStream reader(&file);
    QString arduinoVersion = reader.readLine();
    while (arduinoVersion == QChar::fromLatin1('\n') && ! reader.atEnd())
    {
        arduinoVersion = reader.readLine();
    }
    QList<QString> list = arduinoVersion.split(QChar::fromLatin1(' '));
    if (list.size() >= 2)
    {
        return  list.at(1).trimmed();
    }
    return QString();
}

QString Toolkit::toolkitVersion()
{
    return toolkitVersion(arduinoPath());
}

QString Toolkit::avrdudeConfigPath()
{
    return QStringLiteral("/hardware/tools/avr/etc/avrdude.conf");
}
QString Toolkit::avrConfigFile()
{
    return getPath(arduinoPath() + avrdudeConfigPath());
}

QString Toolkit::avrdudePath()
{
    return QString(avrProgramPath() + QStringLiteral("/avrdude"));
}

QString Toolkit::getAvrdudeFile()
{
    return getPath(arduinoPath() + avrProgramPath() + QStringLiteral("/avrdude"));
}

QString Toolkit::boardFilePath()
{
    return QStringLiteral("/hardware/arduino/avr/boards.txt");
}

QString Toolkit::avrProgramPath()
{
    return QStringLiteral("/hardware/tools/avr/bin");
}

bool Toolkit::isValidArduinoPath(QString path)
{
    QString version = Toolkit::toolkitVersion(path);
    return (version == QStringLiteral("1.6.8") || version == QStringLiteral("1.6.7"));
}

QString Toolkit::getPath(QString path)
{
    return QFile(path).exists() ? path : QString();
}

bool Toolkit::setArduinoPath(QString path)
{
    if (QFile(path).exists())
    {
        m_arduinoFolder = new QFile(path);
        m_arduinoPath = getPath(m_arduinoFolder->fileName());
        return true;
    }
    return false;
}

