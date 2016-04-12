#include "tollkit.h"

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>


QString Toolkit::toolkitVersion(const QString &path)
{
    QFile file(QDir(path).filePath("revisions.txt"));
    if(!file.open(QFile::ReadOnly))
      return QString();

    QByteArray arduinoVersion = file.readLine();
    while (arduinoVersion == "\n" && ! file.atEnd()) { arduinoVersion = file.readLine();}
    QList<QByteArray> list = arduinoVersion.split(' ');
    if (list.size() >= 2)
      return  list.at(1).trimmed();
  return QString();
}

bool Toolkit::isValidArduinoPath(const QString &path)
{
  //return acceptableVersion.contains(toolkitVersion(path));
  QString version = Toolkit::toolkitVersion(path);
  return (version=="1.6.8" || version=="1.6.7");
}
