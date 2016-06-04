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
