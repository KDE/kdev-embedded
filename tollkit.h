#ifndef TOOLKIT_H
#define TOOLKIT_H

// file to manage tools

#include <QStringList>

class Toolkit
{
private:
  static QString toolkitVersion(const QString &path);
public:
    /**
     * @brief Check if path is a valid arduino folder
     *
     * @param Arduino path
     * @return bool True if valid and False if not
     */
    static bool isValidArduinoPath(const QString &path);
};

#endif // TOOLKIT_H
