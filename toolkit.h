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

#pragma once

// file to manage tools

#include <QStringList>

class Toolkit
{
private:
    static QString toolkitVersion(const QString &path);
    static QString boardFilePath();
    static QString avrProgramPath();

    Toolkit& operator = (Toolkit& other) = delete;
    Toolkit(const Toolkit& other) = delete;
    Toolkit();
public:
    static  Toolkit& instance();

    /**
     * @brief Check if path is a valid arduino folder
     *
     * @param Arduino path
     * @return bool True if valid and False if not
     */
    static bool isValidArduinoPath(const QString &path);

    static QString getBoardFile(const QString &path);
    static QString avrdudePath();
};
