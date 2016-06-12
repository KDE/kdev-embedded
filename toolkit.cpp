/*
 * This file is part of KDevelop project
 * Copyright 2016 Patrick José Pereira <patrickelectric@gmail.com>
 * Based onde the work Arduide Project of:
 *  Denis Martinez
 *  Martin Peres
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

QString Toolkit::getBoardFile(const QString &path)
{
    QFile file(path + boardFilePath());
    if (!file.open(QFile::ReadOnly))
    {
        return QString();
    }
    return path + boardFilePath();
}

QString Toolkit::toolkitVersion(const QString &path)
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

QString Toolkit::avrdudeConfigPath()
{
    return QStringLiteral("/hardware/tools/avr/etc/avrdude.conf");
}

QString Toolkit::avrdudePath()
{
    return QString(avrProgramPath() + QStringLiteral("/avrdude"));
}

QString Toolkit::boardFilePath()
{
    return QStringLiteral("/hardware/arduino/avr/boards.txt");
}

QString Toolkit::avrProgramPath()
{
    return QStringLiteral("/hardware/tools/avr/bin");
}

bool Toolkit::isValidArduinoPath(const QString &path)
{
    QString version = Toolkit::toolkitVersion(path);
    return (version == QStringLiteral("1.6.8") || version == QStringLiteral("1.6.7"));
}
