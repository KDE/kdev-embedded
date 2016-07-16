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

#pragma once

// file to manage tools

#include <QStringList>

class QFile;

class Toolkit
{
private:
    Toolkit& operator = (Toolkit& other) = delete;
    Toolkit(const Toolkit& other) = delete;
    Toolkit();
    ~Toolkit();

    QString avrdudeConfigPath();
    QString avrProgramPath();
    QString boardFilePath();
    QString getPath(QString path);

    QFile* m_arduinoFolder;
    QString m_arduinoPath;

    QStringList m_avrdudeMcuList;

public:
    static Toolkit& instance();

    /**
     * @brief Check if path is a valid arduino folder
     *
     * @param Arduino path
     * @return bool True if valid and False if not
     */
    bool isValidArduinoPath(QString path);

    QString toolkitVersion(QString path);
    QString toolkitVersion();

    QString arduinoPath();

    QString boardFile(QString path);
    QString boardFile();

    QString avrConfigFile();

    QString avrdudePath();
    QString getAvrdudeFile();
    QStringList avrdudeMcuList();

    bool setArduinoPath(QString path);
};
