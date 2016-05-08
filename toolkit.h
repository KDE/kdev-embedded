#ifndef EMBEDDED_TOOLKIT_H
#define EMBEDDED_TOOLKIT_H

// file to manage tools

#include <QStringList>

class Toolkit
{
private:
    static QString toolkitVersion(const QString &path);
    static QString boardFilePath();

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
};

#endif // EMBEDDED_TOOLKIT_H
